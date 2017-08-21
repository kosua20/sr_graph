/* sr_graph - v1.0 - public domain plot tracer ; no warranties implied, use at your own risk.

 Do this:
	 #define SRG_IMPLEMENTATION_SR_GRAPH
	 before you include this file in *one* C++ file to create the implementation.
 You will also need to include some OpenGL headers before including this one.

	 // i.e. it should look like this:
	 #include <gl.h>/<gl3w.h>/<GL/glew.h>
	 #include ...
	 #include ...
	 #define SRG_IMPLEMENTATION_SR_GRAPH
	 #include "srg_graph.h"


 See below the exact list of types and functions expected from OpenGL.
	 @TODO
 
 Usage:
	 @TODO
 
 Release 1.0 notes:
	 @TODO
 
 Revision history
	 @TODO
 
 License:
	 See end of file.
*/

#ifndef SRG_INCLUDE_SR_GRAPH_H
#define SRG_INCLUDE_SR_GRAPH_H

// @TODO: extern/static

#include <vector>

namespace sr_graph {
	
	extern int setup(const float minx, const float maxx, const float miny, const float maxy, const float ratio, const float margins, const float bg_r, const float bg_g, const float bg_b);
	
	extern void add_axes(const int graph_id, const float width, const float axis_r, const float axis_g, const float axis_b, const bool axisOnSide);
	
	extern void add_grid(const int graph_id, const float stepx, const float stepy, const float width, const float lines_r, const float lines_g, const float lines_b, const bool fromZero);
	
	extern int add_curve(const int graph_id, const std::vector<float> & xs, const std::vector<float> & ys, const float width, const float color_r, const float color_g, const float color_b);
	
	extern void update_curve(const int graph_id, const int curve_id, const std::vector<float> & xs, const std::vector<float> & ys);
	
	extern int add_hist(const int graph_id, const unsigned int bins, const std::vector<float> & ys, const float spacing, const float color_r, const float color_g, const float color_b);
	
	extern void update_hist(const int graph_id, const int hist_id, const std::vector<float> & ys);
	
	extern int add_points(const int graph_id, const std::vector<float> & xs, const std::vector<float> & ys, const float size, const float color_r, const float color_g, const float color_b);
	
	extern void update_points(const int graph_id, const int points_id, const std::vector<float> & xs, const std::vector<float> & ys);
	
	extern void draw(const int graph_id, const float ratio = 0.0f);
	
	extern void free(const int graph_id);
	
	extern void free();
	
}

#endif

#ifdef SRG_IMPLEMENTATION_SR_GRAPH

#include <stdio.h>
#include <math.h>
#include <vector>

namespace sr_graph {
	
	/// Internal structs.
	
	typedef struct {
		float r;
		float g;
		float b;
	} _srg_Color;

	typedef struct {
		GLuint id;
		GLuint bid;
		GLsizei count;
	} _srg_Buffers;
	
	typedef struct {
		_srg_Buffers buffer;
		_srg_Color color;
		float param0;
		int param1;
	} _srg_Curve;
	
	typedef struct {
		_srg_Color color;
		float minx;
		float maxx;
		float miny;
		float maxy;
		float ratio;
		float margin;
		bool freed;
		
		_srg_Buffers bufferAxes;
		_srg_Color colorAxes;

		_srg_Buffers bufferGrid;
		_srg_Color colorGrid;
		
		std::vector<_srg_Curve> curves;
		std::vector<_srg_Curve> points;
		std::vector<_srg_Curve> hists;
	} _srg_Graph;
	
	typedef struct {
		_srg_Buffers bufferQuad;
		GLuint pid;
		GLuint ppid;
		GLuint lpid;
		GLuint cid;
		GLuint pcid;
		GLuint lcid;
		GLuint rid;
		GLuint prid;
		GLuint lrid;
	} _srg_InternalState ;
	
	enum _srg_Orientation {
		VERTICAL, HORIZONTAL
	};

	
	
	
	/// Internal variables.
	
	static bool _srg_isInit = false;
	static _srg_InternalState _srg_state;
	static std::vector<_srg_Graph> _srg_graphs;

	
	/// Foreward declarations.
	
