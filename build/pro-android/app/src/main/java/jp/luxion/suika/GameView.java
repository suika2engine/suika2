/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
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

/**
 * メインのビューです。ビデオ再生以外で使用されます。
 */
private class MainView extends GLSurfaceView implements View.OnTouchListener, Renderer {
	/** 仮想ビューポートの幅です。 */
	private static final int VIEWPORT_WIDTH = 1280;

	/** 仮想ビューポートの高さです。 */
	private static final int VIEWPORT_HEIGHT = 720;

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
		if(!resumeFromVideo) {
			// JNIコードで初期化処理を実行する
			init();
		} else {
			resumeFromVideo = false;
			reinit();
		}
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
		if(video != null)
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
