#ifndef SRG_INCLUDE_SR_GRAPH_H
#define SRG_INCLUDE_SR_GRAPH_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

namespace sr_graph {
	
	extern int setup(const float ratio, const float margins, const float bg_r, const float bg_g, const float bg_b);
	
	extern void add_axes(const unsigned int graph_id, const float minx, const float maxx, const float miny, const float maxy, const float stepx, const float stepy, const float axis_r, const float axis_g, const float axis_b, const float lines_r, const float lines_g, const float lines_b, const bool axisOnSide);
	
	extern void add_curve(const unsigned int graph_id, const std::vector<float> & xs, const std::vector<float> & ys, const float color_r, const float color_g, const float color_b);
	
	extern void add_hist(const unsigned int graph_id, const unsigned int bins, const std::vector<float> & ys, const float color_r, const float color_g, const float color_b);
	
	extern void draw(const unsigned int graph_id, const float ratio);
	
	extern void free(const unsigned int graph_id);
	/*
	options(line width, point size) others ? or move them to setup ?
	draw(width, height, scaling) // do we really need to know the viewport ?
	labels() ?
	clean() ?
	global init ?*/
	
}


#endif

#ifdef SRG_IMPLEMENTATION_SR_GRAPH

#define SRG_DEBUG
namespace sr_graph {
	
	void _internalSetup();
	GLuint _setDataBuffer(const float * data, const unsigned int count);
	void _getLine(const float p0x, const float p0y, const float p1x, const float p1y, const float w, const float ratio, std::vector<float> & points);
	
	static bool _isInit = false;
	static unsigned int _nextGraphId = 0;
	
	
	typedef struct {
		float r;
		float g;
		float b;
	} Color;
	
	typedef struct {
		Color color;
		GLuint vid;
		GLsizei vcount;
	} Axis;
	
	typedef struct {
		Color color;
		float ratio;
		float margin;
		Axis hAxis;
		Axis vAxis;
	} Graph;

	typedef struct {
		GLuint pid;
		GLuint cid;
		GLuint rid;
	} _InternalState ;
	static _InternalState _state;

	static std::vector<Graph> _graphs;
	
	
	int setup(const float ratio, const float margins, const float bg_r, const float bg_g, const float bg_b){
		// If we haven't initialized our GL programs, do it.
		if (!_isInit) {
			_internalSetup();
		}
		
		Graph graph;
		graph.color = {bg_r, bg_g, bg_b };
		graph.ratio = fabs(ratio);
		graph.margin = fmin(1.0f, fmax(0.0f, fabs(margins*2.0)));
		
		_graphs.push_back(graph);
		
		int graphId = _nextGraphId;
		++_nextGraphId;
		return graphId;
		
	}

    void add_axes(const unsigned int graph_id, const float minx, const float maxx, const float miny, const float maxy, const float stepx, const float stepy, const float axis_r, const float axis_g, const float axis_b, const float lines_r, const float lines_g, const float lines_b, const bool axisOnSide){
		
		if(graph_id >= _nextGraphId){
			return;
		}
		Graph & graph = _graphs[graph_id];
		/// Generate data for axis.
		// Horizontal axis: from (-margin, -margin) to (margin, -margin)
		float hx0 = -1.0f+graph.margin;
		float hx1 = 1.0f-graph.margin;
		
		float hy0 = -1.0f+graph.margin;
		float hy1 = -1.0f+graph.margin;
		
		
		std::vector<float> hAxisData;
		_getLine(hx0, hy0, hx1, hy1, 0.025f, graph.ratio, hAxisData);
		
		Axis hAxis;
		hAxis.color = { axis_r, axis_g, axis_b };
		hAxis.vid = _setDataBuffer(&hAxisData[0], 6);
		hAxis.vcount = 6;
		graph.hAxis = hAxis;
		
		float vx0 = -1.0f+graph.margin;
		float vy0 = -1.0f+graph.margin;
		float vx1 = -1.0f+graph.margin;
		float vy1 = 1.0f-graph.margin;
		std::vector<float> vAxisData;
		_getLine(vx0, vy0, vx1, vy1, 0.025f, graph.ratio, vAxisData);
		_getLine(vx0, vy0, -vx1, vy1, 0.025f, graph.ratio, vAxisData);
		
		Axis vAxis;
		vAxis.color = { axis_r, axis_g, axis_b };
		vAxis.vid = _setDataBuffer(&vAxisData[0], 12);
		vAxis.vcount = 12;
		graph.vAxis = vAxis;
		
	}
	
