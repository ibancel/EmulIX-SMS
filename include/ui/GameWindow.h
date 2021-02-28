#pragma once

#include <QCloseEvent>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLWidget>

#include "Frame.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class GameWindow : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	GameWindow(QWidget* parent = nullptr);
	virtual ~GameWindow();

	virtual void closeEvent(QCloseEvent* event) override;
	void drawFrame(const Frame& iFrame);
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
	virtual QSize sizeHint() const override;
	void setResolution(int iWidth, int iHeight);

public slots:
	void cleanup();

protected:
	virtual void initializeGL() override;
	virtual void paintGL() override;
	virtual void resizeGL(int iWidth, int iHeight) override;

private:
	Frame _frameToDraw;
	QOpenGLShaderProgram* _program;
	QMatrix4x4 _projection;
	int _projMatrixLoc;
	int _resolutionWidth;
	int _resolutionHeight;
	GLuint _textureLocation;
	QOpenGLTexture* _textureFrame;

	void updateProjection();
};
