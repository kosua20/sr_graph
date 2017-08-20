#ifndef Utilities_h
#define Utilities_h
#include <gl3w/gl3w.h>
#include <random>
#include <string>

class Random {
public:
	
	static void seed();
	
	static void seed(unsigned int seedValue);
	
	static int Int(int min, int max);
	
	static float Float();
	
	static float Float(float min, float max);
	
	static unsigned int getSeed();
	
private:
	
	static unsigned int _seed;
	static std::mt19937 _mt;
	static std::uniform_real_distribution<float> _realDist;
};

/// This macro is used to check for OpenGL errors with access to the file and line number where the error is detected.
#define checkGLError() _checkGLError(__FILE__, __LINE__)

/// Converts a GLenum error number into a human-readable string.
std::string getGLErrorString(GLenum error);

/// Check if any OpenGL error has been detected and log it.
int _checkGLError(const char *file, int line);

#endif
