#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QColor>
#include <QOpenGLFunctions_1_0>

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
    float m_scale;
    float m_viewportX;
    float m_viewportY;
    float m_viewportWidth;
    float m_viewportHeight;
};

#endif // OPENGLWIDGET_H
