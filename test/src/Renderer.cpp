#include <stdio.h>
#include <iostream>
#include <vector>
#include <cstring>
#include "Utilities.h"
#include "Renderer.h"

// Include sr_graph here.
#define SRG_IMPLEMENTATION_SR_GRAPH
#include "../../sr_graph.h"


int gr1, gr2, gr3, gr4;

Renderer::~Renderer(){}

Renderer::Renderer(int width, int height){

	// Initialize the timer.
	_timer = glfwGetTime();
	// Initialize random generator;
	Random::seed();
	// Setup projection matrix.
	_width = width;
	_height = height;
	
	// Query the renderer identifier, and the supported OpenGL version.
	const GLubyte* renderer = glGetString(GL_RENDERER);
	const GLubyte* version = glGetString(GL_VERSION);
	std::cout << "Renderer: " << renderer << std::endl;
	std::cout << "OpenGL version supported: " << version << std::endl;
	checkGLError();

	// GL options
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CCW);
	glCullFace(GL_BACK);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	checkGLError();
	
	// Create graphs.
	std::vector<float> xs11 = { -198.0f, -121.0f, -112.0f, -51.0f, -2.0f, 18.0f, 49.5f, 75.0f, 79.0f, 94.0f };
	std::vector<float> ys11 = { 112.0f, 121.0f, 202.0f, 238.0f, 198.0f, 197.0f, 220.0f, 289.0f, 143.0f, 138.0f };
	std::vector<float> xs12 = { -190.0f, -172.0f, -168.0f, -154.0f, -115.0f, -98.0f, -84.0f, -33.0f, 7.0f, 67.0f, 71.0f, 100.0f };
	std::vector<float> ys12 = { 289.0f, 232.0f, 225.0f, 200.0f, 212.0f, 180.0f, 184.0f, 133.0f, 218.0f, 218.0f, 176.0f, 120.0f };
	
	gr1 = sr_graph::setup(-200.0f, 100.0f, 100.0f, 300.0f, (float)width/(float)height, 0.1f, 1.0f, 1.0f, 1.0f);
	sr_graph::add_axes(gr1, 0.005f, 0.1f, 0.1f, 0.1f, false);
	//sr_graph::add_grid(gr1, 10.0f, 20.0f, 0.0025f, 0.7f, 0.7f, 0.7f, false);
	sr_graph::add_curve(gr1, xs11, ys11, 0.007f, 0.9f, 0.1f, 0.2f);
	sr_graph::add_points(gr1, xs11, ys11, 0.03f, 0.9f, 0.1f, 0.2f);
	sr_graph::add_curve(gr1, xs12, ys12, 0.007f, 0.6f, 0.8f, 0.3f);
	sr_graph::add_points(gr1, xs12, ys12, 0.03f, 0.6f, 0.8f, 0.3f);
	

	
	std::vector<float> ys2;
	for(int i = 0; i < 1000; ++i){
		ys2.push_back(Random::Float());
	}
	
	gr2 = sr_graph::setup(0.0f, 1.0f, 0.0f, 100.0f, (float)width/(float)height, 0.2f, 0.1f, 0.1f, 0.1f);
	sr_graph::add_axes(gr2, 0.01f, 0.9f, 0.9f, 0.9f, true);
	sr_graph::add_grid(gr2, 0.0f, 10.0f, 0.0025f, 0.5f, 0.5f, 0.5f, false);
	sr_graph::add_hist(gr2, 25, ys2, 0.0f, 0.35f, 0.95f, 0.48f);
	
	
	
	std::vector<float> xs3;
	std::vector<float> ys3;
	for(float theta = -M_PI; theta <= M_PI; theta += 2.0f*M_PI/(100.0f)){
		xs3.push_back(theta);
		ys3.push_back(sin(theta));
	}
	
	
	gr3 = sr_graph::setup(-M_PI, M_PI, -1.1f, 1.1f, (float)width/(float)height, 0.05f, 0.2f, 0.4f, 0.9f);
	sr_graph::add_axes(gr3, 0.005f, 0.9f, 0.9f, 0.9f, false);
	sr_graph::add_grid(gr3, 1.0f, 0.2f, 0.0025f, 0.8f, 0.8f, 0.8f, true);
	sr_graph::add_curve(gr3, xs3, ys3, 0.01f, 1.0f, 0.53f, 0.1f);
	
	
	std::vector<float> xs4;
	std::vector<float> ys4;
	for(float x = 0.0f; x < 10.0f; x += 0.1f){
		xs4.push_back(x);
		ys4.push_back(std::pow(x,2));
	}
	
	/*for(float x = 0.0; x < 10.0f; x+=0.2f){
		for(float y = 0.0; y < 10.0f; y+=0.2f){
			if(fabs(cos(x) - cos(y)) <= 0.1f){
				xs4.push_back(x);
				ys4.push_back(y);
			}
		}
	}*/
	
	gr4 = sr_graph::setup(0.0f, 10.0f, 0.0f, 10.0f, (float)width/(float)height, 0.05f, 0.5f, 0.0f, 0.0f);
	sr_graph::add_grid(gr4, 1.0f, 1.0f, 0.0025f, 0.7f, 0.2f, 0.0f, false); // @TODO: investigate when true
	sr_graph::add_points(gr4, xs4, ys4, 0.02f, 1.0f, 1.0f, 1.0f);
	
}


