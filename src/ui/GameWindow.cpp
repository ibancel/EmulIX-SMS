#include "ui/GameWindow.h"

#include <QOpenGLShaderProgram>

#include "definitions.h"
#include "Inputs.h"

GameWindow::GameWindow(QWidget* parent)
    : QOpenGLWidget(parent),
      _resolutionWidth{GRAPHIC_WIDTH},
      _resolutionHeight{GRAPHIC_HEIGHT},
      _frameToDraw{GRAPHIC_WIDTH,GRAPHIC_HEIGHT}
{
    setWindowTitle(tr("Game Window"));
    installEventFilter(Inputs::Instance());
}

GameWindow::~GameWindow()
{
    cleanup();
}

void GameWindow::closeEvent(QCloseEvent *event)
{
    Inputs::Instance()->requestStop();
    event->accept();
}

void GameWindow::drawFrame(const Frame& iFrame)
{
    _frameToDraw = iFrame;
}

void GameWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton) {
        if(isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    }

    event->accept();
}

QSize GameWindow::sizeHint() const
{
    return QSize(_resolutionWidth, _resolutionHeight);
}

void GameWindow::setResolution(int iWidth, int iHeight)
{
    _resolutionWidth = iWidth;
    _resolutionHeight = iHeight;
    updateProjection();
}

void GameWindow::cleanup()
{
    if(!_program) {
        return;
    }
    makeCurrent();
    delete _program;
    delete _textureFrame;
    doneCurrent();
}

void GameWindow::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);

    _textureFrame = new QOpenGLTexture(QOpenGLTexture::Target2D);
    _textureFrame->create();
    _textureFrame->setMagnificationFilter(QOpenGLTexture::Filter::Nearest);
    _textureFrame->setSize(256, 192);
    _textureFrame->setFormat(QOpenGLTexture::RGBA8_UNorm);
    _textureFrame->allocateStorage();

    _program = new QOpenGLShaderProgram();
    _program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/ressources/shaders/vertex_main.glsl");
    _program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/ressources/shaders/fragment_main.glsl");
    if(!_program->link()) {
        throw std::runtime_error("Cannot load shaders");
    }
    _program->bind();

    _projMatrixLoc = _program->uniformLocation("projMatrix");
    _textureLocation = _program->uniformLocation("frameTexture");
    _program->setUniformValue(_textureLocation, 0);

    _program->release();
}

void GameWindow::paintGL()
{
    _textureFrame->setData(QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, _frameToDraw.data(FrameOriginFormat::kBottomLeft));
    PixelColor clearColor = _frameToDraw.getBackdropColor();
    glClearColor(clearColor.r, clearColor.g, clearColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _program->bind();
    _textureFrame->bind();
    _program->setUniformValue(_textureLocation, 0);
    _program->setUniformValue(_projMatrixLoc, _projection);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    _textureFrame->release();
    _program->release();
    update();
}

void GameWindow::resizeGL(int iWidth, int iHeight)
{
    updateProjection();
}

// PRIVATE:

void GameWindow::updateProjection()
{
    _projection.setToIdentity();
    _projection.ortho(0.0f, static_cast<float>(_resolutionWidth), static_cast<float>(_resolutionHeight), 0.0f, 0.0f, 1.0f);
    _projection.translate(0.0f, 0.0f, -0.5f);
}