	void _srg_internalSetup();
	_srg_Buffers _srg_setDataBuffer(const float * data, const unsigned int count);
	void _srg_getLine(const float p0x, const float p0y, const float p1x, const float p1y, const float w, const float ratio, std::vector<float> & points);
	void _srg_getRectangle(const float p0x, const float p0y, const float p1x, const float p1y, const float w, std::vector<float> & points);
	void _srg_getPoint(const float p0x, const float p0y, const float radius, const float ratio, std::vector<float> & points);
	void _srg_generateAxis(const _srg_Orientation orientation, const float margin, const float ratio, const float width, const float mini, const float maxi, const bool axisOnSide, const bool reverse, std::vector<float> & axisData);
	void _srg_generateCurve(const _srg_Graph & graph, const std::vector<float> & xs, const std::vector<float> & ys, _srg_Curve & curve);
	void _srg_generatePoints(const _srg_Graph & graph, const std::vector<float> & xs, const std::vector<float> & ys, _srg_Curve & curve);
	void _srg_generateHist(const _srg_Graph & graph, const std::vector<float> & ys, _srg_Curve & curve);
	
	
	/// Exposed functions.
	
	int setup(const float minx, const float maxx, const float miny, const float maxy, const float ratio, const float margins, const float bg_r, const float bg_g, const float bg_b){
		// If we haven't initialized our GL programs, do it.
		if (!_srg_isInit) {
			_srg_internalSetup();
		}
		// Create a graph with the given infos.
		_srg_Graph graph;
		graph.color = {bg_r, bg_g, bg_b };
		graph.minx = minx;
		graph.maxx = maxx;
		graph.miny = miny;
		graph.maxy = maxy;
		graph.ratio = fabs(ratio);
		graph.margin = fmin(1.0f, fmax(0.0f, fabs(margins*2.0)));
		graph.freed = false;
		graph.bufferAxes = { 0, 0, 0 };
		graph.colorAxes = graph.color;
		graph.bufferGrid = { 0, 0, 0 };
		graph.colorGrid = graph.color;
		graph.curves.clear();
		graph.points.clear();
		graph.hists.clear();

		// Store it.
		_srg_graphs.push_back(graph);
		return (int)_srg_graphs.size() - 1;

	}
	

