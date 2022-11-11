#include <QMouseEvent>
#include <QGuiApplication>
#include "NGLScene.h"
#include <ngl/Transformation.h>
#include <ngl/NGLInit.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/ShaderLib.h>
#include <memory>
#include <string>
#include <iostream>

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
  m_fpsTimer = startTimer(0);
  m_fps = 0;
  m_frames = 0;
  // m_timer.start();
  m_polyMode = GL_FILL;
}

void NGLScene::loadTexture()
{
  QImage image;
  bool loaded = image.load("textures/crate.bmp");
  if (loaded == true)
  {
    int width = image.width();
    int height = image.height();

    std::unique_ptr<unsigned char[]> data = std::make_unique<unsigned char[]>(width * height * 3);
    size_t index = 0;
    QRgb colour;
    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        colour = image.pixel(x, y);

        data[index++] = qRed(colour);
        data[index++] = qGreen(colour);
        data[index++] = qBlue(colour);
      }
    }

    glGenTextures(1, &m_textureName);
    glBindTexture(GL_TEXTURE_2D, m_textureName);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data.get());

    glGenerateMipmap(GL_TEXTURE_2D); //  Allocate the mipmaps
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

//----------------------------------------------------------------------------------------------------------------------
/// @brief create a cube and stuff it into a VBO on the GPU
/// @param[in] _scale a scale factor for the unit vertices
void NGLScene::createCube(GLfloat _scale)
{

  // vertex coords array
  GLfloat vertices[] = {
      -1, 1, -1, 1, 1, -1, 1, -1, -1, -1, 1, -1, -1, -1, -1, 1, -1, -1, // back
      -1, 1, 1, 1, 1, 1, 1, -1, 1, -1, -1, 1, 1, -1, 1, -1, 1, 1,       // front
      -1, 1, -1, 1, 1, -1, 1, 1, 1, -1, 1, 1, 1, 1, 1, -1, 1, -1,       // top
      -1, -1, -1, 1, -1, -1, 1, -1, 1, -1, -1, 1, 1, -1, 1, -1, -1, -1, // bottom
      -1, 1, -1, -1, 1, 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, -1, 1, 1, // left
      1, 1, -1, 1, 1, 1, 1, -1, -1, 1, -1, -1, 1, -1, 1, 1, 1, 1,       // left

  };
  GLfloat texture[] = {
      0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, // back
      0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, // front
      0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, // top
      0, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, // bottom
      1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, // left
      1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, // right

  };

  std::cout << sizeof(vertices) / sizeof(GLfloat) << "\n";
  // first we scale our vertices to _scale
  for (uint i = 0; i < sizeof(vertices) / sizeof(GLfloat); ++i)
  {
    vertices[i] *= _scale;
  }

  // first we create a vertex array Object
  glGenVertexArrays(1, &m_vaoID);

  // now bind this to be the currently active one
  glBindVertexArray(m_vaoID);
  // now we create two VBO's one for each of the objects these are only used here
  // as they will be associated with the vertex array object
  GLuint vboID[2];
  glGenBuffers(2, &vboID[0]);
  // now we will bind an array buffer to the first one and load the data for the verts
  glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
  // now we bind the vertex attribute pointer for this object in this case the
  // vertex data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(0);
  // now we repeat for the UV data using the second VBO
  glBindBuffer(GL_ARRAY_BUFFER, vboID[1]);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texture) * sizeof(GLfloat), texture, GL_STATIC_DRAW);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(1);
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
  ngl::Vec3 from(0, 2, 4);
  ngl::Vec3 to(0, 0, 0);
  ngl::Vec3 up(0, 1, 0);
  m_view = ngl::lookAt(from, to, up);
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_project = ngl::perspective(45, (float)720.0 / 576.0, 0.5, 150);
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
  ngl::ShaderLib::setUniform("tex", 0);

  createCube(0.2f);
  loadTexture();
  m_text = std::make_unique<ngl::Text>("fonts/Arial.ttf", 14);
  m_text->setScreenSize(width(), height());
}

void NGLScene::loadMatricesToShader()
{
  ngl::Mat4 MVP = m_project * m_view *
                  m_mouseGlobalTX *
                  m_transform.getMatrix();

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
  // now we bind back our vertex array object and draw
  glBindVertexArray(m_vaoID); // select first VAO

  int instances = 0;
  // need to bind the active texture before drawing
  glBindTexture(GL_TEXTURE_2D, m_textureName);
  glPolygonMode(GL_FRONT_AND_BACK, m_polyMode);

  for (float z = -34; z < 35; z += 0.5)
  {
    for (float x = -34; x < 35; x += 0.5)
    {
      m_transform.reset();
      {
        m_transform.setRotation(x * 20.0f, (x * z) * 40.0f, z * 2.0f);
        m_transform.setPosition(x, 0.49f, z);
        loadMatricesToShader();
        ++instances;
        glDrawArrays(GL_TRIANGLES, 0, 36); // draw object
      }
    }
  }
  // calculate and draw FPS
  ++m_frames;
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  m_text->setColour(1.0f, 1.0f, 0.0f);
  m_text->renderText(10, 700, fmt::format("Texture and Vertex Array Object {} instances Demo {} fps", instances, m_fps));
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
  default:
    break;
  }
  // finally update the GLWindow and re-draw
  // if (isExposed())
  update();
}

void NGLScene::timerEvent(QTimerEvent *_event)
{
  // if(_event->timerId() == m_fpsTimer)
  // 	{
  // 		if( m_timer.elapsed() > 1000.0)
  // 		{
  // 			m_fps=m_frames;
  // 			m_frames=0;
  // 			m_timer.restart();
  // 		}
  // 	 }
  // re-draw GL
  update();
}
