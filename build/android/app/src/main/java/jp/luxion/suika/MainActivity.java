/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

package jp.luxion.suika;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.AssetFileDescriptor;
import android.content.res.Resources;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.util.Log;
import android.util.AttributeSet;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.GLES20;
import android.view.MotionEvent;
import android.view.View;
import android.view.Display;
import android.view.Window;
import android.view.WindowInsetsController;
import android.view.WindowManager;
import android.graphics.Point;

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

    /** 仮想ビューポートの幅です。 */
    private static final int VIEWPORT_WIDTH = 1280;

    /** 仮想ビューポートの高さです。 */
    private static final int VIEWPORT_HEIGHT = 720;

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

        // フルスクリーンにする
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            getWindow().getDecorView().getWindowInsetsController().setSystemBarsBehavior(
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN) {
            getWindow().getDecorView().setSystemUiVisibility(View.SYSTEM_UI_FLAG_FULLSCREEN
                    |View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    |View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                    |View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    |View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    |View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            );
        } else {
            getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
        }
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        // ビューを作成してセットする
        view = new MainView(this);
        setContentView(view);
    }

    /**
     * ビューです。
     */
    private class MainView extends GLSurfaceView implements
            View.OnTouchListener,
            Renderer {
        /**
         * コンストラクタです。
         */
        public MainView(Context context) {
            super(context);

            setFocusable(true);
            setOnTouchListener(this);
            setEGLConfigChooser(8, 8, 8, 8, 0, 0);
            setEGLContextClientVersion(2);
            setRenderer(this);
        }

        /**
         * ビューが作成されるときに呼ばれます。
         */
        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // JNIコードで初期化処理を実行する
            init();
        }

        /**
         * ビューのサイズが決定した際に呼ばれます。
         */
        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            // ゲーム画面のアスペクト比を求める
            float aspect = (float)VIEWPORT_HEIGHT / (float)VIEWPORT_WIDTH;

            // 横幅優先で高さを仮決めする
            float w = width;
            float h = width * aspect;
            scale = w / (float)VIEWPORT_WIDTH;
            offsetX = 0;
            offsetY = (int)((float)(height - h) / 2.0f);

            // 高さが足りなければ、高さ優先で横幅を決める
            if(h > height) {
                h = height;
                w = height / aspect;
                scale = h / (float)VIEWPORT_HEIGHT;
                offsetX = (int)((float)(width - w) / 2.0f);
                offsetY = 0;
            }

            // ビューポートを更新する
            GLES20.glViewport(offsetX, offsetY, (int)w, (int)h);
        }

        /**
         * 表示される際に呼ばれます。
         */
        @Override
        public void onDrawFrame(GL10 gl) {
            if(isFinished)
                return;

            // イベントハンドラと排他制御する
            synchronized(syncObj) {
                // JNIコードでフレームを処理する
                if(!frame()) {
                    // JNIコードで終了処理を行う
                    cleanup();

                    // アプリケーションを終了する
                    finishAndRemoveTask();
                    isFinished = true;
                }
            }
        }

        /**
         * タッチされた際に呼ばれます。
         */
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            int x = (int)((event.getX() - offsetX) / scale);
            int y = (int)((event.getY() - offsetY) / scale);
            int pointed = event.getPointerCount();
            int delta = y - touchLastY;

            // 描画スレッドと排他制御する
            synchronized(syncObj) {
                switch (event.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:
                        touchStartX = x;
                        touchStartY = y;
                        touchLastY = y;
                        if (pointed == 1)
                            touchLeftDown(x, y);
                        else
                            touchRightDown(x, y);
                        break;
                    case MotionEvent.ACTION_MOVE:
                        touchStartX = x;
                        touchStartY = y;
                        if (delta > LINE_HEIGHT)
                            touchScrollDown();
                        else if (delta < -LINE_HEIGHT)
                            touchScrollUp();
                        touchLastY = y;
                        touchMove(x, y);
                        break;
                    case MotionEvent.ACTION_UP:
                        if (touchCount == 1)
                            touchLeftUp(x, y);
                        else
                            touchRightUp(x, y);
                        break;
                }
            }

            touchCount = pointed;
            return true;
        }
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

    /*
     * ネイティブメソッド
     */

    /** 初期化処理を行います。	*/
    private native void init();

    /** 終了処理を行います。 */
    private native void cleanup();

    /** フレーム処理を行います。 */
    private native boolean frame();

    /** タッチ(左押下)を処理します。 */
    private native void touchLeftDown(int x, int y);

    /** タッチ(右押下)を処理します。 */
    private native void touchRightDown(int x, int y);

    /** タッチ(移動)を処理します。 */
    private native void touchMove(int x, int y);

    /** タッチ(上スクロール)を処理します。 */
    private native void touchScrollUp();

    /** タッチ(下スクロール)を処理します。 */
    private native void touchScrollDown();

    /** タッチ(左解放)を処理します。 */
    private native void touchLeftUp(int x, int y);

    /** タッチ(右解放)を処理します。 */
    private native void touchRightUp(int x, int y);

    /*
     * ndkmain.cのためのユーティリティ
     */

    /** 音声の再生を開始します。 */
    private void playSound(int stream, String fileName, boolean loop) {
        assert stream >= 0 && stream < MIXER_STREAMS;

        stopSound(stream);

        try {
            AssetFileDescriptor afd = getAssets().openFd(fileName);
            player[stream] = new MediaPlayer();
            player[stream].setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
            player[stream].setLooping(loop);
            player[stream].prepare();
            player[stream].start();
        } catch(IOException e) {
            Log.e("Suika", "Failed to load sound " + fileName);
            return;
        }
    }

    /** 音声の再生を停止します。 */
    private void stopSound(int stream) {
        assert stream >= 0 && stream < MIXER_STREAMS;

        if(player[stream] != null) {
            player[stream].stop();
            player[stream].reset();
            player[stream].release();
            player[stream] = null;
        }
    }

    /** 音量を設定します。 */
    private void setVolume(int stream, float vol) {
        assert stream >= 0 && stream < MIXER_STREAMS;
        assert vol >= 0.0f && vol <= 1.0f;

        if(player[stream] != null)
            player[stream].setVolume(vol, vol);
    }

    /*
     * ndkfile.cのためのユーティリティ
     */

    /** Assetあるいはセーブファイルの内容を取得します。 */
    private byte[] getFileContent(String fileName) {
        if(fileName.startsWith("sav/"))
            return getSaveFileContent(fileName.split("/")[1]);
        else
            return getAssetFileContent(fileName);
    }

    /** Assetのファイルの内容を取得します。 */
    private byte[] getAssetFileContent(String fileName) {
        byte[] buf = null;
        try {
            InputStream is = getResources().getAssets().open(fileName);
            buf = new byte[is.available()];
            is.read(buf);
            is.close();
        } catch(IOException e) {
            Log.e("Suika", "Failed to read file " + fileName);
        }
        return buf;
    }

    /** セーブファイルの内容を取得します。 */
    private byte[] getSaveFileContent(String fileName) {
        byte[] buf = null;
        try {
            FileInputStream fis = openFileInput(fileName);
            buf = new byte[fis.available()];
            fis.read(buf);
            fis.close();
        } catch(IOException e) {
        }
        return buf;
    }

    /** セーブファイルを削除します。 */
    private void removeSaveFile(String fileName) {
        File file = new File(fileName);
        file.delete();
    }

    /** セーブファイルの書き込みストリームをオープンします。 */
    private OutputStream openSaveFile(String fileName) {
        try {
            FileOutputStream fos = openFileOutput(fileName, 0);
            return fos;
        } catch(IOException e) {
            Log.e("Suika", "Failed to write file " + fileName);
        }
        return null;
    }

    /** セーブファイルにデータを書き込みます。 */
    private boolean writeSaveFile(OutputStream os, int b) {
        try {
            os.write(b);
            return true;
        } catch(IOException e) {
            Log.e("Suika", "Failed to write file.");
        }
        return false;
    }

    /** セーブファイルの書き込みストリームをクローズします。 */
    private void closeSaveFile(OutputStream os) {
        try {
            os.close();
        } catch(IOException e) {
            Log.e("Suika", "Failed to write file.");
        }
    }
}
