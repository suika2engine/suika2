/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

package jp.luxion.suikapro;

import static android.opengl.GLSurfaceView.RENDERMODE_CONTINUOUSLY;
import static android.opengl.GLSurfaceView.RENDERMODE_WHEN_DIRTY;

import static androidx.activity.result.ActivityResultCallerKt.registerForActivityResult;

import static java.security.AccessController.getContext;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.AssetFileDescriptor;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.media.MediaPlayer;
import android.net.Uri;
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
import android.widget.LinearLayout;

import androidx.activity.result.ActivityResult;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.RequiresApi;
import androidx.appcompat.app.AppCompatActivity;

import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.File;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class MainActivity extends AppCompatActivity {

	//
	// JNI
	//

    static {
        System.loadLibrary("suika");
    }
	private native void nativeInitGame();
	private native void nativeReinitOpenGL();
	private native void nativeRunFrame();
	private native void nativeOnTouchOneDown(int x, int y);
	private native void nativeOnTouchTwoDown(int x, int y);
	private native void nativeOnTouchMove(int x, int y);
	private native void nativeOnTouchScrollUp();
	private native void nativeOnTouchScrollDown();
	private native void nativeOnTouchOneUp(int x, int y);
	private native void nativeOnTouchTwoUp(int x, int y);
	private native void nativeOnMouseLeftDown(int x, int y);
	private native void nativeOnMouseLeftUp(int x, int y);
	private native void nativeOnMouseRightDown(int x, int y);
	private native void nativeOnMouseRightUp(int x, int y);
	private native void nativeOnMouseMove(int x, int y);
	private native int nativeGetIntConfigForKey(String key);

	//
	// Constants
	//

    // Y-direction pixels to detect touch-move scroll.
    private static final int LINE_HEIGHT = 10;

	//
	// Project
	//

	// The project base path.
	private String basePath;

	//
	// Views
	//

    // The game rendering view.
    private GameView gameView;

    // The video view.
    private VideoSurfaceView videoView;

	// The editor view.
	private EditView editView;

	//
	// Screen Information
	//

    // The scale factor of the game view.
    private float scale;

    // The view port X offset.
    private int offsetX;

    // The view port Y offset.
    private int offsetY;

    // Touch coordiantes.
    private int touchStartX, touchStartY, touchLastY;

    // A count of touched fingers.
    private int touchCount;

	//
	// Running Status
	//

	// A flag to show if the game project is loaded.
	private boolean isGameInitialized;

	// Buttons.
	private boolean isContinuePressed;
	private boolean isNextPressed;
	private boolean isStopPressed;
	private boolean isMovePressed;

    // A flag to show if we are restarting game view after a video playback.
    private boolean resumeFromVideo;

	//
	// MediaPlayer
	//
	private MediaPlayer video;

	//
	// Threads and Handlers
	//

    // A thread to call frame() periodically when a video is playing.
    private Thread videoThread;

    // A handler to start video playback in the main thread.
    private Handler videoStartHandler;

	// A handler to stop video playback in the main thread.
    private Handler videoStopHandler;

    // A Hander to invalidate the video view in the main thread.
    private Handler videoLoopHandler;

    // An object to synchronize between the main thread and rendering or video threads. 
    private Object syncObj = new Object();

	//
	// MainActivity
	//

    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN)
    @SuppressWarnings("deprecation")
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Create the game and editviews.
        LinearLayout linearLayout = new LinearLayout(this);
		linearLayout.setOrientation(LinearLayout.VERTICAL);
		setContentView(linearLayout);
        gameView = new GameView(this);
		gameView.setLayoutParams(new WindowManager.LayoutParams(-1, -1));
		editView = new EditView(this);
		editView.setLayoutParams(new WindowManager.LayoutParams(EditView.VIEW_WIDTH, -1));
		linearLayout.addView(gameView);
		linearLayout.addView(editView);

        // Prepare the video view.
        videoView = new VideoSurfaceView(this);
        videoThread = new Thread(videoView);
        videoThread.start();

        // Create handlers for video playback.
        videoStartHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                gameView.setRenderMode(RENDERMODE_WHEN_DIRTY);
                setContentView(videoView);
                video.start();
                super.handleMessage(msg);
            }
        };
        videoStopHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                resumeFromVideo = true;
                setContentView(gameView);
                gameView.setRenderMode(RENDERMODE_CONTINUOUSLY);
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

		// Start.
		initProject();
		nativeInitGame();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        if(!isGameInitialized)
			isGameInitialized = false;
    }

	//
	// Frame execution
	//

	/**
	 * Run a frame.
	 */
	public void runFrame() {
		if (isGameInitialized) {
			// Run a native code.
			nativeRunFrame();
		}
	}

	//
	// Project
	//

	private void initProject() {
		openProject();

		// If a game does not exist, copy the template game.
		if (!new File(basePath + "conf/config.txt").exists() || !new File(basePath + "txt/init.txt").exists())
			extractTemplateGame();
	}

	final ActivityResultLauncher<Uri> dirRequest = registerForActivityResult(
			new ActivityResultContracts.OpenDocumentTree(),
			new ActivityResultCallback<Uri>() {
				@Override
				public void onActivityResult(Uri uri) {
					if (uri != null) {
						// Persist the permission.
						//getContentResolver().takePersistableUriPermission(uri, Intent.AC;

						// Get the base path.
						basePath = uri.getPath() + "/";
					} else {
						// Exit.
						finishAndRemoveTask();
					}
				}
			});

	private void openProject() {
		dirRequest.launch(null);
	}

	private void extractTemplateGame() {
		try {
			InputStream is = getResources().getAssets().open("game.zip");
			ZipInputStream zis = new ZipInputStream(new BufferedInputStream(is));
			ZipEntry ze;
			byte[] buffer = new byte[1024];
			int count;
			while((ze = zis.getNextEntry()) != null) {
				String filename = ze.getName();
				if (ze.isDirectory()) {
					File fmd = new File(basePath + filename);
					fmd.mkdirs();
					continue;
				}

				FileOutputStream fout = new FileOutputStream(basePath + filename);
				while ((count = zis.read(buffer)) != -1) {
					fout.write(buffer, 0, count);
				}
				fout.close();
				zis.closeEntry();
			}
			zis.close();
		} catch(IOException e) {
			Log.e("Suika2 Pro Mobile", "Failed to read file.");
			finishAndRemoveTask();
		}
	}

	/*
	 * Utilities for the NDK code.
	 */

	/** Start video playback. */
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
			Log.e("Suika2 Pro Mobile", "Failed to play video " + fileName);
		}
	}

	/** Stop video playback. */
	private void stopVideo() {
		if(video != null) {
			video.stop();
			video.reset();
			video.release();
			video = null;
			videoStopHandler.sendEmptyMessage(0);
		}
	}

	/** Get video playback status. */
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

	/**
	 * The game view.
	 */
	private class GameView extends GLSurfaceView implements View.OnTouchListener, Renderer {
		/** 仮想ビューポートの幅です。 */
		private static final int VIEWPORT_WIDTH = 1280;

		/** 仮想ビューポートの高さです。 */
		private static final int VIEWPORT_HEIGHT = 720;

		/**
		 * コンストラクタです。
		 */
		public GameView(Context context) {
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
			if(resumeFromVideo) {
				resumeFromVideo = false;
				if (isGameInitialized)
					nativeReinitOpenGL();
			}
		}

		/**
		 * ビューのサイズが決定した際に呼ばれます。
		 */
		@Override
		public void onSurfaceChanged(GL10 gl, int width, int height) {
			int viewportWidth = 1280;
			int viewportHeight = 720;
			if (isGameInitialized) {
				viewportWidth = nativeGetIntConfigForKey("window.width");
				viewportHeight = nativeGetIntConfigForKey("window.height");
			}

			float aspect = (float)viewportHeight / (float)viewportWidth;

			// 1. Width-first.
			float w = width;
			float h = width * aspect;
			scale = w / (float)VIEWPORT_WIDTH;
			offsetX = 0;
			offsetY = (int)((float)(height - h) / 2.0f);

			// 2. Height-first.
			if(h > height) {
				h = height;
				w = height / aspect;
				scale = h / (float)VIEWPORT_HEIGHT;
				offsetX = (int)((float)(width - w) / 2.0f);
				offsetY = 0;
			}

			// Update the viewport.
			GLES20.glViewport(offsetX, offsetY, (int)w, (int)h);
		}

		@Override
		public void onDrawFrame(GL10 gl) {
			if(!isGameInitialized)
				return;
			if(video != null)
				return;

			// Mutually exclude with the main thread's event handlers.
			synchronized(syncObj) {
				nativeRunFrame();
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
						nativeOnTouchOneDown(x, y);
					else
						nativeOnTouchTwoDown(x, y);
					break;
				case MotionEvent.ACTION_MOVE:
					touchStartX = x;
					touchStartY = y;
					if (delta > LINE_HEIGHT)
						nativeOnTouchScrollDown();
					else if (delta < -LINE_HEIGHT)
						nativeOnTouchScrollUp();
					touchLastY = y;
					nativeOnTouchMove(x, y);
					break;
				case MotionEvent.ACTION_UP:
					if (touchCount == 1)
						nativeOnTouchOneUp(x, y);
					else
						nativeOnTouchTwoUp(x, y);
					break;
				}
			}

			touchCount = pointed;
			return true;
		}
	}

	/**
	 * The SurfaceView for video playback.
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
			if(video != null)
				runFrame();
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

		@Override
		public boolean onTouch(View v, MotionEvent event) {
			// Mutually exclude with the rendering thread.
			synchronized(syncObj) {
				// No need for a coordinate, just skip the video.
				nativeOnTouchOneDown(0, 0);
				nativeOnTouchOneUp(0, 0);
			}
			return true;
		}
	}

	/**
	 * The editor view.
	 */
	private class EditView extends View {
		public static final int VIEW_WIDTH = 440;

		public EditView(Context context) {
			super(context);
		}
	}
}