	void add_axes(const int graph_id, const float width, const float axis_r, const float axis_g, const float axis_b, const bool axisOnSide){
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		/// Generate data for axis.
		std::vector<float> axisData;
		_srg_generateAxis(HORIZONTAL, graph.margin, graph.ratio, width, graph.miny, graph.maxy, axisOnSide, graph.minx > graph.maxx, axisData);
		_srg_generateAxis(VERTICAL, graph.margin, graph.ratio, width, graph.minx, graph.maxx, axisOnSide, graph.miny > graph.maxy, axisData);

		graph.colorAxes = { axis_r, axis_g, axis_b };
		graph.bufferAxes = _srg_setDataBuffer(&axisData[0], axisData.size() / 2);
	}
	
	
	void add_grid(const int graph_id, const float stepx, const float stepy, const float width, const float lines_r, const float lines_g, const float lines_b, const bool fromZero) {
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		std::vector<float> gridData;
		
		const float ax = (2.0f*(1.0f-graph.margin))/(graph.maxx - graph.minx);
		const float bx = -1.0f + graph.margin - ax * graph.minx;
		const float ay = (2.0f*(1.0f-graph.margin))/(graph.maxy - graph.miny);
		const float by = -1.0f + graph.margin - ay * graph.miny;
		
		if(stepx != 0.0f && graph.maxx != graph.minx){
			float shiftH = fabs(ax)*fabs(stepx);
			if(fromZero){
				float xZero = bx;
				while(xZero < -1.0f + graph.margin){
					xZero += shiftH;
				}
				while(xZero > 1.0f - graph.margin){
					xZero -= shiftH;
				}
				for(float xi = xZero; xi >= -1.0f+graph.margin; xi -= shiftH){
					_srg_getLine(xi, -1.0f+graph.margin, xi, 1.0f-graph.margin, width, graph.ratio, gridData);
				}
				for(float xi = xZero+shiftH; xi <= 1.0f-graph.margin; xi += shiftH){
					_srg_getLine(xi, -1.0f+graph.margin, xi, 1.0f-graph.margin, width, graph.ratio, gridData);
				}
			} else {
				for(float x = -1.0f+graph.margin; x < 1.0f-graph.margin; x += shiftH){
					_srg_getLine(x, -1.0f+graph.margin, x, 1.0f-graph.margin, width, graph.ratio, gridData);
				}
				_srg_getLine(1.0f - graph.margin, -1.0f+graph.margin, 1.0f - graph.margin, 1.0f-graph.margin, width, graph.ratio, gridData);
			}
		}
		if(stepy != 0.0f && graph.maxy != graph.miny){
			float shiftV = fabs(ay)*fabs(stepy);
			if(fromZero){
				float yZero = by;
				while(yZero < -1.0f + graph.margin){
					yZero += shiftV;
				}
				while(yZero > 1.0f - graph.margin){
					yZero -= shiftV;
				}
				for(float yi = yZero; yi >= -1.0f+graph.margin; yi -= shiftV){
					_srg_getLine(-1.0f+graph.margin, yi, 1.0f-graph.margin, yi, width, graph.ratio, gridData);
				}
				for(float yi = yZero+shiftV; yi <= 1.0f-graph.margin; yi += shiftV){
					_srg_getLine(-1.0f+graph.margin, yi, 1.0f-graph.margin, yi, width, graph.ratio, gridData);
				}
			} else {
				for(float y = -1.0f+graph.margin; y < 1.0f-graph.margin; y += shiftV){
					_srg_getLine(-1.0f+graph.margin, y, 1.0f-graph.margin, y, width, graph.ratio, gridData);
				}
				_srg_getLine(-1.0f+graph.margin, 1.0f - graph.margin, 1.0f-graph.margin, 1.0f - graph.margin, width, graph.ratio, gridData);
			}
		}
		graph.colorGrid = { lines_r, lines_g, lines_b };
		graph.bufferGrid = _srg_setDataBuffer(&gridData[0], gridData.size() / 2);
	}
	
	
	int add_curve(const int graph_id, const std::vector<float> & xs, const std::vector<float> & ys, const float width, const float color_r, const float color_g, const float color_b){
		
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return -1;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		_srg_Curve curve;
		curve.color = {color_r, color_g, color_b};
		curve.param0 = width;
		_srg_generateCurve(graph, xs, ys, curve);
		graph.curves.push_back(curve);
		return (int)graph.curves.size()-1;
	}
	
	
	void update_curve(const int graph_id, const int curve_id, const std::vector<float> & xs, const std::vector<float> & ys) {
		
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		if(curve_id < 0 || curve_id >= graph.curves.size()){
			return;
		}
		_srg_Curve & curve = graph.curves[curve_id];
		
		glDeleteVertexArrays(1, &(curve.buffer.id));
		glDeleteBuffers(1, &(curve.buffer.bid));
		_srg_generateCurve(graph, xs, ys, curve);
	}
	
	
	int add_points(const int graph_id, const std::vector<float> & xs, const std::vector<float> & ys, const float size, const float color_r, const float color_g, const float color_b) {
		
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return -1;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		_srg_Curve curve;
		curve.color = {color_r, color_g, color_b};
		curve.param0 = size;
		_srg_generatePoints(graph, xs, ys, curve);
		graph.points.push_back(curve);
		return (int)graph.points.size()-1;
	}
	
	
	void update_points(const int graph_id, const int curve_id, const std::vector<float> & xs, const std::vector<float> & ys) {
		
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		if(curve_id < 0 || curve_id >= graph.points.size()){
			return;
		}
		_srg_Curve & curve = graph.points[curve_id];
		glDeleteVertexArrays(1, &(curve.buffer.id));
		glDeleteBuffers(1, &(curve.buffer.bid));
		_srg_generatePoints(graph, xs, ys, curve);
	}
	
	
	int add_hist(const int graph_id, const unsigned int bins, const std::vector<float> & ys, const float spacing, const float color_r, const float color_g, const float color_b) {
		
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return -1;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		_srg_Curve curve;
		curve.color = {color_r, color_g, color_b};
		curve.param0 = spacing;
		curve.param1 = bins;
		_srg_generateHist(graph, ys, curve);
		graph.hists.push_back(curve);
		return (int)graph.hists.size()-1;
	}
	
