/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2023, Keiichi Tabata. All rights reserved.
 */

package jp.luxion.suikapro;

import android.annotation.SuppressLint;
import android.content.ContentUris;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.database.Cursor;
import android.graphics.Canvas;
import android.media.MediaPlayer;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.provider.DocumentsContract;
import android.provider.MediaStore;
import android.util.AttributeSet;
import android.util.Log;
import android.opengl.GLSurfaceView;
import android.opengl.GLSurfaceView.Renderer;
import android.opengl.GLES20;
import android.view.MotionEvent;
import android.view.SurfaceView;
import android.view.View;
import android.view.SurfaceHolder;

import androidx.activity.ComponentActivity;
import androidx.activity.result.ActivityResultCallback;
import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.annotation.NonNull;

import java.io.BufferedInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.File;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

/**
 * The main activity.
 */
public class MainActivity extends ComponentActivity {
	//
	// JNI
	//
    static {
		// Load libsuika.so and native*() methods will be available.
		System.loadLibrary("suika");
	}
	public native void nativeInitGame();
	public native void nativeReinitOpenGL();
	public native void nativeRunFrame();
	public native void nativeOnTouchOneDown(int x, int y);
	public native void nativeOnTouchTwoDown(int x, int y);
	public native void nativeOnTouchMove(int x, int y);
	public native void nativeOnTouchScrollUp();
	public native void nativeOnTouchScrollDown();
	public native void nativeOnTouchOneUp(int x, int y);
	public native void nativeOnTouchTwoUp(int x, int y);
	public native void nativeOnMouseLeftDown(int x, int y);
	public native void nativeOnMouseLeftUp(int x, int y);
	public native void nativeOnMouseRightDown(int x, int y);
	public native void nativeOnMouseRightUp(int x, int y);
	public native void nativeOnMouseMove(int x, int y);
	public native int nativeGetIntConfigForKey(String key);

    //
    // Singleton
    //

    public static MainActivity instance;

	//
	// Project
	//

	// The project base path.
	private String basePath;

	//
	// Non-resource View
	//

    // The video view.
    private VideoSurfaceView videoView;

	//
	// Screen Information
	//

    // The scale factor of the game view.
    public float scale;

    // The view port X offset.
	public int offsetX;

    // The view port Y offset.
	public int offsetY;

    // Touch coordiantes.
	public int touchLastY;

    // A count of touched fingers.
	public int touchCount;

	//
	// Running Status
	//

	// A flag to show if the game project is loaded.
	public boolean isGameInitialized;

	// Buttons.
	private boolean isContinuePressed;
	private boolean isNextPressed;
	private boolean isStopPressed;
	private boolean isMovePressed;

    // A flag to show if we are restarting game view after a video playback.
    public boolean resumeFromVideo;

	//
	// MediaPlayer
	//
	public MediaPlayer video;

    // An object to synchronize between the main thread and rendering or video threads. 
    public final Object syncObj = new Object();

	//
	// MainActivity
	//

	@Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        instance = this;

        // Load layout.
		setContentView(R.layout.main);

        // Prepare the video view.
        videoView = new VideoSurfaceView(this);
        Thread videoThread = new Thread(videoView);
        videoThread.start();

