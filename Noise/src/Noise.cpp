// simple Perlin noise class cobbled together from Computer Graphics OpenGL by F,S Hill
// and Texturing and Modeling Ebert et-al
// also thanks to Ian Stephenson for help and debuggin tips
// more work needs to be done to add interpolated noise functions and improve the
// aliasing of the textures but it is ok for OpenGL use

#include "Noise.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <numeric>
#include <ngl/Random.h>

GLfloat Noise::sqr(GLfloat _in)
{
	return _in*_in;
}


void Noise :: resetTables()
{
	int i;

	for(i=0; i<256; ++i)
	{
		int which=int(ngl::Random::randomPositiveNumber(256));
		unsigned char tmp=m_index[which];
		m_index[which]=m_index[i];
		m_index[i]=tmp;
	}
	for(i=0; i<256; ++i)
	{
		m_noiseTable[i]=ngl::Random::randomPositiveNumber(32767.99f);
	}

}

Noise :: Noise()
{
	int i;
	std::iota(std::begin(m_index),std::end(m_index),0);

	for(i=0; i<256; i++)
	{
		int which=int(ngl::Random::randomPositiveNumber(256));
		unsigned char tmp=m_index[which];
		m_index[which]=m_index[i];
		m_index[i]=tmp;
	}

	m_noiseTable = std::make_unique<float []>(256);
	for(i=0; i<256; ++i)
	{
		m_noiseTable[i]=ngl::Random::randomPositiveNumber(32767.99f);
	}
}

GLfloat Noise::latticeNoise(int i, int j, int k)
{
	#define PERM(x) m_index[(x)&255]
	#define m_index(ix,iy,iz) PERM( (ix) + PERM((iy)+PERM(iz)))
	return m_noiseTable[m_index(i,j,k)];
}

GLfloat Noise::noise(GLfloat scale, ngl::Vec3 p)
{

	#define Lerp(F, A,B) A + F * ( B - A )
	GLfloat d[2][2][2];
	ngl::Vec3 pp;
	pp.m_x=p.m_x * scale ;
	pp.m_y=p.m_y * scale ;
	pp.m_z=p.m_z * scale ;

	long ix = (long) pp.m_x;
	long iy = (long) pp.m_y;
	long iz = (long) pp.m_z;
	GLfloat tx,ty,tz,x0,x1,x2,x3,y0,y1;
	tx=pp.m_x-ix; ty=pp.m_y-iy; tz=pp.m_z-iz;


	for(int k=0; k<=1; k++)
	{
		for(int j=0; j<=1; j++)
		{
			for(int i=0; i<=1; i++)
			{
				d[k][j][i]=latticeNoise(ix+i,iy+j,iz+k);
			}
		}
	}
	x0=Lerp(tx, d[0][0][0],d[0][0][1]);
	x1=Lerp(tx, d[0][1][0],d[0][1][1]);
	x2=Lerp(tx, d[1][0][0],d[1][0][1]);
	x3=Lerp(tx, d[1][1][0],d[1][1][1]);
	y0=Lerp(ty, x0,x1);
	y1=Lerp(ty, x2,x3);
	return Lerp(tz,y0,y1);

}

GLfloat Noise::undulate(GLfloat x)
{
	if(x<-0.4f) return 0.15f + 2.857f * sqr(x+0.75f);
	else if(x < 0.4f) return 0.95f - 2.8125f * sqr(x);
	else return 0.26f + 2.666f * sqr(x-0.7f);
}



GLfloat Noise :: turbulance(GLfloat s, ngl::Vec3 p)
{
	float val= (noise(s,p)/2.0f) + (noise(2.0f*s,p)/4.0f) + (noise(4.0f*s,p)/8.0f) + (noise(8.0f*s,p)/16.0f);
	return val;
}

GLfloat Noise::marble(GLfloat A, GLfloat s, ngl::Vec3 p)
{
	float val= undulate(cosf(2.0f*static_cast<float>(M_PI)*p.m_z+A*turbulance(s,p)));

	return val;
}
GLfloat Noise::marble(GLfloat strength, ngl::Vec3 p)
{
	float turb=turbulance(10,p);
	float val=sin(6*p.m_z+strength*turb);
	return undulate(val);
}
