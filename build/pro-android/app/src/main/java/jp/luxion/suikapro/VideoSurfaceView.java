/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

package jp.luxion.suikapro;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Canvas;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.SurfaceHolder;

import androidx.annotation.NonNull;


/**
 * The SurfaceView for video playback.
 */
public class VideoSurfaceView extends SurfaceView implements SurfaceHolder.Callback, View.OnTouchListener, Runnable {
	public VideoSurfaceView(Context context) {
		super(context);
		SurfaceHolder holder = getHolder();
		holder.addCallback(this);
		setOnTouchListener(this);
	}

	@Override
	public void surfaceCreated(SurfaceHolder paramSurfaceHolder) {
		if(MainActivity.instance.video != null) {
			SurfaceHolder holder = getHolder();
			MainActivity.instance.video.setDisplay(holder);
			setWillNotDraw(false);
		}
	}

	@Override
	public void surfaceChanged(@NonNull SurfaceHolder paramSurfaceHolder, int paramInt1, int paramInt2, int paramInt3) {
	}

	@Override
	public void surfaceDestroyed(@NonNull SurfaceHolder paramSurfaceHolder) {
	}

	@Override
	public void onDraw(Canvas canvas) {
		if(MainActivity.instance.video != null)
			MainActivity.instance.nativeRunFrame();
	}

	public void run() {
		while(true) {
			if(MainActivity.instance.video != null) {
				new Handler(Looper.getMainLooper()).post(() -> {
					//videoView.invalidate();
					//super.handleMessage(msg);
					//video = null;
				});
			}
			try {
				Thread.sleep(33);
			} catch(InterruptedException e) {
			}
		}
	}

	@Override
	public boolean onTouch(View v, MotionEvent event) {
		// Mutually exclude with the rendering thread.
		synchronized(MainActivity.instance.syncObj) {
			// No need for a coordinate, just skip the video.
			MainActivity.instance.nativeOnTouchOneDown(0, 0);
			MainActivity.instance.nativeOnTouchOneUp(0, 0);
		}
		return true;
	}
}