	/*
	 
	 
	 private func setupAxis(range: Float, mini: Float, steps: [(Float, Int)], shift: float2, vertical: Bool ) -> Axis {
	 
	 var stepsScaled = steps.map({ ($0.0-mini) / range })
	 for i in 0..<stepsScaled.count {
	 if vertical {
	 stepsScaled[i] =  2.0 * shift.y * stepsScaled[i] - shift.y
	 } else {
	 stepsScaled[i] =  2.0 * shift.x * stepsScaled[i] - shift.x
	 }
	 }
	 // Main axis and arrow.
	 let x0 = -shift.x-kMargin*0.5/self.infos.ratio
	 let y0 = -shift.y-kMargin*0.5
	 let dx0 = kArrowSize
	 let dy0 = self.infos.ratio * kArrowSize
	 let axis : [float2]
	 if vertical {
	 axis = getLine(p0: float2(x0, y0), p1: float2(x0, -y0+dy0)) + getLine(p0: float2(x0, -y0+dy0), p1: float2(x0-dx0, -y0)) + getLine(p0: float2(x0, -y0+dy0), p1: float2(x0+dx0, -y0))
	 } else {
	 axis = getLine(p0: float2(x0, y0), p1: float2(-x0+dx0, y0)) + getLine(p0: float2(-x0+dx0, y0), p1: float2(-x0, y0-dy0)) + getLine(p0: float2(-x0+dx0, y0), p1: float2(-x0, y0+dy0))
	 }*/
	
	void add_curve(const unsigned int graph_id, const std::vector<float> & xs, const std::vector<float> & ys, const float color_r, const float color_g, const float color_b) {
		
		if(graph_id >= _nextGraphId){
			return;
		}
		
		std::cout << "Curving graph " << graph_id << std::endl;
	}

	void add_hist(const unsigned int graph_id, const unsigned int bins, const std::vector<float> & ys, const float color_r, const float color_g, const float color_b) {
		
		if(graph_id >= _nextGraphId){
			return;
		}
		
		std::cout << "Histing graph " << graph_id << std::endl;
	}

	void draw(const unsigned int graph_id, float ratio) {
		
		if(graph_id >= _nextGraphId){
			return;
		}
		
		glDisable(GLenum(GL_DEPTH_TEST));
		glEnable(GLenum(GL_CULL_FACE));
		glFrontFace(GLenum(GL_CCW));
		glCullFace(GLenum(GL_BACK));
		//glBlendFunc(GLenum(GL_SRC_ALPHA), GLenum(GL_ONE_MINUS_SRC_ALPHA))
		//glEnable(GLenum(GL_BLEND))
		const Graph & graph = _graphs[graph_id];
		
		glClearColor(graph.color.r, graph.color.g, graph.color.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glUseProgram(_state.pid);
		glUniform3f(_state.cid, graph.hAxis.color.r, graph.hAxis.color.g, graph.hAxis.color.b);
		glUniform1f(_state.rid, ratio/graph.ratio);
		glBindVertexArray(graph.hAxis.vid);
		glDrawArrays(GL_TRIANGLES, 0, graph.hAxis.vcount);
		glBindVertexArray(graph.vAxis.vid);
		glDrawArrays(GL_TRIANGLES, 0, graph.vAxis.vcount);
		glBindVertexArray(0);
		glUseProgram(0);
		// RESTORE
		
	}

	void free(const unsigned int graph_id) {
		
		if(graph_id >= _nextGraphId){
			return;
		}
		
		std::cout << "Freeing graph " << graph_id << std::endl;
		
		// Free GL ressources;
		//glDeleteVertexArrays(1, &self.horizontal.linesId)
		Graph emptyGraph;
		_graphs[graph_id] = emptyGraph;
	}
	
		 // RATIO at runtime
    // private tuils
	 void _getLine(const float p0x, const float p0y, const float p1x, const float p1y, const float w, const float ratio, std::vector<float> & points) {
		 // Compute normal vector.
		 float dirx = p1x - p0x;
		 float diry = p1y - p0y;
		 const float dirNorm = sqrtf(dirx*dirx+diry*diry);
		 if(dirNorm != 0.0f){
			 dirx /= dirNorm;
			 diry /= dirNorm;
		 }
		 const float norx = -diry;
		 const float nory = dirx;
		 const float shiftx = w;
		 const float shifty = ratio * w;
		 const float deltaNx = shiftx * norx;
		 const float deltaNy = shifty * nory;
		 const float deltaDx = shiftx * dirx;
		 const float deltaDy = shifty * diry;
		 
		 const float ax = p0x - deltaNx - deltaDx;
		 const float bx = p1x - deltaNx + deltaDx;
		 const float cx = p1x + deltaNx + deltaDx;
		 const float dx = p0x + deltaNx - deltaDx;
		 
		 const float ay = p0y - deltaNy - deltaDy;
		 const float by = p1y - deltaNy + deltaDy;
		 const float cy = p1y + deltaNy + deltaDy;
		 const float dy = p0y + deltaNy - deltaDy;
		 
		 points.push_back(ax); points.push_back(ay);
		 points.push_back(bx); points.push_back(by);
		 points.push_back(cx); points.push_back(cy);
		 points.push_back(ax); points.push_back(ay);
		 points.push_back(cx); points.push_back(cy);
		 points.push_back(dx); points.push_back(dy);
		 
	 }
	
	// Sahders strings
	
	static const std::string vstr =
	std::string("#version 330\n") +
	"layout(location = 0) in vec2 v;" + "\n" +
	"uniform float ratio;" + "\n" +
	"void main(){" + "\n" +
	"vec2 finalRatio = ratio < 1.0 ? vec2(1.0, ratio) : vec2(1.0/ratio, 1.0); " + "\n"
	"gl_Position.xy = v * finalRatio;" + "\n" +
	"gl_Position.zw = vec2(1.0);" + "\n" +
	"}" + "\n";
	
	static const std::string fstr =
	std::string("#version 330\n") +
	"uniform vec3 color;" + "\n" +
	"out vec3 frag_Color;" + "\n" +
	"void main(){" + "\n" +
	"frag_Color = color;" + "\n" +
	"}" + "\n";
	
	static const std::string ffstr = std::string("#version 330\n") +
	"uniform vec3 color;" + "\n" +
	"out vec4 fragColor;" + "\n" +
	"void main(){" + "\n" +
	"vec2 pointDiff = gl_PointCoord.xy - 0.5;" + "\n" +
	"fragColor = vec4(color, 1.0-smoothstep(0.4, 0.5, length(pointDiff)));" + "\n" +
	"}" + "\n";
	
	
	// GL helpers
	
	GLuint _setDataBuffer(const float * data, const unsigned int count){
		GLuint vaId;
		glGenVertexArrays(1, &vaId);
		glBindVertexArray(vaId);
		GLuint bufferPos;
		glGenBuffers(1, &bufferPos);
		glBindBuffer(GL_ARRAY_BUFFER, bufferPos);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * count, data, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, bufferPos);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindVertexArray(0);
		return vaId;
	}
	