	void update_hist(const int graph_id, const int curve_id, const std::vector<float> & ys) {
		
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		if(curve_id < 0 || curve_id >= graph.hists.size()){
			return;
		}
		_srg_Curve & curve = graph.hists[curve_id];
		glDeleteVertexArrays(1, &(curve.buffer.id));
		glDeleteBuffers(1, &(curve.buffer.bid));
		_srg_generateHist(graph, ys, curve);
	}

	
	void draw(const int graph_id, float ratio) {
		
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
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
		
		
		const _srg_Graph & graph = _srg_graphs[graph_id];
		const float finalRatio = (ratio == 0.0) ? 1.0f : ratio/graph.ratio;
		glUseProgram(_srg_state.pid);
		glUniform1f(_srg_state.rid, finalRatio);
		glUseProgram(_srg_state.lpid);
		glUniform1f(_srg_state.lrid, finalRatio);
		glUseProgram(_srg_state.ppid);
		glUniform1f(_srg_state.prid, finalRatio);
		// Draw quad to clear.
		glUseProgram(_srg_state.pid);
		glUniform3f(_srg_state.cid, graph.color.r, graph.color.g, graph.color.b);
		glBindVertexArray(_srg_state.bufferQuad.id);
		glDrawArrays(GL_TRIANGLES, 0, _srg_state.bufferQuad.count);
		glBindVertexArray(0);
		
		// Draw grid.
		glUseProgram(_srg_state.lpid);
		glUniform3f(_srg_state.lcid, graph.colorGrid.r, graph.colorGrid.g, graph.colorGrid.b);
		glBindVertexArray(graph.bufferGrid.id);
		glDrawArrays(GL_TRIANGLES, 0, graph.bufferGrid.count);
		glBindVertexArray(0);
		
		// Draw histograms.
		glUseProgram(_srg_state.pid);
		for(unsigned int i = 0; i < graph.hists.size(); ++i){
			glUniform3f(_srg_state.cid, graph.hists[i].color.r, graph.hists[i].color.g, graph.hists[i].color.b);
			glBindVertexArray(graph.hists[i].buffer.id);
			glDrawArrays(GL_TRIANGLES, 0, graph.hists[i].buffer.count);
		}
		
		// Draw curves.
		glUseProgram(_srg_state.lpid);
		for(unsigned int i = 0; i < graph.curves.size(); ++i){
			glUniform3f(_srg_state.lcid, graph.curves[i].color.r, graph.curves[i].color.g, graph.curves[i].color.b);
			glBindVertexArray(graph.curves[i].buffer.id);
			glDrawArrays(GL_TRIANGLES, 0, graph.curves[i].buffer.count);
		}
		
		// Draw axes.
		glUseProgram(_srg_state.lpid);
		glUniform3f(_srg_state.lcid, graph.colorAxes.r, graph.colorAxes.g, graph.colorAxes.b);
		glBindVertexArray(graph.bufferAxes.id);
		glDrawArrays(GL_TRIANGLES, 0, graph.bufferAxes.count);
		
		// Draw points.
		glUseProgram(_srg_state.ppid);
		for(unsigned int i = 0; i < graph.points.size(); ++i){
			glUniform3f(_srg_state.pcid, graph.points[i].color.r, graph.points[i].color.g, graph.points[i].color.b);
			glBindVertexArray(graph.points[i].buffer.id);
			glDrawArrays(GL_TRIANGLES, 0, graph.points[i].buffer.count);
		}
		
		// Restore OpenGL state.
		glBindVertexArray(0);
		glUseProgram(0);
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
		glPolygonMode(GL_BACK, polygonModes[1] );
		
	}

	
	void free(const int graph_id) {
		if(graph_id < 0 || graph_id >= _srg_graphs.size() || _srg_graphs[graph_id].freed){
			return;
		}
		_srg_Graph & graph = _srg_graphs[graph_id];
		// Free vertex arrays.
		glDeleteVertexArrays(1, &graph.bufferGrid.id);
		glDeleteVertexArrays(1, &graph.bufferAxes.id);
		glDeleteBuffers(1, &(graph.bufferGrid.bid));
		glDeleteBuffers(1, &(graph.bufferAxes.bid));

		for(unsigned int i = 0; i < graph.curves.size(); ++i){
			glDeleteVertexArrays(1, &(graph.curves[i].buffer.id));
			glDeleteBuffers(1, &(graph.curves[i].buffer.bid));
		}
		for(unsigned int i = 0; i < graph.hists.size(); ++i){
			glDeleteVertexArrays(1, &(graph.hists[i].buffer.id));
			glDeleteBuffers(1, &(graph.hists[i].buffer.bid));
		}
		for(unsigned int i = 0; i < graph.points.size(); ++i){
			glDeleteVertexArrays(1, &(graph.points[i].buffer.id));
			glDeleteBuffers(1, &(graph.points[i].buffer.bid));
		}
		graph.freed = true;
	}
	
