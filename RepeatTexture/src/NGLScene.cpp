#include <QMouseEvent>
#include <QGuiApplication>
#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Texture.h>
#include <iostream>

//#include <QGLWidget>
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for x/y translation with mouse movement
//----------------------------------------------------------------------------------------------------------------------
const static float INCREMENT = 0.01f;
//----------------------------------------------------------------------------------------------------------------------
/// @brief the increment for the wheel zoom
//----------------------------------------------------------------------------------------------------------------------
const static float ZOOM = 0.1f;

NGLScene::NGLScene()
{
  // re-size the widget to that of the parent (in this case the GLFrame passed in on construction)
  m_rotate = false;
  // mouse rotation values set to 0
  m_spinXFace = 0;
  m_spinYFace = 0;
  setTitle("Simple OpenGL Texture");
  m_speed = 0;
  m_repeat = 1;
}

void NGLScene::loadTexture()
{
  ngl::Texture t("textures/Road.png");
  m_textureName = t.setTextureGL();
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
  // remove the texture now we are done
  glDeleteTextures(1, &m_textureName);
}

void NGLScene::resizeGL(int _w, int _h)
{
  m_project = ngl::perspective(45.0f, (float)_w / _h, 0.05f, 350.0f);
  m_width = _w * devicePixelRatio();
  m_height = _h * devicePixelRatio();
}

void NGLScene::initializeGL()
{
  // we must call this first before any other GL commands to load and link the
  // gl commands from the lib, if this is not done program will crash
  ngl::NGLInit::initialize();

  glClearColor(0.4f, 0.4f, 0.4f, 1.0f); // Grey Background
  // enable depth testing for drawing
  glEnable(GL_DEPTH_TEST);
  // enable multisampling for smoother drawing
  glEnable(GL_MULTISAMPLE);

  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from(0, 1, 2);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45, (float)720.0 / 576.0, 0.5, 160);
  // now to load the shader and set the values
  // load a frag and vert shaders

  ngl::ShaderLib::createShaderProgram("TextureShader");

  ngl::ShaderLib::attachShader("SimpleVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("SimpleFragment", ngl::ShaderType::FRAGMENT);
  ngl::ShaderLib::loadShaderSource("SimpleVertex", "shaders/TextureVert.glsl");
  ngl::ShaderLib::loadShaderSource("SimpleFragment", "shaders/TextureFrag.glsl");

  ngl::ShaderLib::compileShader("SimpleVertex");
  ngl::ShaderLib::compileShader("SimpleFragment");
  ngl::ShaderLib::attachShaderToProgram("TextureShader", "SimpleVertex");
  ngl::ShaderLib::attachShaderToProgram("TextureShader", "SimpleFragment");

  ngl::ShaderLib::linkProgramObject("TextureShader");
  ngl::ShaderLib::use("TextureShader");

  // now pass the modelView and projection values to the shader
  ngl::ShaderLib::setUniform("xMultiplyer", m_repeat);
  ngl::ShaderLib::setUniform("yOffset", 0.0f);

  loadTexture();
  glEnable(GL_DEPTH_TEST); // for removal of hidden surfaces
  ngl::VAOPrimitives::createTrianglePlane("plane", 10, 10, 20, 20, ngl::Vec3(0, 1, 0));
  // as re-size is not explicitly called we need to do this.
  glViewport(0, 0, width(), height());
  // timer needs starting after gl context created
  m_texTimer = startTimer(50);
}

void NGLScene::loadMatricesToShader()
{

  ngl::Mat4 MVP = m_project * m_view * m_mouseGlobalTX;
  ngl::ShaderLib::setUniform("MVP", MVP);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_width, m_height);
  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;

  ngl::ShaderLib::use("TextureShader");
  glBindTexture(GL_TEXTURE_2D, m_textureName);

  ngl::ShaderLib::setUniform("xMultiplyer", m_repeat);

  // now we bind back our vertex array object and draw

  loadMatricesToShader();
  ngl::VAOPrimitives::draw("plane");
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseMoveEvent(QMouseEvent *_event)
{
// note the method buttons() is the button state when event was called
// this is different from button() which is used to check which button was
// pressed when the mousePress/Release event is generated
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  auto position = _event->position();
#else
  auto position = _event->pos();
#endif
  if (m_rotate && _event->buttons() == Qt::LeftButton)
  {
    int diffx = position.x() - m_origX;
    int diffy = position.y() - m_origY;
    m_spinXFace += (float)0.5f * diffy;
    m_spinYFace += (float)0.5f * diffx;
    m_origX = position.x();
    m_origY = position.y();
    update();
  }
  // right mouse translate code
  else if (m_translate && _event->buttons() == Qt::RightButton)
  {
    int diffX = (int)(position.x() - m_origXPos);
    int diffY = (int)(position.y() - m_origYPos);
    m_origXPos = position.x();
    m_origYPos = position.y();
    m_modelPos.m_x += INCREMENT * diffX;
    m_modelPos.m_y -= INCREMENT * diffY;
    update();
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mousePressEvent(QMouseEvent *_event)
{
// this method is called when the mouse button is pressed in this case we
// store the value where the maouse was clicked (x,y) and set the Rotate flag to true
#if QT_VERSION > QT_VERSION_CHECK(6, 0, 0)
  auto position = _event->position();
#else
  auto position = _event->pos();
#endif
  if (_event->button() == Qt::LeftButton)
  {
    m_origX = position.x();
    m_origY = position.y();
    m_rotate = true;
  }
  // right mouse translate mode
  else if (_event->button() == Qt::RightButton)
  {
    m_origXPos = position.x();
    m_origYPos = position.y();
    m_translate = true;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::mouseReleaseEvent(QMouseEvent *_event)
{
  // this event is called when the mouse button is released
  // we then set Rotate to false
  if (_event->button() == Qt::LeftButton)
  {
    m_rotate = false;
  }
  // right mouse translate mode
  if (_event->button() == Qt::RightButton)
  {
    m_translate = false;
  }
}

//----------------------------------------------------------------------------------------------------------------------
void NGLScene::wheelEvent(QWheelEvent *_event)
{

  // check the diff of the wheel position (0 means no change)
  if (_event->angleDelta().x() > 0)
  {
    m_modelPos.m_z += ZOOM;
  }
  else if (_event->angleDelta().x() < 0)
  {
    m_modelPos.m_z -= ZOOM;
  }
  update();
}
//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent(QKeyEvent *_event)
{
  // this method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch (_event->key())
  {
  // escape key to quite
  case Qt::Key_Escape:
    QGuiApplication::exit(EXIT_SUCCESS);
    break;
  // turn on wirframe rendering
  case Qt::Key_W:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case Qt::Key_S:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;

  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
    break;
  case Qt::Key_Minus:
    incrementSpeed();
    break;
  case Qt::Key_Plus:
    decrementSpeed();
    break;
  case Qt::Key_Left:
    incrementRepeat();
    break;
  case Qt::Key_Right:
    decrementRepeat();
    break;

  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}

void NGLScene::timerEvent(QTimerEvent *_event)
{
  // re-draw GL
  if (_event->timerId() == m_texTimer)
  {
    static float offset = 0.0;

    offset += m_speed;
    ngl::ShaderLib::use("TextureShader");
    ngl::ShaderLib::setUniform("yOffset", offset);
  }
  update();
}
