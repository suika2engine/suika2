/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: t; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2016, TABATA Keiichi. All rights reserved.
 */

package jp.luxion.suika;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.media.MediaPlayer;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class MainActivity extends Activity {
	static {
		System.loadLibrary("suika");
	}

	/** 仮想ビューポートの幅です。 */
	private static final int VIEWPORT_WIDTH = 1280;

	/** 仮想ビューポートの高さです。 */
	private static final int VIEWPORT_HEIGHT = 720;

	/** 60fpsを実現するための待ち時間です。 */
	private static final int DELAY = 0;

	/** タッチをホールドと判断する時間[ms]です */
	private static final int HOLD_TIME_MIN = 200;

	/** タッチをホールドと判断する時間[ms]です */
	private static final int HOLD_TIME_MAX = 1000;

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

	/** フレームを処理するためのHandlerです。 */
	private Handler handler;

	/** 背景ビットマップです。 */
	private Bitmap backBitmap;

	/** ビューポートサイズを1としたときの、レンダリング先の拡大率です。 */
	private float scale;

	/** レンダリング先のXオフセットです。 */
	private int offsetX;

	/** レンダリング先のXオフセットです。 */
	private int offsetY;

	/** レンダリング先の幅です。 */
	private int width;

	/** レンダリング先の高さです。 */
	private int height;

	/** invalidateされたかを表します。 */
	private boolean isInvalidated;

	/** 前回タッチされていた箇所の数です。 */
	private int lastPointedCount;

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
		view.setBackgroundColor(0xff000000);
		setContentView(view);

		// JNIコードで初期化処理を実行する
		backBitmap = init();
		if (backBitmap == null)
			throw new RuntimeException("onCreate() returned false");
	}

	/**
	 * ビューです。
	 */
	private class MainView extends View implements View.OnTouchListener {
		/**
		 * コンストラクタです。
		 */
		public MainView(Context context) {
			super(context);
			setFocusable(true);
			setOnTouchListener(this);
		}

		/**
		 * ビューのサイズが決定した際に呼ばれます。
		 */
		@Override
		protected void onSizeChanged(int w, int h, int oldw, int oldh) {
			// ビューポートの拡大率を求める
			float scaleX = (float) w / VIEWPORT_WIDTH;
			float scaleY = (float) h / VIEWPORT_HEIGHT;
			scale = scaleX > scaleY ? scaleY : scaleX;

			// 実際に描画する領域のサイズを求める
			width = (int) (VIEWPORT_WIDTH * scale);
			height = (int) (VIEWPORT_HEIGHT * scale);

			// 実際に描画する領域のオフセットを求める
			offsetX = (w - width) / 2;
			offsetY = (h - height) / 2;
		}

		/**
		 * 表示される際に呼ばれます。
		 */
		@Override
		protected void onAttachedToWindow() {
			handler = new Handler() {
				public void handleMessage(Message msg) {
					// ビューがデタッチされた場合は処理しない
					if (handler == null)
						return;

					// JNIコードでフレームを処理する
					if (!frame()) {
						// 終了する
						finish();
					} else {
						// 更新領域がない場合、タイマをセットする
						if (!isInvalidated)
							sendEmptyMessageDelayed(0, DELAY);
					}
				}
			};
			handler.sendEmptyMessage(0);
			super.onAttachedToWindow();
		}

		/**
		 * 表示されなくなる際に呼ばれます。
		 */
		@Override
		protected void onDetachedFromWindow() {
			handler = null;
			super.onDetachedFromWindow();

			// 終了処理を行う
			cleanup();
		}

		/**
		 * ビューへの描画を行う際に呼ばれます。
		 */
		@Override
		protected void onDraw(Canvas canvas) {
			isInvalidated = false;

			// 描画を行う
			Paint paint = new Paint();
			Rect srcRect = new Rect(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
			Rect dstRect = new Rect(offsetX, offsetY, offsetX + width, offsetY + height);
			canvas.drawBitmap(backBitmap, srcRect, dstRect, paint);

			// フレーム処理時刻を更新する
			handler.sendEmptyMessageDelayed(0, DELAY);
		}

		/**
		 * タッチされた際に呼ばれます。
		 */
		@Override
		public boolean onTouch(View v, MotionEvent event) {
			int x = (int) (event.getX() / scale);
			int y = (int) (event.getY() / scale);
			int pointed = event.getPointerCount();
			long hold = event.getEventTime() - event.getDownTime();

			switch(event.getActionMasked()) {
			case MotionEvent.ACTION_MOVE:
//			case MotionEvent.ACTION_DOWN:
//			case MotionEvent.ACTION_CANCEL:
				if(pointed == 1) {
					Log.i("Suika", "touchMove");
					touchMove(x, y);
				} else {
					Log.i("Suika", "touchScroll " + y);
					touchScroll(x, y);
				}
				break;
			case MotionEvent.ACTION_UP:
				if(lastPointedCount == 1) {
					if(hold >= HOLD_TIME_MIN && hold <= HOLD_TIME_MAX) {
						Log.i("Suika", "touchHold");
						touchHold(x, y);
					} else {
						Log.i("Suika", "touchUp");
						touchUp(x, y);
					}
				}
				break;
			}

			lastPointedCount = pointed;
			return true;
		}
	}

	/*
	 * ネイティブメソッド
	 */

	/** 初期化処理を行います。	*/
	private native Bitmap init();

	/** 終了処理を行います。 */
	private native void cleanup();

	/** フレーム処理を行います。 */
	private native boolean frame();

	/** タッチ(移動)を処理します。 */
	private native void touchMove(int x, int y);

	/** タッチ(解放)を処理します。 */
	private native void touchUp(int x, int y);

	/** タッチ(ホールド)を処理します。 */
	private native void touchHold(int x, int y);

	/** タッチ(スクロール)を処理します。 */
	private native void touchScroll(int x, int y);

	/*
	 * ndkmain.cのためのユーティリティ
	 */

	/** 再描画を行います。 */
	private void invalidateView() {
		isInvalidated = true;
		view.invalidate();
	}

	/** 音声の再生を開始します。 */
	private void playSound(int stream, String fileName, boolean loop) {
		assert stream > 0 && stream < MIXER_STREAMS;

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
		assert stream > 0 && stream < MIXER_STREAMS;

		if(player[stream] != null) {
			player[stream].stop();
			player[stream].reset();
			player[stream].release();
			player[stream] = null;
		}
	}

	/** 音量を設定します。 */
	private void setVolume(int stream, float vol) {
		assert stream > 0 && stream < MIXER_STREAMS;
		assert vol >= 0.0f && vol <= 1.0f;

		if(player[stream] != null)
			player[stream].setVolume(vol, vol);
	}

	/*
	 * ndkimage.cのためのユーティリティ
	 */

	/**
	 * ビットマップを作成します。
	 */
	private Bitmap createBitmap(int w, int h) {
		return Bitmap.createBitmap(w, h, Bitmap.Config.ARGB_8888);
	}

	/**
	 * assetからビットマップを作成します。
	 */
	private Bitmap loadBitmap(String fileName) {
		try {
			InputStream is = getResources().getAssets().open(fileName);
			return BitmapFactory.decodeStream(is);
		} catch (IOException e) {
			Log.e("Suika", "Failed to load image " + fileName);
			return null;
		}
	}

	/**
	 * Bitmapに矩形を描画します。
	 */
	private void drawRect(Bitmap bm, int x, int y, int w, int h, int color) {
		Canvas canvas = new Canvas(bm);
		Paint paint = new Paint();
		Rect rect = new Rect(x, y, x + w, y + h);
		paint.setColor(color);
		paint.setStyle(Paint.Style.FILL);
		paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
		canvas.drawRect(rect, paint);
	}

	/**
	 * BitmapにBitmapを描画します。
	 */
	private void drawBitmapAlpha(Bitmap dst, int dx, int dy, Bitmap src, int w, int h, int sx, int sy, int alpha) {
		Canvas canvas = new Canvas(dst);
		Rect dstRect = new Rect(dx, dy, dx + w, dy + h);
		Rect srcRect = new Rect(sx, sy, sx + w, sy + h);
		Paint paint = new Paint();
		paint.setAlpha(alpha);
		canvas.drawBitmap(src, srcRect, dstRect, paint);
	}

	/**
	 * BitmapにBitmapを描画します。
	 */
	private void drawBitmapCopy(Bitmap dst, int dx, int dy, Bitmap src, int w, int h, int sx, int sy) {
		Canvas canvas = new Canvas(dst);
		Rect dstRect = new Rect(dx, dy, dx + w, dy + h);
		Rect srcRect = new Rect(sx, sy, sx + w, sy + h);
		Paint paint = new Paint();
		paint.setAlpha(255);
		paint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
		canvas.drawBitmap(src, srcRect, dstRect, paint);
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

	/*
	 * ndkglyph.cのためのユーティリティ
	 */

	/** Bitmapに文字を描画します。 */
	private int drawGlyph(Bitmap bm, int x, int y, int size, int color, int codepoint) {
		String s = "" + (char)codepoint;
		Canvas canvas = new Canvas(bm);
		Paint paint = new Paint();
		paint.setTextSize(size);
		paint.setStyle(Paint.Style.FILL);
		paint.setTextAlign(Paint.Align.LEFT);
		paint.setColor(0xff000000 | color);
		paint.setAntiAlias(true);
		Paint.FontMetrics fontMetrics = paint.getFontMetrics();
		canvas.drawText(s, x, y - fontMetrics.ascent, paint);
		return (int)paint.measureText(s);
	}

	/** 文字を描画したときの幅を取得します。 */
	private int getGlyphWidth(int size, int codepoint) {
		String s = "" + (char)codepoint;
		Paint paint = new Paint();
		paint.setTextSize(size);
		paint.setAntiAlias(true);
		return (int)paint.measureText(s);
	}

	/** 文字を描画したときの幅を取得します。 */
	private int getGlyphHeight(int size, int codepoint) {
		Paint paint = new Paint();
		paint.setTextSize(size);
		paint.setAntiAlias(true);
		Paint.FontMetrics fontMetrics = paint.getFontMetrics();
		return (int)(-fontMetrics.ascent + fontMetrics.descent);
	}
}