		// Start.
		openProject();
    }

	@Override
	public void onDestroy() {
		super.onDestroy();
		isGameInitialized = false;
	}

	public void runFrame() {
		if (isGameInitialized) {
			// Run a native code.
			nativeRunFrame();
		}
	}

	//
	// Project
	//

	private void openProject() {
		// This will call onActivityResult() if a user give us a permission to a folder.
		dirRequest.launch(null);
	}

	final ActivityResultLauncher<Uri> dirRequest = registerForActivityResult(
			new ActivityResultContracts.OpenDocumentTree(),
			new ActivityResultCallback<Uri>() {
				@Override
				public void onActivityResult(Uri uri) {
					if (uri != null) {
						// Get the real path.
						String basePath = getPathFromUri(MainActivity.instance, uri);

						// Append the "Suika2 Pro" directory path/
						basePath = basePath + "/Suika2 Pro/";

						// Copy the template game.
						File fmd = new File(basePath);
						if (!fmd.exists()) {
							fmd.mkdirs();
							extractTemplateGame();
						}

						// Initialize the Suika2 engine.
						nativeInitGame();
					} else {
						// Exit.
						finishAndRemoveTask();
					}
				}
			});

	// From https://stackoverflow.com/questions/32661221/android-cursor-didnt-have-data-column-not-found/33930169#33930169
	public String getPathFromUri(Context context, Uri uri) {
		boolean isAfterKitKat = Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT;
		// DocumentProvider
		Log.e("Suika2 Pro Mobile","uri:" + uri.getAuthority());
		if (isAfterKitKat && DocumentsContract.isDocumentUri(context, uri)) {
			if ("com.android.externalstorage.documents".equals(
					uri.getAuthority())) {// ExternalStorageProvider
				final String docId = DocumentsContract.getDocumentId(uri);
				final String[] split = docId.split(":");
				final String type = split[0];
				if ("primary".equalsIgnoreCase(type)) {
					return Environment.getExternalStorageDirectory() + "/" + split[1];
				}else {
					return "/stroage/" + type +  "/" + split[1];
				}
			}else if ("com.android.providers.downloads.documents".equals(
					uri.getAuthority())) {// DownloadsProvider
				final String id = DocumentsContract.getDocumentId(uri);
				final Uri contentUri = ContentUris.withAppendedId(
						Uri.parse("content://downloads/public_downloads"), Long.valueOf(id));
				return getDataColumn(context, contentUri, null, null);
			}else if ("com.android.providers.media.documents".equals(
					uri.getAuthority())) {// MediaProvider
				final String docId = DocumentsContract.getDocumentId(uri);
				final String[] split = docId.split(":");
				final String type = split[0];
				Uri contentUri = null;
				contentUri = MediaStore.Files.getContentUri("external");
				final String selection = "_id=?";
				final String[] selectionArgs = new String[] {
						split[1]
				};
				return getDataColumn(context, contentUri, selection, selectionArgs);
			}
		}else if ("content".equalsIgnoreCase(uri.getScheme())) {//MediaStore
			return getDataColumn(context, uri, null, null);
		}else if ("file".equalsIgnoreCase(uri.getScheme())) {// File
			return uri.getPath();
		}
		return null;
	}

	public static String getDataColumn(Context context, Uri uri, String selection, String[] selectionArgs) {
		Cursor cursor = null;
		final String[] projection = {
				MediaStore.Files.FileColumns.DATA
		};
		try {
			cursor = context.getContentResolver().query(
					uri, projection, selection, selectionArgs, null);
			if (cursor != null && cursor.moveToFirst()) {
				final int cindex = cursor.getColumnIndexOrThrow(projection[0]);
				return cursor.getString(cindex);
			}
		} finally {
			if (cursor != null)
				cursor.close();
		}
		return null;
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

	//
	// Video playback
	//

	// A handler to start video playback in the main thread.
	@SuppressLint("HandlerLeak")
	public Handler videoStartHandler = new Handler() {
		@Override
		public void handleMessage(@NonNull Message msg) {
			//findViewById(R.id.gameView).setRenderMode(RENDERMODE_WHEN_DIRTY);
			//setContentView(videoView);
			//video.start();
			//super.handleMessage(msg);
		}
	};

	// A handler to stop video playback in the main thread.
	@SuppressLint("HandlerLeak")
	public Handler videoStopHandler = new Handler() {
		@Override
		public void handleMessage(@NonNull Message msg) {
			//resumeFromVideo = true;
			//setContentView(findViewById(R.id.gameView));
			//findViewById(R.id.gameView).setRenderMode(RENDERMODE_CONTINUOUSLY);
			//super.handleMessage(msg);
		}
	};

	// A Hander to invalidate the video view in the main thread.
	@SuppressLint("HandlerLeak")
	public Handler videoLoopHandler = new Handler() {
		@Override
		public void handleMessage(@NonNull Message msg) {
			//videoView.invalidate();
			//super.handleMessage(msg);
		}
	};

	// Called from JNI code.
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

	// Called from JNI code.
	private void stopVideo() {
		if(video != null) {
			video.stop();
			video.reset();
			video.release();
			video = null;
			videoStopHandler.sendEmptyMessage(0);
		}
	}

	// Called from JNI code.
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
}
