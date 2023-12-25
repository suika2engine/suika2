/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

package jp.luxion.suikapro;

import android.content.Context;
import android.util.AttributeSet;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.GLES20;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * The game view.
 */
public class GameView extends GLSurfaceView implements View.OnTouchListener, Renderer {
    //
    // Constants
    //

    // Y-direction pixels to detect touch-move scroll.
    private static final int LINE_HEIGHT = 10;

    /** 仮想ビューポートの幅です。 */
    private static final int VIEWPORT_WIDTH = 1280;

    /** 仮想ビューポートの高さです。 */
    private static final int VIEWPORT_HEIGHT = 720;

    private boolean isGameInitialized;

    /**
     * コンストラクタです。
     */
    public GameView(Context context) {
        super(context);
        init();
    }
    public GameView(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }
    public GameView(Context context, AttributeSet attrs, int defStyleAttr) {
        super(context, attrs);
        init();
    }
    private void init() {
        setFocusable(true);
        setOnTouchListener(this);
        setEGLConfigChooser(8, 8, 8, 8, 0, 0);
        setEGLContextClientVersion(2);
        setRenderer(this);
    }
    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        if(MainActivity.instance.resumeFromVideo)
            MainActivity.instance.resumeFromVideo = false;
        MainActivity.instance.nativeReinitOpenGL();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        int viewportWidth = 1280;
        int viewportHeight = 720;

        // FIXME: support custom window size

        float aspect = (float)viewportHeight / (float)viewportWidth;

        // 1. Width-first.
        float w = width;
        float h = width * aspect;
        MainActivity.instance.scale = w / (float)VIEWPORT_WIDTH;
        MainActivity.instance.offsetX = 0;
        MainActivity.instance.offsetY = (int)((float)(height - h) / 2.0f);

        // 2. Height-first.
        if(h > height) {
            h = height;
            w = height / aspect;
            MainActivity.instance.scale = h / (float)VIEWPORT_HEIGHT;
            MainActivity.instance.offsetX = (int)((float)(width - w) / 2.0f);
            MainActivity.instance.offsetY = 0;
        }

        // Update the viewport.
        GLES20.glViewport(MainActivity.instance.offsetX, MainActivity.instance.offsetY, (int)w, (int)h);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        // Do nothing if a project is not opened.
        if(!MainActivity.instance.isProjectOpened)
            return;

        // Do nothing is a video is playing.
        if(MainActivity.instance.video != null)
            return;

        // Mutually exclude with the main thread's event handlers.
        synchronized(MainActivity.instance.syncObj) {
            // Run native codes.
            if (!isGameInitialized) {
                try {
                    MainActivity.instance.nativeInitGame(MainActivity.instance.basePath);
                    MainActivity.instance.nativeGetScript();
                } catch (Exception e) {
                    Log.e("Suika2 Pro", e.toString());
                }
                isGameInitialized = true;
            } else {
                try {
                    MainActivity.instance.nativeRunFrame();
                } catch (Exception e) {
                    Log.e("Suika2 Pro", e.toString());
                }
            }
        }
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        int x = (int)((event.getX() - MainActivity.instance.offsetX) / MainActivity.instance.scale);
        int y = (int)((event.getY() - MainActivity.instance.offsetY) / MainActivity.instance.scale);
        int pointed = event.getPointerCount();
        int delta = y - MainActivity.instance.touchLastY;

        // 描画スレッドと排他制御する
        synchronized(MainActivity.instance.syncObj) {
            switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                MainActivity.instance.touchLastY = y;
                if (pointed == 1)
                    MainActivity.instance.nativeOnTouchOneDown(x, y);
                else
                    MainActivity.instance.nativeOnTouchTwoDown(x, y);
                break;
            case MotionEvent.ACTION_MOVE:
                if (delta > LINE_HEIGHT)
                    MainActivity.instance.nativeOnTouchScrollDown();
                else if (delta < -LINE_HEIGHT)
                    MainActivity.instance.nativeOnTouchScrollUp();
                MainActivity.instance.touchLastY = y;
                MainActivity.instance.nativeOnTouchMove(x, y);
                break;
            case MotionEvent.ACTION_UP:
                if (MainActivity.instance.touchCount == 1)
                    MainActivity.instance.nativeOnTouchOneUp(x, y);
                else
                    MainActivity.instance.nativeOnTouchTwoUp(x, y);
                break;
            }
        }

        MainActivity.instance.touchCount = pointed;
        return true;
    }
}
