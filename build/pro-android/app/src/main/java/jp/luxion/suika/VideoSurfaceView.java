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

/*
 * ビデオ再生用のSurfaceViewです。
 */
class VideoSurfaceView extends SurfaceView implements SurfaceHolder.Callback, View.OnTouchListener, Runnable {
	public VideoSurfaceView(Context context) {
		super(context);
		SurfaceHolder holder = getHolder();
		holder.addCallback(this);
		setOnTouchListener(this);
	}

	@Override
	public void surfaceCreated(SurfaceHolder paramSurfaceHolder) {
		if(video != null) {
			SurfaceHolder holder = videoView.getHolder();
			video.setDisplay(holder);
			setWillNotDraw(false);
		}
	}

	@Override
	public void surfaceChanged(SurfaceHolder paramSurfaceHolder, int paramInt1, int paramInt2, int paramInt3) {
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder paramSurfaceHolder) {
	}

	@Override
	public void onDraw(Canvas canvas) {
		if(video != null) {
			// JNIコードでフレームを処理する
			if (!frame()) {
				// JNIコードで終了処理を行う
				cleanup();

				// アプリケーションを終了する
				finishAndRemoveTask();
				isFinished = true;
			}
		}
	}

	public void run() {
		while(true) {
			if(video != null)
				videoLoopHandler.sendEmptyMessage(0);
			try {
				Thread.sleep(33);
			} catch(InterruptedException e) {
			}
		}
	}

	/**
	 * タッチされた際に呼ばれます。
	 */
	@Override
	public boolean onTouch(View v, MotionEvent event) {
		// 描画スレッドと排他制御する
		synchronized(syncObj) {
			touchLeftDown(0, 0);
			touchLeftUp(0, 0);
		}
		return true;
	}
}

/*
 * ネイティブメソッド
 */

/** 初期化処理を行います。 */
private native void init();

/** 再初期化処理を行います。ビデオ再生から復帰します。 */
private native void reinit();

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

/** 動画の再生を開始します。 */
private void playVideo(String fileName, boolean isSkippable) {
	if (video != null) {
		video.stop();
		video = null;
	}

	try {
		AssetFileDescriptor afd = getAssets().openFd("mov/" + fileName);
		video = new MediaPlayer();
		video.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
		video.prepare();
		videoStartHandler.sendEmptyMessage(0);
	} catch(IOException e) {
		Log.e("Suika", "Failed to play video " + fileName);
	}
}

/** 動画の再生を開始します。 */
private void stopVideo() {
	if(video != null) {
		video.stop();
		video.reset();
		video.release();
		video = null;
		videoStopHandler.sendEmptyMessage(0);
	}
}

/** 動画の再生状態を取得します。 */
private boolean isVideoPlaying() {
	if(video != null) {
		int pos = video.getCurrentPosition();
		if (pos == 0)
			return true;
		if (video.isPlaying()) {
			return true;
		}
		video.stop();
		videoStopHandler.sendEmptyMessage(0);
		video = null;
	}
	return false;
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
