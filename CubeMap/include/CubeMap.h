#ifndef CUBEMAP_H__
#define CUBEMAP_H__

#include <string>
#include <ngl/Image.h>

class CubeMap
{
public :
  CubeMap(const std::string &_right, const std::string &_left,
          const std::string &_bottom, const std::string &_top,
          const std::string &_front, const std::string &_back);

  CubeMap(std::string *_names);

  ~CubeMap(){  glDeleteTextures(1,&m_id);}
  void enable(){glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);   glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);}
  void disable(){glBindTexture(GL_TEXTURE_CUBE_MAP, 0);   glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);}
  GLuint getTexID(){return m_id;}
private :
  GLuint m_id;
  void createCubeMap();
};


#endif