	void free(){
		// Free each graph and then clear the map.
		for(unsigned int i = 0; i < _srg_graphs.size(); ++i){
			free(i);
		}
		_srg_graphs.clear();
		// Delete the programs and quad data.
		glDeleteProgram(_srg_state.pid);
		glDeleteProgram(_srg_state.lpid);
		glDeleteProgram(_srg_state.ppid);
		glDeleteVertexArrays(1, &_srg_state.bufferQuad.id);
		glDeleteBuffers(1, &_srg_state.bufferQuad.bid);
	}
	
	
	/// Internal functions.
	
	void _srg_getLine(const float p0x, const float p0y, const float p1x, const float p1y, const float w, const float ratio, std::vector<float> & points) {
		// Compute normal vector.
		float dirx = p1x - p0x; float diry = p1y - p0y;
		const float dirNorm = sqrtf(dirx*dirx+diry*diry);
		if(dirNorm != 0.0f){
			dirx /= dirNorm; diry /= dirNorm;
		}
		
		const float norx = -diry; const float nory = dirx;
		const float sdx = w; const float sdy = ratio * w;
		const float dNx = sdx * norx; const float dNy = sdy * nory;
		float dDx = sdx * dirx; float dDy = sdy * diry;
		dDx = 00; dDy = 0.0;// @DEBUG
		
		const float ax = p0x - dNx - dDx; const float ay = p0y - dNy - dDy;
		const float bx = p1x - dNx + dDx; const float by = p1y - dNy + dDy;
		const float cx = p1x + dNx + dDx; const float cy = p1y + dNy + dDy;
		const float dx = p0x + dNx - dDx; const float dy = p0y + dNy - dDy;
		points.push_back(ax); points.push_back(ay);
		points.push_back(bx); points.push_back(by);
		points.push_back(cx); points.push_back(cy);
		points.push_back(ax); points.push_back(ay);
		points.push_back(cx); points.push_back(cy);
		points.push_back(dx); points.push_back(dy);
	}
	
	
	void _srg_getRectangle(const float p0x, const float p0y, const float p1x, const float p1y, const float w, std::vector<float> & points) {
		const float wx = w * 0.5;
		const float ax = p0x - wx; const float ay = p0y;
		const float bx = p0x + wx; const float by = p0y;
		const float cx = p1x + wx; const float cy = p1y;
		const float dx = p1x - wx; const float dy = p1y;
		points.push_back(ax); points.push_back(ay);
		points.push_back(bx); points.push_back(by);
		points.push_back(cx); points.push_back(cy);
		points.push_back(ax); points.push_back(ay);
		points.push_back(cx); points.push_back(cy);
		points.push_back(dx); points.push_back(dy);
	}
	
	
	void _srg_getPoint(const float p0x, const float p0y, const float radius, const float ratio, std::vector<float> & points) {
		const float wx = radius;   const float wy = radius * ratio;
		const float ax = p0x - wx; const float ay = p0y - wy;
		const float bx = p0x + wx; const float by = p0y - wy;
		const float cx = p0x + wx; const float cy = p0y + wy;
		const float dx = p0x - wx; const float dy = p0y + wy;
		points.push_back(ax); points.push_back(ay);
		points.push_back(bx); points.push_back(by);
		points.push_back(cx); points.push_back(cy);
		points.push_back(ax); points.push_back(ay);
		points.push_back(cx); points.push_back(cy);
		points.push_back(dx); points.push_back(dy);
	}
	
	
	void _srg_generateAxis(const _srg_Orientation orientation, const float margin, const float ratio, const float width, const float mini, const float maxi, const bool axisOnSide, const bool reverse, std::vector<float> & axisData){
		float hx0 = -1.0f+margin;
		float hx1 = 1.0f-margin;
		float hy;
		// Three positions.
		if(axisOnSide || 0.0 <= fmin(mini, maxi)){
			// Axis on the bottom
			hy = -1.0f+margin;
			if(!axisOnSide && maxi < mini){
				hy *= -1.0f;
			}
		} else if (0 >= fmax(maxi, mini)){
			// Axis on the top
			hy = 1.0f-margin;
			if(maxi < mini){
				hy *= -1.0f;
			}
		} else {
			// Need to find 0 y coord.
			hy = -2.0f*(1.0f - margin)*(mini/(maxi-mini))+margin-1.0f;
		}
		
		const float ld = fmin(0.03f, 0.3f*margin);
		
		if (orientation == VERTICAL) {
			hx0 -= (reverse ? 1.5*ld*ratio : 0.0f);
			hx1 += (reverse ? 0.0f : 1.5*ld*ratio);
			_srg_getLine(hy, hx0, hy, hx1, width, ratio, axisData);
			if(reverse){
				_srg_getLine(hy, hx0, hy+ld, hx0+ld*ratio, width, ratio, axisData);

				axisData[axisData.size() - 6] = hy;
				axisData[axisData.size() - 12] = hy;
				axisData[axisData.size() - 2] = hy;
				axisData[axisData.size() - 1] = hx0 + sqrt(2.0f)*width;

				_srg_getLine(hy, hx0, hy-ld, hx0+ld*ratio, width, ratio, axisData);
				axisData[axisData.size() - 2] = hy;
				axisData[axisData.size() - 12] = hy;
				axisData[axisData.size() - 11] = hx0 + sqrt(2.0f)*width;
				axisData[axisData.size() - 6] = hy;
				axisData[axisData.size() - 5] = hx0 + sqrt(2.0f)*width;

			} else {
				_srg_getLine(hy, hx1+width, hy+ld, hx1-ld*ratio, width, ratio, axisData);
				axisData[axisData.size() - 2] = hy;
				axisData[axisData.size() - 12] = hy;
				axisData[axisData.size() - 11] = hx1 - sqrt(2.0f)*width;
				axisData[axisData.size() - 6] = hy;
				axisData[axisData.size() - 5] = hx1 - sqrt(2.0f)*width;

				_srg_getLine(hy, hx1+width, hy-ld, hx1-ld*ratio, width, ratio, axisData);
				axisData[axisData.size() - 6] = hy;
				axisData[axisData.size() - 12] = hy;
				axisData[axisData.size() - 2] = hy;
				axisData[axisData.size() - 1] = hx1-sqrt(2.0f)*width;
			}
		} else {
			hx0 -= (reverse ? 1.5*ld : 0.0f);
			hx1 += (reverse ? 0.0f : 1.5*ld);
			_srg_getLine(hx0, hy, hx1, hy, width, ratio, axisData);
			// Add arrow.
			if(reverse){
				_srg_getLine(hx0, hy, hx0+ld, hy+ld*ratio, width, ratio, axisData);
				axisData[axisData.size() - 5] = hy;
				axisData[axisData.size() - 11] = hy;
				axisData[axisData.size() - 1] = hy;
				axisData[axisData.size() - 2] = hx0 + sqrt(2.0f)*width;
				axisData[axisData.size() - 1] = hy;
				axisData[axisData.size() - 11] = hy;
				axisData[axisData.size() - 12] = hx0 + sqrt(2.0f)*width;
				axisData[axisData.size() - 5] = hy;
				axisData[axisData.size() - 6] = hx0 + sqrt(2.0f)*width;
				_srg_getLine(hx0, hy, hx0+ld, hy-ld*ratio, width, ratio, axisData);
			} else {
				_srg_getLine(hx1+width, hy, hx1-ld, hy+ld*ratio, width, ratio, axisData);
				axisData[axisData.size() - 5] = hy;
				axisData[axisData.size() - 11] = hy;
				axisData[axisData.size() - 1] = hy;
				axisData[axisData.size() - 2] = hx1 - sqrt(2.0f)*width;
				
				_srg_getLine(hx1 + width, hy, hx1 - ld, hy - ld*ratio, width, ratio, axisData);
				axisData[axisData.size() - 1] = hy;
				axisData[axisData.size() - 11] = hy;
				axisData[axisData.size() - 12] = hx1 - sqrt(2.0f)*width;
				axisData[axisData.size() - 5] = hy;
				axisData[axisData.size() - 6] = hx1 - sqrt(2.0f)*width;
			}
		}
		
	}
	
