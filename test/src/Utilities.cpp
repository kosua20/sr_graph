#include <iostream>
#include "Utilities.h"

void Random::seed(){
	std::random_device rd;
	_seed = rd();
	_mt = std::mt19937(_seed);
	_realDist = std::uniform_real_distribution<float>(0,1);
}

void Random::seed(unsigned int seedValue){
	_seed = seedValue;
	_mt = std::mt19937(_seed);
	_realDist = std::uniform_real_distribution<float>(0,1);
}

int Random::Int(int min, int max){
	return (int)(floor(Float() * (max+1 - min)) + min);
}

float Random::Float(){
	return _realDist(_mt);
}

float Random::Float(float min, float max){
	return _realDist(_mt)*(max-min)+min;
}

unsigned int Random::getSeed(){
	return _seed;
}

std::mt19937 Random::_mt;
std::uniform_real_distribution<float> Random::_realDist;
unsigned int Random::_seed;

std::string getGLErrorString(GLenum error) {
	std::string msg;
	switch (error) {
		case GL_INVALID_ENUM:
			msg = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			msg = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			msg = "GL_INVALID_OPERATION";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			msg = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_NO_ERROR:
			msg = "GL_NO_ERROR";
			break;
		case GL_OUT_OF_MEMORY:
			msg = "GL_OUT_OF_MEMORY";
			break;
		default:
			msg = "UNKNOWN_GL_ERROR";
	}
	return msg;
}

int _checkGLError(const char *file, int line){
	GLenum glErr = glGetError();
	if (glErr != GL_NO_ERROR){
		std::cerr << "glError in " << file << " (" << line << ") : " << getGLErrorString(glErr) << std::endl;
		return 1;
	}
	return 0;
}

