#include "CubeMap.h"
#include <iostream>
CubeMap::CubeMap(const std::string &_right, const std::string &_left, const std::string &_bottom, const std::string &_top, const std::string &_front, const std  ::string &_back)
{
	createCubeMap();
	ngl::Texture t(_right);
	GLuint width=t.getWidth();
	GLuint height=t.getHeight();
	GLuint format=t.getFormat();
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, t.getPixels());
	t.loadImage(_left);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, t.getPixels());

	t.loadImage(_bottom);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, t.getPixels());

	t.loadImage(_top);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, t.getPixels());

	t.loadImage(_front);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, t.getPixels());

	t.loadImage(_back);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, t.getPixels());
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}


CubeMap::CubeMap(std::string *_names)
{
	createCubeMap();
	for(int i = 0; i < 6; i++)
	{

		ngl::Texture t(_names[i]);
		GLuint width=t.getWidth();
		GLuint height=t.getHeight();
		GLuint format=t.getFormat();


		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, t.getPixels());
	}
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

}


void CubeMap::createCubeMap()
{
	glGenTextures(1, &m_id);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLfloat anisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_TRUE);

}