void Renderer::draw() {
	
	// Compute the time elapsed since last frame
	_timer = glfwGetTime();
	
	
	std::vector<float> ys2;
	for(int i = 0; i < 1000; ++i){
		ys2.push_back(Random::Float());
	}
	sr_graph::update_hist(gr2, 0, ys2);
	
	
	std::vector<float> xs3;
	std::vector<float> ys3;
	for(float theta = -M_PI; theta <= M_PI; theta += 2.0f*M_PI/(100.0f)){
		xs3.push_back(theta);
		ys3.push_back(sin(theta*3.5f+4.0*_timer));
		// Funny: ys3.push_back(sin(theta*(0.5+(cos(_timer)+1.0))*2.0+_timer));
	}
	sr_graph::update_curve(gr3, 0, xs3, ys3);
	sr_graph::update_points(gr3, 0, xs3, ys3);

	std::vector<float> xs4;
	std::vector<float> ys4;
	const float exponent = (sin(_timer)+1.0)*2.0;
	for(float x = 0.0f; x < 10.0f; x += 0.1f){
		xs4.push_back(x);
		ys4.push_back(std::pow(x,exponent));
	}
	sr_graph::update_points(gr4, 0, xs4, ys4);
	
	// Set screen viewport.
	glViewport(0, 0, GLsizei(_width), GLsizei(_height));
	
	// Draw the fullscreen quad
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
	GLsizei halfWidth = (GLsizei)floor(_width*0.5);
	GLsizei halfHeight = (GLsizei)floor(_height*0.5);
	float ratio = (float)_width/(float)_height;
	
	glViewport(0, halfHeight, halfWidth, halfHeight);
	sr_graph::draw(gr1, ratio);
	glViewport(0, 0, halfWidth, halfHeight);
	sr_graph::draw(gr2, ratio);
	glViewport(halfWidth, 0, halfWidth, halfHeight);
	sr_graph::draw(gr3, ratio);
	glViewport(halfWidth, halfHeight, halfWidth, halfHeight);
	sr_graph::draw(gr4, ratio);
	
	
	// Update timer
	_timer = glfwGetTime();
}


void Renderer::clean() const {
	// Clean objects.
	sr_graph::free();
}


void Renderer::resize(int width, int height){
	//Update the size of the viewport.
	glViewport(0, 0, width, height);
	// Update the projection matrix.
	_width = width;
	_height = height;
}


