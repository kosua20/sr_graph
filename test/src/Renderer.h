#ifndef Renderer_h
#define Renderer_h

class Renderer {

public:


	~Renderer();

	/// Init function
	Renderer(int width, int height);

	/// Draw function
	void draw();

	/// Clean function
	void clean() const;

	/// Handle screen resizing
	void resize(int width, int height);
	
	
private:
	
	double _timer;
	int _width;
	int _height;
};

#endif