	void _srg_generateCurve(const _srg_Graph & graph, const std::vector<float> & xs, const std::vector<float> & ys, _srg_Curve & curve) {
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
			_srg_getLine(x0, y0, x1, y1, curve.param0, graph.ratio, curveData);
			// @TODO: handle lines intersections.
			x0 = x1;
			y0 = y1;
		}
		
		curve.buffer = _srg_setDataBuffer(&curveData[0], curveData.size() / 2);
	}
	
	void _srg_generatePoints(const _srg_Graph & graph, const std::vector<float> & xs, const std::vector<float> & ys, _srg_Curve & curve){
		
		if(xs.size() != ys.size() || xs.size() == 0){
			return;
		}
		const float ax = (2.0f*(1.0f-graph.margin))/(graph.maxx - graph.minx);
		const float bx = -1.0f + graph.margin - ax * graph.minx;
		const float ay = (2.0f*(1.0f-graph.margin))/(graph.maxy - graph.miny);
		const float by = -1.0f + graph.margin - ay * graph.miny;
		
		std::vector<float> curveData;
		
		for(unsigned int i = 0; i < xs.size(); ++i){
			const float x0 = ax*xs[i]+bx;
			const float y0 = ay*ys[i]+by;
			_srg_getPoint(x0, y0, curve.param0, graph.ratio, curveData);
		}
		
		curve.buffer = _srg_setDataBuffer(&curveData[0], curveData.size() / 2);
	}
	
	void _srg_generateHist(const _srg_Graph & graph, const std::vector<float> & ys, _srg_Curve & curve){
		if(ys.size() == 0){
			return;
		}
		const float binSize = (graph.maxx - graph.minx)/(float)curve.param1;
		std::vector<int> binCounts(curve.param1, 0);
		for(unsigned int i = 0; i < ys.size(); ++i){
			const int j = (unsigned int)floor((ys[i] - graph.minx)/binSize);
			if(j < 0 || j >= binCounts.size()){
				continue;
			}
			binCounts[j] += 1;
		}
		
		std::vector<float> histData;
		const float ax = (2.0f*(1.0f-graph.margin))/(graph.maxx - graph.minx);
		const float bx = -1.0f + graph.margin - ax * graph.minx;
		const float ay = (2.0f*(1.0f-graph.margin))/(graph.maxy - graph.miny);
		const float by = -1.0f + graph.margin - ay * graph.miny;
		const float binWidth = fmax(0.0f, 2.0f*(1.0 - graph.margin)/(float)curve.param1 - curve.param0);
		for(unsigned int i = 0; i < curve.param1; ++i){
			if(binCounts[i] == 0){
				continue;
			}
			float x0 = ax*(graph.minx+(float)(i+0.5)*binSize) + bx; //@TODO: check corner cases.
			float y0 = by;
			float y1 = ay * (float)binCounts[i] + by;
			_srg_getRectangle(x0, y0, x0, y1, binWidth, histData);
		}
		curve.buffer = _srg_setDataBuffer(&histData[0], histData.size() / 2);
	}
	
	
	/// Shaders strings.
	
	static const char * _srg_vstr =
		"#version 330\nlayout(location = 0) in vec2 v;\nuniform float ratio;\nout vec2 coords;\nvoid main(){\nvec2 finalRatio = ratio < 1.0 ? vec2(1.0, ratio) : vec2(1.0/ratio, 1.0);\ngl_Position.xy = v * finalRatio;\ngl_Position.zw = vec2(1.0);\nint id = int(mod(gl_VertexID, 6));\nbool uzero = (id == 0) || (id == 3) || (id == 5);\nbool vzero = (id == 0) || (id == 3) || (id == 1);\ncoords = vec2(uzero ? -1.0 : 1.0, vzero ? -1.0 : 1.0);\n}\n";
	
	static const char * _srg_fstr = "#version 330\nin vec2 coords;\nuniform vec3 color;\nout vec4 frag_Color;\nvoid main(){\nfrag_Color.rgb = color;\nfrag_Color.a = 1.0;\n}\n";
	
	static const char * _srg_lfstr = "#version 330\nin vec2 coords;\nuniform vec3 color;\nout vec4 frag_Color;\nvoid main(){\nfrag_Color.rgb = color;\nfrag_Color.a = 1.0-smoothstep(0.5, 1.0, abs(coords.y));\n}\n";

	static const char * _srg_pfstr = "#version 330\nin vec2 coords;\nuniform vec3 color;\nout vec4 fragColor;\nvoid main(){\nfragColor = vec4(color, 1.0-smoothstep(0.9, 1.0, length(coords)));\n}\n";
	
	
	/// OpenGL helpers.
	
	_srg_Buffers _srg_setDataBuffer(const float * data, const unsigned int count){
		// Create a vertex array and a vertex buffer.
		GLuint vaId;
		glGenVertexArrays(1, &vaId);
		glBindVertexArray(vaId);
		GLuint bufferPos;
		glGenBuffers(1, &bufferPos);
		glBindBuffer(GL_ARRAY_BUFFER, bufferPos);
		// Populate the buffer with the 2D vertices.
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * count, data, GL_STATIC_DRAW);
		// Setup the corresponding shader attribute at location 0.
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, bufferPos);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		glBindVertexArray(0);
		return { vaId, bufferPos, (GLsizei)count };
	}
	
	
	GLuint _srg_loadShader(const char * prog, GLuint type) {
		// Create shader, setup string as source and compile it.
		GLuint id = glCreateShader(type);
		glShaderSource(id, 1, &prog, (const GLint*)NULL);
		glCompileShader(id);
		// Check shader compilation status.
		GLint success;
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		// If compilation failed, get information and display it.
		if (success != GL_TRUE) {
			printf("Failed OpenGL shader setup.\n");
			return 0;
		}
		return id;
	}
	
	
	GLuint _srg_createGLProgram(const char* vertexString, const char * fragmentString) {
		GLuint id = glCreateProgram();
		// Load and attach shaders, then link them w/ the program.
		GLuint vp = _srg_loadShader(vertexString, GL_VERTEX_SHADER);
		GLuint fp = _srg_loadShader(fragmentString, GL_FRAGMENT_SHADER);
		glAttachShader(id, vp);
		glAttachShader(id, fp);
		glLinkProgram(id);
		//Check linking status.
		GLint success = GL_FALSE;
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		if (!success) {
			printf("Failed OpenGL setup.\n");
			return 0;
		}
		// Clean the shaders objects, detach and delete them.
		if (vp != 0) { glDetachShader(id, vp); }
		if (fp != 0) { glDetachShader(id, fp); }
		glDeleteShader(vp);
		glDeleteShader(fp);
		return id;
	}
	
	
	void _srg_internalSetup() {
		// Quads program.
		_srg_state.pid = _srg_createGLProgram(_srg_vstr, _srg_fstr);
		glUseProgram(_srg_state.pid);
		_srg_state.cid = glGetUniformLocation(_srg_state.pid, "color");
		_srg_state.rid = glGetUniformLocation(_srg_state.pid, "ratio");
		// Lines program.
		_srg_state.lpid = _srg_createGLProgram(_srg_vstr, _srg_lfstr);
		glUseProgram(_srg_state.lpid);
		_srg_state.lcid = glGetUniformLocation(_srg_state.lpid, "color");
		_srg_state.lrid = glGetUniformLocation(_srg_state.lpid, "ratio");
		// Points program.
		_srg_state.ppid = _srg_createGLProgram(_srg_vstr, _srg_pfstr);
		glUseProgram(_srg_state.ppid);
		_srg_state.pcid = glGetUniformLocation(_srg_state.ppid, "color");
		_srg_state.prid = glGetUniformLocation(_srg_state.ppid, "ratio");
		glUseProgram(0);
		// Quad data for full viewport clearing.
		const float quadData[12] = { -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f };
		_srg_state.bufferQuad = _srg_setDataBuffer(quadData, 6);
		 // @TODO: cleanly handle errors.
		_srg_isInit = true;
	}
	
}

#endif


/*
 ------------------------------------------------------------------------------
 This software is available under 2 licenses -- choose whichever you prefer.
 ------------------------------------------------------------------------------
 ALTERNATIVE A - MIT License
 Copyright (c) 2017 Simon Rodriguez
 Permission is hereby granted, free of charge, to any person obtaining a copy of
 this software and associated documentation files (the "Software"), to deal in
 the Software without restriction, including without limitation the rights to
 use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 of the Software, and to permit persons to whom the Software is furnished to do
 so, subject to the following conditions:
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 ------------------------------------------------------------------------------
 ALTERNATIVE B - Public Domain (www.unlicense.org)
 This is free and unencumbered software released into the public domain.
 Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
 software, either in source code form or as a compiled binary, for any purpose,
 commercial or non-commercial, and by any means.
 In jurisdictions that recognize copyright laws, the author or authors of this
 software dedicate any and all copyright interest in the software to the public
 domain. We make this dedication for the benefit of the public at large and to
 the detriment of our heirs and successors. We intend this dedication to be an
 overt act of relinquishment in perpetuity of all present and future rights to
 this software under copyright law.
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ------------------------------------------------------------------------------
*/