	GLuint _loadShader(const char * prog, GLuint type) {
		GLuint id = glCreateShader(type);
		// Setup string as source.
		glShaderSource(id, 1, &prog, (const GLint*)NULL);
		// Compile the shader on the GPU.
		glCompileShader(id);
		
		GLint success;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		// If compilation failed, get information and display it.
		if (success != GL_TRUE) {
#ifdef SRG_DEBUG
			GLint infoLogLength;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<char> infoLog(std::max(infoLogLength, int(1)));
			glGetShaderInfoLog(id, infoLogLength, NULL, &infoLog[0]);
			std::cerr << std::endl
			<< "*--- "
			<< (type == GL_VERTEX_SHADER ? "Vertex" : (type == GL_FRAGMENT_SHADER ? "Fragment" : "Geometry (or tess.)"))
			<< " shader failed to compile ---*"
			<< std::endl
			<< &infoLog[0]
			<< "*---------------------------------*"
			<< std::endl << std::endl;
#endif
			return 0;
		}
		
		// Return the id to the successfuly compiled  shader program.
		return id;
	}
	
	GLuint _createGLProgram(const char* vertexString, const char * fragmentString) {
		GLuint vp(0), fp(0), id(0);
		id = glCreateProgram();
		// Load shaders
		vp = _loadShader(vertexString, GL_VERTEX_SHADER);
		glAttachShader(id, vp);
		fp = _loadShader(fragmentString, GL_FRAGMENT_SHADER);
		glAttachShader(id, fp);
		// Link everything
		glLinkProgram(id);
		//Check linking status.
		GLint success = GL_FALSE;
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		// If linking failed, query info and display it.
		if (!success) {
#ifdef SRG_DEBUG
			GLint infoLogLength;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			std::vector<char> infoLog(std::max(infoLogLength, int(1)));
			glGetProgramInfoLog(id, infoLogLength, NULL, &infoLog[0]);
			std::cerr << "Failed loading program: " << &infoLog[0] << std::endl;
#endif
			return 0;
		}
		// We can now clean the shaders objects, by first detaching them
		if (vp != 0) { glDetachShader(id, vp); }
		if (fp != 0) { glDetachShader(id, fp); }
		//And deleting them
		glDeleteShader(vp);
		glDeleteShader(fp);
		return id;
	}
	
	void _internalSetup() {
		_isInit = true;
		const char * vertexPointString = vstr.c_str();
		const char * fragmentPointString = fstr.c_str();
		_state.pid = _createGLProgram(vertexPointString, fragmentPointString);
		glUseProgram(_state.pid);
		_state.cid = glGetUniformLocation(_state.pid, "color");
		_state.rid = glGetUniformLocation(_state.pid, "ratio");
		glUseProgram(0);
	}
	
}

#endif
