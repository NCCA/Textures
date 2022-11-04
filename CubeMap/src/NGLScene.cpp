#include <QMouseEvent>
#include <QGuiApplication>

#include "NGLScene.h"
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleIndexVAO.h>
#include <ngl/ShaderLib.h>
#include <ngl/Texture.h>
#include <ngl/NGLStream.h>

const std::string NGLScene::s_vboNames[8] =
    {
        "sphere",
        "cylinder",
        "cone",
        "disk",
        "plane",
        "torus",
        "teapot",
        "cube"};
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
  m_debug = false;
}

void NGLScene::createSkyBox()
{
  // create a vao as a series of GL_TRIANGLES
  m_skybox = ngl::VAOFactory::createVAO(ngl::simpleIndexVAO, GL_TRIANGLES);
  m_skybox->bind();

  const static GLubyte indices[] = {
      0, 1, 2, 2, 1, 3,
      2, 3, 4, 4, 3, 5,
      4, 5, 6, 6, 5, 7,
      6, 7, 0, 0, 7, 1,
      1, 7, 3, 3, 7, 5,
      6, 0, 4, 4, 0, 2};

  GLfloat vertices[] = {-0.5f, -0.5f, 0.5f,
                        0.5f, -0.5f, 0.5f,
                        -0.5f, 0.5f, 0.5f,
                        0.5f, 0.5f, 0.5f,
                        -0.5f, 0.5f, -0.5f,
                        0.5f, 0.5f, -0.5f,
                        -0.5f, -0.5f, -0.5f,
                        0.5f, -0.5f, -0.5f};

  // in this case we are going to set our data as the vertices above

  m_skybox->setData(ngl::SimpleIndexVAO::VertexData(24 * sizeof(GLfloat), vertices[0], sizeof(indices), &indices[0], GL_UNSIGNED_BYTE, GL_STATIC_DRAW));
  // now we set the attribute pointer to be 0 (as this matches vertIn in our shader)
  m_skybox->setVertexAttributePointer(0, 3, GL_FLOAT, 0, 0);
  m_skybox->setNumIndices(sizeof(indices));
  // now unbind
  m_skybox->unbind();
}

NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
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
  m_project = ngl::perspective(45, (float)720.0f / 576.0f, 0.1f, 350.0f);

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
  ngl::ShaderLib::autoRegisterUniforms("TextureShader");
  ngl::VAOPrimitives::createSphere("sphere", 1.0, 40);
  ngl::VAOPrimitives::createCylinder("cylinder", 0.5f, 5.0f, 30.0f, 30.0f);
  ngl::VAOPrimitives::createCone("cone", 0.5f, 1.4f, 20.0f, 20.0f);
  ngl::VAOPrimitives::createDisk("disk", 0.5f, 40.0f);
  ngl::VAOPrimitives::createTrianglePlane("plane", 1.0f, 1.0f, 10.0f, 10.0f, ngl::Vec3(0.0f, 1.0f, 0.0f));
  ngl::VAOPrimitives::createTorus("torus", 0.15f, 0.4f, 40.0f, 40.0f);
  // as re-size is not explicitly called we need to do this.
  glViewport(0, 0, width(), height());
  m_cubeMap.reset(new CubeMap("textures/right.png", "textures/left.png",
                              "textures/bottom.png", "textures/top.png",
                              "textures/front.png", "textures/back.png"));
  std::string debug[6] = {"textures/DebugRight.png", "textures/DebugLeft.png",
                          "textures/DebugBottom.png", "textures/DebugTop.png",
                          "textures/DebugFront.png", "textures/DebugBack.png"};
  m_cubeMapDebug.reset(new CubeMap(debug));

  createSkyBox();
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib::use("TextureShader");
  ngl::Mat4 M = m_transform.getMatrix() * m_mouseGlobalTX;
  ngl::Mat4 MVP = m_project * m_view * M;
  ngl::ShaderLib::setUniform("MVP", MVP);
  ngl::ShaderLib::setUniform("M", M);
  ngl::ShaderLib::setUniform("cameraPos", ngl::Vec3(M.openGL()[12], M.openGL()[13], M.openGL()[14]));
  ngl::Mat3 normalMatrix = m_view * M;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform("normalMatrix", normalMatrix);
}

void NGLScene::paintGL()
{
  // clear the screen and depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glViewport(0, 0, m_width, m_height);

  // need to bind the active texture before drawing

  // render skybox without depth test
  glDisable(GL_DEPTH_TEST);
  m_transform.setScale(5, 5, 5);
  // m_transform.setRotation(0,0,90);
  m_mouseGlobalTX.identity();
  loadMatricesToShader();
  if (m_debug)
    m_cubeMapDebug->enable();
  else
    m_cubeMap->enable();
  m_skybox->bind();
  m_skybox->draw();
  m_skybox->unbind();
  ngl::ShaderLib::setUniform("reflectOn", 0);
  // now draw object
  glEnable(GL_DEPTH_TEST);
  //  glEnable(GL_CULL_FACE);

  m_transform.reset();
  // Rotation based on the mouse position for our global transform
  auto rotX = ngl::Mat4::rotateX(m_spinXFace);
  auto rotY = ngl::Mat4::rotateY(m_spinYFace);
  // multiply the rotations
  m_mouseGlobalTX = rotY * rotX;
  // add the translations
  m_mouseGlobalTX.m_m[3][0] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[3][1] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[3][2] = m_modelPos.m_z;
  glPolygonMode(GL_FRONT_AND_BACK, m_polyMode);
  ngl::ShaderLib::setUniform("reflectOn", 1);

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
  case Qt::Key_D:
    m_debug ^= true;
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

  if (++m_primIndex >= 7)
  {
    m_primIndex = 7;
  }
}

void NGLScene::previousPrim()
{
  if (--m_primIndex < 0)
  {
    m_primIndex = 0;
  }
}
