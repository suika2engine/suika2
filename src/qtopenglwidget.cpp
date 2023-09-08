/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika 2
 * Copyright (C) 2001-2023, TABATA Keiichi. All rights reserved.
 */

/*
 * [Changes]
 *  2023-09-05 Created.
 */

#include "openglwidget.h"

#include <QApplication>
#include <QMouseEvent>

extern "C" {
#include "glrender.h"
};

OpenGLWidget::OpenGLWidget(QWidget *parent) : QOpenGLWidget(parent)
{
    // No transparency.
    setAttribute(Qt::WA_TranslucentBackground, false);

    // Receive mouse move events.
    setMouseTracking(true);
}

OpenGLWidget::~OpenGLWidget()
{
}

//
// Initialization.
//
void OpenGLWidget::initializeGL()
{
    // Initialize Qt's OpenGL function pointers.
    initializeOpenGLFunctions();

    // Start using the Suika2's OpenGL ES rendering subsystem.
    if(!init_opengl())
        abort();
    m_isOpenGLInitialized = true;
    m_isFirstFrame = true;
}

//
// Frame drawing (every 33ms)
//
void OpenGLWidget::paintGL()
{
    // Guard if not initialized.
    if (!m_isOpenGLInitialized)
        return;

    // On the first frame.
    if (m_isFirstFrame) {
        // Start the Suika2's event handling subsystem.
        if(!on_event_init())
            abort();
        m_isFirstFrame = false;
    }

    // Set the viewport.
    // It's a lazy update because we can't set viewport in resizeGL().
    glViewport(m_viewportX, m_viewportY, m_viewportWidth, m_viewportHeight);

    // Start Suika2's OpenGL rendering.
    opengl_start_rendering();

    // Run an event frame rendering.
    //  - x, y, w and h are ignored
    int x, y, w, h;
    bool cont = on_event_frame(&x, &y, &w, &h);

    // End Suika2's OpenGL rendering.
    opengl_end_rendering();

    // If we reached EOF of a script.
    if (!cont) {
        // Save global variables.
        save_global_data();

        // Save seen flags.
        save_seen();

        // Cleanup the Suika2's event handling subsystem.
        on_event_cleanup();

        // End using the Suika2's OpenGL rendering subsystem.
        cleanup_opengl();
        m_isOpenGLInitialized = false;

        // Exit the app.
        qApp->exit();
    }
}

//
// Resize.
//
void OpenGLWidget::resizeGL(int width, int height)
{
    // Calc the aspect ratio of the game.
    float aspect = (float)conf_window_height / (float)conf_window_width;

    // Get the view size.
    float viewWidthHiDPI = width * devicePixelRatio();
    float viewHeightHiDPI = height * devicePixelRatio();

    // Set the height temporarily with "width-first".
    float useWidthHiDPI = viewWidthHiDPI;
    float useHeightHiDPI = viewWidthHiDPI * aspect;
    m_mouseScale = (float)conf_window_width / (float)width;

    // If height is not enough, determine width with "height-first".
    if(useHeightHiDPI > viewHeightHiDPI) {
        useHeightHiDPI = viewHeightHiDPI;
        useWidthHiDPI = viewHeightHiDPI / aspect;
        m_mouseScale = (float)conf_window_height / (float)height;
    }

    // Calc the OpenGL screen origin.
    float originXHiDPI = (viewWidthHiDPI - useWidthHiDPI) / 2.0f;
    float originYHiDPI = (viewHeightHiDPI - useHeightHiDPI) / 2.0f;

    // Will be applied in a next frame.
    m_viewportX = (int)originXHiDPI;
    m_viewportY = (int)originYHiDPI;
    m_viewportWidth = (int)useWidthHiDPI;
    m_viewportHeight = (int)useHeightHiDPI;

    // Mouse events are not HiDPI.
    m_mouseLeft = (int)(originXHiDPI / devicePixelRatio());
    m_mouseTop = (int)(originYHiDPI / devicePixelRatio());
}

//
// Mouse press event.
//
void OpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    int x = event->position().x();
    int y = event->position().y();

    x = (int)((float)(x - m_mouseLeft) * m_mouseScale);
    y = (int)((float)(y - m_mouseTop) * m_mouseScale);
    if (x < 0 || x > conf_window_width)
        return;
    if (y < 0 || y > conf_window_height)
        return;

    if (event->button() == Qt::LeftButton)
        on_event_mouse_press(MOUSE_LEFT, x, y);
    else if (event->button() == Qt::RightButton)
        on_event_mouse_press(MOUSE_RIGHT, x, y);
}

//
// Mouse release event.
//
void OpenGLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    int x = event->position().x();
    int y = event->position().y();

    x = (int)((float)(x - m_mouseLeft) * m_mouseScale);
    y = (int)((float)(y - m_mouseTop) * m_mouseScale);
    if (x < 0 || x > conf_window_width)
        return;
    if (y < 0 || y > conf_window_height)
        return;

    if (event->button() == Qt::LeftButton)
        on_event_mouse_release(MOUSE_LEFT, x, y);
    else if (event->button() == Qt::RightButton)
        on_event_mouse_release(MOUSE_RIGHT, x, y);
}

//
// Mouse move event.
//
void OpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    int x = event->position().x();
    int y = event->position().y();

    x = (int)((float)(x - m_mouseLeft) * m_mouseScale);
    y = (int)((float)(y - m_mouseTop) * m_mouseScale);
    if (x < 0 || x > conf_window_width)
        return;
    if (y < 0 || y > conf_window_height)
        return;

    on_event_mouse_move(x, y);
}
