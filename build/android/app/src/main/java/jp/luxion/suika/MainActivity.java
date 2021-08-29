/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

package jp.luxion.suika;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.content.res.Resources;
import android.media.MediaPlayer;
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
import android.view.WindowManager;
import android.graphics.Point;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

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

	/** タッチスクロールの1行分の移動距離です */
	private static final int LINE_HEIGHT = 10;

	/** ミキサのストリーム数です。 */
	private static final int MIXER_STREAMS = 3;

	/** ミキサのBGMストリームです。 */
	private static final int BGM_STREAM = 0;

	/** ミキサのVOICEストリームです。 */
	private static final int VOICE_STREAM = 1;

	/** ミキサのSEストリームです。 */
	private static final int SE_STREAM = 2;

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

	/** BE/VOICE/SEのMediaPlayerです。 */
	private MediaPlayer[] player = new MediaPlayer[MIXER_STREAMS];

	/**
	 * アクティビティが作成されるときに呼ばれます。
	 */
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// フルスクリーンにする
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
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

			// JNIコードでフレームを処理する
			if(!frame()) {
				// JNIコードで終了処理を行う
				cleanup();

				// アプリケーションを終了する
				finish();
				isFinished = true;
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

			switch(event.getActionMasked()) {
			case MotionEvent.ACTION_DOWN:
				touchStartX = x;
				touchStartY = y;
				touchLastY = y;
				touchMove(x, y);
				break;
			case MotionEvent.ACTION_MOVE:
				touchStartX = x;
				touchStartY = y;
				if(delta > LINE_HEIGHT)
					touchScrollDown();
				else if(delta < -LINE_HEIGHT)
					touchScrollUp();
				touchLastY = y;
				touchMove(x, y);
				break;
			case MotionEvent.ACTION_UP:
				if(touchCount == 1)
					touchLeftClick(x, y);
				else
					touchRightClick(x, y);
				break;
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

	/*
	 * ネイティブメソッド
	 */

	/** 初期化処理を行います。	*/
	private native void init();

	/** 終了処理を行います。 */
	private native void cleanup();

	/** フレーム処理を行います。 */
	private native boolean frame();

	/** タッチ(移動)を処理します。 */
	private native void touchMove(int x, int y);

	/** タッチ(上スクロール)を処理します。 */
	private native void touchScrollUp();

	/** タッチ(下スクロール)を処理します。 */
	private native void touchScrollDown();

	/** タッチ(左クリック)を処理します。 */
	private native void touchLeftClick(int x, int y);

	/** タッチ(右クリック)を処理します。 */
	private native void touchRightClick(int x, int y);

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
		if (fileName.startsWith("sav/"))
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
