#include <QMouseEvent>
#include <QGuiApplication>
#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <ngl/Texture.h>
#include <array>
#include <iostream>

//----------------------------------------------------------------------------------------------------------------------
/// @brief names of the primitives to draw
//----------------------------------------------------------------------------------------------------------------------
const std::array<std::string, 7> s_vboNames = {
    "sphere",
    "cylinder",
    "cone",
    "disk",
    "plane",
    "torus",
    "teapot",
};

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

  m_polyMode = GL_FILL;
  m_primIndex = 0;
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
  ngl::Vec3 from(0, 1, 4);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45, (float)720.0 / 576.0, 0.5, 150);
  // load a frag and vert shaders

  ngl::ShaderLib::createShaderProgram("TextureShader");

  ngl::ShaderLib::attachShader("TextureVertex", ngl::ShaderType::VERTEX);
  ngl::ShaderLib::attachShader("TextureFragment", ngl::ShaderType::FRAGMENT);
  ngl::ShaderLib::loadShaderSource("TextureVertex", "shaders/TextureVert.glsl");
  ngl::ShaderLib::loadShaderSource("TextureFragment", "shaders/TextureFrag.glsl");

  ngl::ShaderLib::compileShader("TextureVertex");
  ngl::ShaderLib::compileShader("TextureFragment");
  ngl::ShaderLib::attachShaderToProgram("TextureShader", "TextureVertex");
  ngl::ShaderLib::attachShaderToProgram("TextureShader", "TextureFragment");

  ngl::ShaderLib::linkProgramObject("TextureShader");
  ngl::ShaderLib::use("TextureShader");

  ngl::Texture texture("textures/ratGrid.png");
  m_textureName = texture.setTextureGL();
  ngl::VAOPrimitives::createSphere("sphere", 1.0, 40);
  ngl::VAOPrimitives::createCylinder("cylinder", 0.5, 5, 30, 30);
  ngl::VAOPrimitives::createCone("cone", 0.5, 1.4f, 20, 20);
  ngl::VAOPrimitives::createDisk("disk", 0.5, 40);
  ngl::VAOPrimitives::createTrianglePlane("plane", 1, 1, 10, 10, ngl::Vec3(0, 1, 0));
  ngl::VAOPrimitives::createTorus("torus", 0.15f, 0.4f, 40, 40);
  // as re-size is not explicitly called we need to do this.
  glViewport(0, 0, width(), height());
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use("TextureShader");

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

  // need to bind the active texture before drawing
  glBindTexture(GL_TEXTURE_2D, m_textureName);
  glPolygonMode(GL_FRONT_AND_BACK, m_polyMode);

  loadMatricesToShader();
  ngl::VAOPrimitives::draw(s_vboNames[m_primIndex]);
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
    m_polyMode = GL_LINE;
    break;
  // turn off wire frame
  case Qt::Key_S:
    m_polyMode = GL_FILL;
    break;
  // show full screen
  case Qt::Key_F:
    showFullScreen();
    break;
  // show windowed
  case Qt::Key_N:
    showNormal();
    break;
  case Qt::Key_Left:
    previousPrim();
    break;
  case Qt::Key_Right:
    nextPrim();
    break;

  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}

void NGLScene::nextPrim()
{

  if (++m_primIndex >= 6)
  {
    m_primIndex = 6;
  }
}

void NGLScene::previousPrim()
{
  if (--m_primIndex < 0)
  {
    m_primIndex = 0;
  }
}
