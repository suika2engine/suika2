/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

package jp.luxion.suika;

import static android.opengl.GLSurfaceView.RENDERMODE_CONTINUOUSLY;
import static android.opengl.GLSurfaceView.RENDERMODE_WHEN_DIRTY;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.AssetFileDescriptor;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.GLES20;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.Window;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.view.SurfaceHolder;

import androidx.annotation.RequiresApi;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.File;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends Activity {
    static {
        System.loadLibrary("suika");
    }

    /** タッチスクロールの1行分の移動距離です。 */
    private static final int LINE_HEIGHT = 10;

    /** ミキサのストリーム数です。 */
    private static final int MIXER_STREAMS = 4;

    /** ミキサのBGMストリームです。 */
    private static final int BGM_STREAM = 0;

    /** ミキサのVOICEストリームです。 */
    private static final int VOICE_STREAM = 1;

    /** ミキサのSEストリームです。 */
    private static final int SE_STREAM = 2;

    /** ミキサのSYSストリームです。 */
    private static final int SYS_STREAM = 2;

    /** Viewです。 */
    private MainView view;

    /** ビューポートサイズを1としたときの、レンダリング先の拡大率です。 */
    private float scale;

    /** レンダリング先のXオフセットです。 */
    private int offsetX;

    /** レンダリング先のXオフセットです。 */
    private int offsetY;

    /** タッチ座標です。 */
    private int touchStartX, touchStartY, touchLastY;

    /** タッチされている指の数です。 */
    private int touchCount;

    /** 終了処理が完了しているかを表します。 */
    private boolean isFinished;

    /** BGM/VOICE/SEのMediaPlayerです。 */
    private MediaPlayer[] player = new MediaPlayer[MIXER_STREAMS];

    /** ビデオのMediaPlayerです。 */
    private MediaPlayer video;

    /** ビデオのビューです。 */
    private VideoSurfaceView videoView;

    /** ビデオ再生開始のHandlerです。 */
    private Handler videoStartHandler;

    /** ビデオ再生終了のHandlerです。 */
    private Handler videoStopHandler;

    /** ビデオ再生中のHandlerです。 */
    private Handler videoLoopHandler;

    /** ビデオ再生中に一定周期でコマンドのイベント処理を行うスレッドです。 */
    private Thread videoThread;

    /** ビデオ再生から復帰した直後であるかを表します。 */
    private boolean resumeFromVideo;

    /** 同期用オブジェクトです。 */
    private Object syncObj = new Object();

    /**
     * アクティビティが作成されるときに呼ばれます。
     */
    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
    @SuppressWarnings("deprecation")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        isFinished = false;

        // ゲームビューを作成してセットする
        gameView = new GameView(this);
        setContentView(gameView);

		// 編集ビューを作成してセットする
		

        // ビデオ用のビューを作成しておく
        videoView = new VideoSurfaceView(this);
        videoThread = new Thread(videoView);
        videoThread.start();

        // ビデオの再生開始・再生終了・再生中をイベントスレッドで処理するためのHandlerを作る
        videoStartHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                view.setRenderMode(RENDERMODE_WHEN_DIRTY);
                setContentView(videoView);
                video.start();
                super.handleMessage(msg);
            }
        };
        videoStopHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                resumeFromVideo = true;
                setContentView(view);
                view.setRenderMode(RENDERMODE_CONTINUOUSLY);
                super.handleMessage(msg);
            }
        };
        videoLoopHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                videoView.invalidate();
                super.handleMessage(msg);
            }
        };
    }

    /**
     * 一時停止する際に呼ばれます。
     */
    @Override
    public void onPause() {
        super.onPause();

        // サウンドの再生を一時停止する
        for(int i=0; i<player.length; i++) {
            if(player[i] != null) {
                // すでに再生終了している場合を除外する
                if(!player[i].isPlaying())
                    player[i] = null;
                else
                    player[i].pause();
            }
        }
    }

    /**
     * 再開する際に呼ばれます。
     */
    @Override
    public void onResume() {
        super.onResume();

        // サウンドの再生を再開する
        for(int i=0; i<player.length; i++)
            if(player[i] != null)
                player[i].start();
    }

    /**
     * バックキーが押下された際に呼ばれます。
     */
    @SuppressWarnings("deprecation")
    @Override
    public void onBackPressed() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Quit?");
        builder.setNegativeButton("Yes", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                finishAndRemoveTask();
            }
        });
        builder.setNeutralButton("No", null);
        builder.create().show();
    }

    /**
     * 終了する際に呼ばれます。
     */
    @Override
    public void onDestroy() {
        super.onDestroy();

        if(!isFinished) {
            // JNIコードで終了処理を行う
            cleanup();

            isFinished = true;
        }
    }
}
