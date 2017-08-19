#ifndef SRG_INCLUDE_SR_GRAPH_H
#define SRG_INCLUDE_SR_GRAPH_H

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>

namespace sr_graph {
	
	extern int setup(const float minx, const float maxx, const float miny, const float maxy, const float ratio, const float margins, const float bg_r, const float bg_g, const float bg_b);
	
	extern void add_axes(const unsigned int graph_id, const float width, const float axis_r, const float axis_g, const float axis_b, const bool axisOnSide);
	
	extern void  add_grid(const unsigned int graph_id, const float stepx, const float stepy, const float width, const float lines_r, const float lines_g, const float lines_b);
	
	extern void add_curve(const unsigned int graph_id, const std::vector<float> & xs, const std::vector<float> & ys, const float width, const float color_r, const float color_g, const float color_b);
	

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
	void _internalFree();
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
		GLuint id;
		GLsizei count;
		Color color;
	} Curve;
	
	typedef struct {
		Color color;
		float minx;
		float maxx;
		float miny;
		float maxy;
		float ratio;
		float margin;
		
		GLuint idAxes;
		GLuint countAxes;
		Color colorAxes;
		
		GLuint idGrid;
		GLuint countGrid;
		Color colorGrid;
		
		std::vector<Curve> curves;
		
	} Graph;

	typedef struct {
		GLuint idQuad;
		GLuint pid;
		GLuint cid;
		GLuint rid;
	} _InternalState ;
	
	static _InternalState _state;

	static std::map<unsigned int, Graph> _graphs;
	
	
	int setup(const float minx, const float maxx, const float miny, const float maxy, const float ratio, const float margins, const float bg_r, const float bg_g, const float bg_b){
		// If we haven't initialized our GL programs, do it.
		if (!_isInit) {
			_internalSetup();
		}
		
		Graph graph;
		graph.color = {bg_r, bg_g, bg_b };
		graph.minx = minx;
		graph.maxx = maxx;
		graph.miny = miny;
		graph.maxy = maxy;
		graph.ratio = fabs(ratio);
		graph.margin = fmin(1.0f, fmax(0.0f, fabs(margins*2.0)));
		
		
		int graphId = _nextGraphId;
		_graphs[graphId] = graph;
		++_nextGraphId;
		return graphId;
		
	}
	
	
	enum Orientation {
		VERTICAL, HORIZONTAL
	};
	
	void _generateAxis(const Orientation orientation, const float margin, const float ratio, const float width, const float mini, const float maxi, const bool axisOnSide, std::vector<float> & axisData){
		float hx0 = -1.0f+margin;
		float hx1 = 1.0f-margin;
		float hy;
		bool reverse = maxi < mini;
		// Three positions.
		if(axisOnSide || 0.0 <= fmin(mini, maxi)){
			// Axis on the bottom
			hy = -1.0f+margin;
			if(!axisOnSide && reverse){
				hy *= -1.0f;
			}
		} else if (0 >= fmax(maxi, mini)){
			// Axis on the top
			hy = 1.0f-margin;
			if(reverse){
				hy *= -1.0f;
			}
		} else {
			// Need to find 0 y coord.
			hy = -2.0f*(1.0f - margin)*(mini/(maxi-mini))+margin-1.0f;
		}
		
		const float ld = fmin(0.05f, 0.5f*margin);
		
		if (orientation == VERTICAL) {
			hx0 -= (reverse ? 1.5*ld*ratio : 0.0f);
			hx1 += (reverse ? 0.0f : 1.5*ld*ratio);
			_getLine(hy, hx0, hy, hx1, width, ratio, axisData);
			if(reverse){
				_getLine(hy, hx0, hy+ld, hx0+ld*ratio, width, ratio, axisData);
				_getLine(hy, hx0, hy-ld, hx0+ld*ratio, width, ratio, axisData);
			} else {
				_getLine(hy, hx1+width, hy+ld, hx1-ld*ratio, width, ratio, axisData);
				_getLine(hy, hx1+width, hy-ld, hx1-ld*ratio, width, ratio, axisData);
			}
		} else {
			hx0 -= (reverse ? 1.5*ld : 0.0f);
			hx1 += (reverse ? 0.0f : 1.5*ld);
			_getLine(hx0, hy, hx1, hy, width, ratio, axisData);
			// Add arrow.
			if(reverse){
				_getLine(hx0, hy, hx0+ld, hy+ld*ratio, width, ratio, axisData);
				_getLine(hx0, hy, hx0+ld, hy-ld*ratio, width, ratio, axisData);
			} else {
				_getLine(hx1+width, hy, hx1-ld, hy+ld*ratio, width, ratio, axisData);
				_getLine(hx1+width, hy, hx1-ld, hy-ld*ratio, width, ratio, axisData);
			}
		}
		
	}

    void add_axes(const unsigned int graph_id, const float width, const float axis_r, const float axis_g, const float axis_b, const bool axisOnSide){
		
		if(graph_id >= _nextGraphId || _graphs.count(graph_id) == 0){
			return;
		}
		Graph & graph = _graphs[graph_id];
		/// Generate data for axis.
		// Horizontal axis: from (-margin, -margin) to (margin, -margin)
		std::vector<float> axisData;
		_generateAxis(HORIZONTAL, graph.margin, graph.ratio, width, graph.miny, graph.maxy, axisOnSide, axisData);
		_generateAxis(VERTICAL, graph.margin, graph.ratio, width, graph.minx, graph.maxx, axisOnSide, axisData);

		graph.colorAxes = { axis_r, axis_g, axis_b };
		graph.countAxes = (GLsizei)(axisData.size()/2);
		graph.idAxes = _setDataBuffer(&axisData[0], graph.countAxes);
	
	}
	
	void add_grid(const unsigned int graph_id, const float stepx, const float stepy, const float width, const float lines_r, const float lines_g, const float lines_b) {
		if(graph_id >= _nextGraphId || _graphs.count(graph_id) == 0){
			return;
		}
		Graph & graph = _graphs[graph_id];
		std::vector<float> gridData;
		
		if(stepx != 0.0f && graph.maxx != graph.minx){
			float shiftH =  2.0f*(1.0f-graph.margin)*fabs(stepx)/fabs(graph.maxx - graph.minx);
			for(float x = -1.0f+graph.margin; x < 1.0f-graph.margin; x += shiftH){
				_getLine(x, -1.0f+graph.margin, x, 1.0f-graph.margin, width, graph.ratio, gridData);
			}
			_getLine(1.0f - graph.margin, -1.0f+graph.margin, 1.0f - graph.margin, 1.0f-graph.margin, width, graph.ratio, gridData);
		}
		if(stepy != 0.0f && graph.maxy != graph.miny){
			float shiftV = 2.0f*(1.0f-graph.margin)*fabs(stepy)/fabs(graph.maxy - graph.miny);
			for(float y = -1.0f+graph.margin; y < 1.0f-graph.margin; y += shiftV){
				_getLine(-1.0f+graph.margin, y, 1.0f-graph.margin, y, width, graph.ratio, gridData);
			}
			_getLine(-1.0f+graph.margin, 1.0f - graph.margin, 1.0f-graph.margin, 1.0f - graph.margin, width, graph.ratio, gridData);
		}
		graph.colorGrid = { lines_r, lines_g, lines_b };
		graph.countGrid = (GLsizei)(gridData.size()/2);
		graph.idGrid = _setDataBuffer(&gridData[0], graph.countGrid);
	}
	
	void add_curve(const unsigned int graph_id, const std::vector<float> & xs, const std::vector<float> & ys, const float width, const float color_r, const float color_g, const float color_b) {
		
		if(graph_id >= _nextGraphId || _graphs.count(graph_id) == 0){
			return;
		}
		
		Graph & graph = _graphs[graph_id];
		if(xs.size() != ys.size() || xs.size() == 0){
			return;
		}
		
		
		const float ax = (2.0f*(1.0f-graph.margin))/(graph.maxx - graph.minx);
		const float bx = -1.0f + graph.margin - ax * graph.minx;
		const float ay = (2.0f*(1.0f-graph.margin))/(graph.maxy - graph.miny);
		const float by = -1.0f + graph.margin - ay * graph.miny;
		std::vector<float> curveData;
		float x0 = ax*xs[0]+bx;
		float y0 = ay*ys[0]+by;
		for(unsigned int i = 1; i < xs.size(); ++i){
			const float x1 = ax*xs[i]+bx;
			const float y1 = ay*ys[i]+by;
			_getLine(x0, y0, x1, y1, width, graph.ratio, curveData);
			x0 = x1;
			y0 = y1;
		}
		Curve curve;
		curve.color = {color_r, color_g, color_b};
		curve.count = (GLsizei)(curveData.size()/2);
		curve.id = _setDataBuffer(&curveData[0], curve.count);
		graph.curves.push_back(curve);
		
	}

	void add_hist(const unsigned int graph_id, const unsigned int bins, const std::vector<float> & ys, const float color_r, const float color_g, const float color_b) {
		
		if(graph_id >= _nextGraphId || _graphs.count(graph_id) == 0){
			return;
		}
		
		std::cout << "Histing graph " << graph_id << std::endl;
	}

	void draw(const unsigned int graph_id, float ratio) {
		
		if(graph_id >= _nextGraphId || _graphs.count(graph_id) == 0){
			return;
		}
		
		// Copy OpenGL states.
		bool depthState = glIsEnabled(GL_DEPTH_TEST) == GL_TRUE;
		bool cullState = glIsEnabled(GL_CULL_FACE) == GL_TRUE;
		bool blendState = glIsEnabled(GL_BLEND) == GL_TRUE;
		GLint faceMode, cullFaceMode, blendSrcMode, blendDstMode;
		glGetIntegerv(GL_FRONT_FACE, &faceMode);
		glGetIntegerv(GL_CULL_FACE_MODE, &cullFaceMode);
		glGetIntegerv(GL_BLEND_SRC, &blendSrcMode);
		glGetIntegerv(GL_BLEND_DST, &blendDstMode);
		GLint polygonModes[2];
		glGetIntegerv(GL_POLYGON_MODE, polygonModes);
		
		// Set states.
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL) ;
		
		const Graph & graph = _graphs[graph_id];
		glUseProgram(_state.pid);
		// Draw quad to clear.
		glUniform1f(_state.rid, ratio/graph.ratio);
		glUniform3f(_state.cid, graph.color.r, graph.color.g, graph.color.b);
		glBindVertexArray(_state.idQuad);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		
		// Draw axes.
		glUseProgram(_state.pid);
		glUniform1f(_state.rid, ratio/graph.ratio);
		
		glUniform3f(_state.cid, graph.colorGrid.r, graph.colorGrid.g, graph.colorGrid.b);
		glBindVertexArray(graph.idGrid);
		glDrawArrays(GL_TRIANGLES, 0, graph.countGrid);
		glBindVertexArray(0);
		
		glUniform3f(_state.cid, graph.colorAxes.r, graph.colorAxes.g, graph.colorAxes.b);
		glBindVertexArray(graph.idAxes);
		glDrawArrays(GL_TRIANGLES, 0, graph.countAxes);
		
		
		for(unsigned int i = 0; i < graph.curves.size(); ++i){
			glUniform3f(_state.cid, graph.curves[i].color.r, graph.curves[i].color.g, graph.curves[i].color.b);
			glBindVertexArray(graph.curves[i].id);
			glDrawArrays(GL_TRIANGLES, 0, graph.curves[i].count);
		}
		
		glBindVertexArray(0);
		glUseProgram(0);
		// Restore OpenGL state.
		if(blendState){
			glEnable(GL_BLEND);
		} else {
			glDisable(GL_BLEND);
		}
		if(cullState){
			glEnable(GL_CULL_FACE);
		} else {
			glDisable(GL_CULL_FACE);
		}
		if(depthState){
			glEnable(GL_DEPTH_TEST);
		} else {
			glDisable(GL_DEPTH_TEST);
		}
		glFrontFace(faceMode);
		glCullFace(cullFaceMode);
		glBlendFunc(blendSrcMode, blendDstMode);
		glPolygonMode(GL_FRONT, polygonModes[0] );
		glPolygonMode(GL_BACK,  polygonModes[1] );
		
	}

	void free(const unsigned int graph_id) {
		
		if(graph_id >= _nextGraphId || _graphs.count(graph_id) == 0){
			return;
		}
		
		std::cout << "Freeing graph " << graph_id << std::endl;
		
		const Graph & graph = _graphs[graph_id];
		// Free GL ressources;
		glDeleteVertexArrays(1, &graph.idGrid);
		glDeleteVertexArrays(1, &graph.idAxes);
		_graphs.erase(graph_id);
	}
	
	
    // Private utils
	
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
	
	// Shaders strings
	
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
	"out vec4 frag_Color;" + "\n" +
	"void main(){" + "\n" +
	"frag_Color.rgb = color;" + "\n" +
	"frag_Color.a = 1.0;" + "\n" +
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
		const float quadData[12] = { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
		_state.idQuad = _setDataBuffer(quadData, 6);
			
	}
	void _internalFree(){
		glDeleteProgram(_state.pid);
		glDeleteVertexArrays(1, &_state.idQuad);
	}
	
}

#endif
