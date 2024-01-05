/* -*- coding: utf-8; tab-width: 4; indent-tabs-mode: nil; -*- */

/*
 * Suika2
 * Copyright (C) 2001-2024, Keiichi Tabata. All rights reserved.
 */

#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QColor>

QT_BEGIN_NAMESPACE
namespace Ui { class OpenGLWidget; }
QT_END_NAMESPACE

class OpenGLWidget : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    virtual ~OpenGLWidget();

    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

signals:

public slots:

private:
    // Whether the Suika2's rendering subsystem initialized.
    bool m_isOpenGLInitialized;

    // Whether the first frame is going to be processed.
    bool m_isFirstFrame;

    // OpenGL rendering scale and offsets for mouse coordinate calculations.
    int m_viewportX;
    int m_viewportY;
    int m_viewportWidth;
    int m_viewportHeight;

    // View mouse scale and origin.
    float m_mouseScale;
    int m_mouseLeft;
    int m_mouseTop;
};

#endif // OPENGLWIDGET_H
