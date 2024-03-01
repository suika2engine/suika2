/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: t; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

/*
 * An Android port.
 * [Changes]
 *  - 2016-08-06 Created.
 *  - 2023-12-25 Modernized.
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
import android.os.Looper;
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
import android.view.WindowManager.LayoutParams;

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
    //
    // Please change these constants for your game.
    //
    private static final String APP_NAME = "Suika2"; // window.title
    private static final int VIEWPORT_WIDTH = 1280; // window.width
    private static final int VIEWPORT_HEIGHT = 720; // window.height

    //
    // JNI
    //
    static {
        System.loadLibrary("suika");
    }
    private native void nativeInitGame();
    private native void nativeReinitOpenGL();
    private native void nativeCleanup();
    private native boolean nativeRunFrame();
    private native boolean nativeOnPause();
    private native boolean nativeOnResume();
    private native void nativeOnTouchStart(int x, int y, int points);
    private native void nativeOnTouchMove(int x, int y);
    private native void nativeOnTouchEnd(int x, int y, int points);

    //
    // Constants
    //

    // Amount of touch-move to determine scroll.
    private static final int LINE_HEIGHT = 10;

    //
    // Variables
    //

    // The main view.
    private MainView view;

    // The viewport scale factor.
    private float scale;

    // The viewport offset X.
    private int offsetX;

    // The viewport offset Y.
    private int offsetY;

    // The last touched Y coordinate.
    private int touchLastY;

    // Finger count of a last touch.
    private int touchCount;

    // A flag that indicates if the game is loaded.
    private boolean isLoaded;

    // A flag that indicates if the game is finished.
    private boolean isFinished;

    // MediaPlayer for a video playback.
    private MediaPlayer video;

    // The view for video playback.
    private VideoSurfaceView videoView;

    // A flag that indicates if we are right after back from video playback.
    private boolean resumeFromVideo;

    // The synchronization object for the mutual exclusion between the main thread and the rendering thread.
    private final Object syncObj = new Object();

    @Override
    @SuppressWarnings("deprecation")
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        isFinished = false;

        // Do full screen settings.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R)
            getWindow().getDecorView().getWindowInsetsController().setSystemBarsBehavior(WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE);
        getWindow().addFlags(LayoutParams.FLAG_FULLSCREEN);
        requestWindowFeature(Window.FEATURE_NO_TITLE);

        // Create the main view.
        view = new MainView(this);
        setContentView(view);

        // Prepare the video view.
        videoView = new VideoSurfaceView(this);
        Thread videoThread = new Thread(videoView);
        videoThread.start();
    }

    @Override
    public void onPause() {
        super.onPause();
        nativeOnPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        nativeOnResume();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if(!isFinished) {
            synchronized (syncObj) {
                nativeCleanup();
            }
            isFinished = true;
        }
    }

    private class MainView extends GLSurfaceView implements View.OnTouchListener, Renderer {
        public MainView(Context context) {
            super(context);

            setFocusable(true);
            setOnTouchListener(this);
            setEGLConfigChooser(8, 8, 8, 8, 0, 0);
            setEGLContextClientVersion(2);
            setRenderer(this);
        }

        @Override
        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            if(!resumeFromVideo) {
                synchronized (syncObj) {
                    nativeInitGame();
					isLoaded = true;
                }
            } else {
                resumeFromVideo = false;
                synchronized (syncObj) {
                    nativeReinitOpenGL();
                }
            }
        }

        @Override
        public void onSurfaceChanged(GL10 gl, int width, int height) {
            // Get the aspect ratir.
            float aspect = (float)VIEWPORT_HEIGHT / (float)VIEWPORT_WIDTH;

            // Width-first.
            float w = width;
            float h = width * aspect;
            scale = w / (float)VIEWPORT_WIDTH;
            offsetX = 0;
            offsetY = (int)((float)(height - h) / 2.0f);

            // Height-first.
            if(h > height) {
                h = height;
                w = height / aspect;
                scale = h / (float)VIEWPORT_HEIGHT;
                offsetX = (int)((float)(width - w) / 2.0f);
                offsetY = 0;
            }

            GLES20.glViewport(offsetX, offsetY, (int)w, (int)h);
        }

        @Override
        public void onDrawFrame(GL10 gl) {
            if(!isLoaded)
                return;
            if(isFinished)
                return;
            if(video != null)
                return;

            boolean ret;
            synchronized(syncObj) {
                ret = nativeRunFrame();
                if(!ret)
                    nativeCleanup();
            }
            if(!ret) {
                if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
                    finishAndRemoveTask();
                isFinished = true;
            }
        }

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            int x = (int)((event.getX() - offsetX) / scale);
            int y = (int)((event.getY() - offsetY) / scale);
            int pointed = event.getPointerCount();
            int delta = y - touchLastY;

            synchronized(syncObj) {
                switch (event.getActionMasked()) {
                    case MotionEvent.ACTION_DOWN:
						nativeOnTouchBegin(x, y, pointed);
                        break;
                    case MotionEvent.ACTION_MOVE:
                        nativeOnTouchMove(x, y);
                        break;
                    case MotionEvent.ACTION_UP:
						nativeOnTouchEnd(x, y, touchCount);
                        break;
                }
            }

            touchCount = pointed;
            return true;
        }
    }

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
				if(!isLoaded)
					return;

                boolean ret = true;
                synchronized (syncObj) {
                    ret = nativeRunFrame();
                    if(!ret)
                        nativeCleanup();
                }
                if (!ret) {
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP)
                        finishAndRemoveTask();
                    isFinished = true;
                }
            }
        }

        public void run() {
            //noinspection InfiniteLoopStatement
            do {
                if (video != null) {
                    new Handler(Looper.getMainLooper()).post(() -> {
                        videoView.invalidate();
                    });
                }
                try {
                    //noinspection BusyWait
                    Thread.sleep(33);
                } catch (InterruptedException ignored) {
                }
            } while (true);
        }

        @Override
        public boolean onTouch(View v, MotionEvent event) {
            synchronized(syncObj) {
                nativeOnTouchOneDown(0, 0);
                nativeOnTouchOneUp(0, 0);
            }
            return true;
        }
    }

    //
    // Bridges
    //  - We name methods that are called from JNI code "bridge*()".
    //

	private void bridgePlayVideo(String fileName, boolean isSkippable) {
		if (video != null) {
			video.stop();
			video = null;
		}
        try {
            AssetFileDescriptor afd = getAssets().openFd("mov/" + fileName);
            video = new MediaPlayer();
            video.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
            video.prepare();
            new Handler(Looper.getMainLooper()).post(() -> {
                view.setRenderMode(RENDERMODE_WHEN_DIRTY);
                setContentView(videoView);
                video.start();
            });
        } catch(IOException e) {
            Log.e(APP_NAME, "Failed to play video " + fileName);
        }
    }

    private void bridgeStopVideo() {
        if(video != null) {
            video.stop();
            video.reset();
            video.release();
            video = null;
            new Handler(Looper.getMainLooper()).post(() -> {
                resumeFromVideo = true;
                setContentView(view);
                view.setRenderMode(RENDERMODE_CONTINUOUSLY);
            });
        }
    }

    private boolean bridgeIsVideoPlaying() {
        if(video != null) {
            int pos = video.getCurrentPosition();
            if (pos == 0)
                return true;
            if (video.isPlaying())
                return true;
            video.stop();
            video = null;
            new Handler(Looper.getMainLooper()).post(() -> {
                resumeFromVideo = true;
                setContentView(view);
                view.setRenderMode(RENDERMODE_CONTINUOUSLY);
            });
        }
		return false;
    }

    private byte[] bridgeGetFileContent(String fileName) {
        if(fileName.startsWith("sav/"))
            return getSaveFileContent(fileName.split("/")[1]);
        else
            return getAssetFileContent(fileName);
    }

    private byte[] getSaveFileContent(String fileName) {
        byte[] buf = null;
        try {
            FileInputStream fis = openFileInput(fileName);
            buf = new byte[fis.available()];
            //noinspection ResultOfMethodCallIgnored
            fis.read(buf);
            fis.close();
        } catch(IOException ignored) {
        }
        return buf;
    }

    private byte[] getAssetFileContent(String fileName) {
        byte[] buf = null;
        try {
            InputStream is = getResources().getAssets().open(fileName);
            buf = new byte[is.available()];
            //noinspection ResultOfMethodCallIgnored
            is.read(buf);
            is.close();
        } catch(IOException e) {
            Log.e(APP_NAME, "Failed to read file " + fileName);
        }
        return buf;
    }

    private void bridgeRemoveSaveFile(String fileName) {
        File file = new File(fileName);
        //noinspection ResultOfMethodCallIgnored
        file.delete();
    }

    private OutputStream bridgeOpenSaveFile(String fileName) {
        try {
            return openFileOutput(fileName, 0);
        } catch(IOException e) {
            Log.e(APP_NAME, "Failed to write file " + fileName);
        }
        return null;
    }

    private boolean bridgeWriteSaveFile(OutputStream os, int b) {
        try {
            os.write(b);
            return true;
        } catch(IOException e) {
            Log.e(APP_NAME, "Failed to write file.");
        }
        return false;
    }

    private void bridgeCloseSaveFile(OutputStream os) {
        try {
            os.close();
        } catch(IOException e) {
            Log.e(APP_NAME, "Failed to write file.");
        }
    }
}
