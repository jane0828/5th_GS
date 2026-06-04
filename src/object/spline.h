#pragma once
/*
 * spline.h
 *
 *  Created on: Sep 26, 2019
 *      Author: Namgyu Kim
 */

#ifndef SPLINE_H_
#define SPLINE_H_

#include <cmath>
#include "shader.h"

using namespace std;

class Spline {

public:
	int ARC_NUM = 1024;
	GLfloat spline_vertices[3072];
	GLfloat spline_colors[3072];

	unsigned int spline_vertex_size;
	unsigned int spline_color_size;

	unsigned int VAO;
	unsigned int VBO;

	GLfloat RGB[3];

	Spline(float dx, float dy, float dz, float s) {
		scale(s);
		translate(dx, dy, dz);
		initBuffers();
	};

	Spline() {
		initBuffers();
	};

	void draw(Shader *shader) {
		shader->use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE_STRIP, 0, (unsigned int) ARC_NUM);
		glBindVertexArray(0);
	};

	void translate(float dx, float dy, float dz) {
		for (unsigned int i = 0; i < spline_vertex_size; i++) {
			if (i % 3 == 0) spline_vertices[i] += dx;
			else if (i % 3 == 1) spline_vertices[i] += dy;
			else if (i % 3 == 2) spline_vertices[i] += dz;
		}
	};

	void scale(float s) {
		for (unsigned int i = 0; i < this->spline_vertex_size / sizeof(float); i++)
			spline_vertices[i] *= s;
	};

	void XYZscale(float Xscale, float Yscale, float Zscale) {
		for (unsigned int i = 0; i < spline_vertex_size; i++) {
			if (i % 3 == 0) spline_vertices[i] *= Xscale;
			else if (i % 3 == 1) spline_vertices[i] *= Yscale;
			else if (i % 3 == 2) spline_vertices[i] *= Zscale;
		}
	};

	void Setcolor(GLfloat red, GLfloat green, GLfloat blue) {
		this->RGB[0] = red;
		this->RGB[1] = green;
		this->RGB[2] = blue; 
		int k = 0;

		for (int i = 0; i < ARC_NUM; i++) {

			spline_colors[k] = this->RGB[0];
			spline_colors[k + 1] = this->RGB[1];
			spline_colors[k + 2] = this->RGB[2];

			k += 3;
		}
	}

	void Update() {
		// copy vertex attrib data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, spline_vertex_size + spline_color_size, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, spline_vertex_size, spline_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, spline_vertex_size, spline_color_size, spline_colors);
	};

private:

	void initBuffers() {
		spline_vertex_size = sizeof(spline_vertices);
		spline_color_size = sizeof(spline_colors);
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		this->Update();

		// attribute position initialization
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);  // position attrib
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) spline_vertex_size);	// color attrib
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	};
};

class StraightLine {

public:
	GLfloat vertices[6];
	GLfloat colors[6];

	unsigned int vertex_size;
	unsigned int color_size;

	unsigned int VAO;
	unsigned int VBO;

	StraightLine(GLfloat x0, GLfloat y0, GLfloat z0, GLfloat x1, GLfloat y1, GLfloat z1) {
		vertices[0] = x0;
		vertices[1] = y0;
		vertices[2] = z0;
		vertices[3] = x1;
		vertices[4] = y1;
		vertices[5] = z1;
		initBuffers();
	};

	StraightLine() {
		initBuffers();
	};

	void draw(Shader *shader) {
		shader->use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, (unsigned int) 2);
		glBindVertexArray(0);
	};

	void Setpoint(GLfloat x0, GLfloat y0, GLfloat z0, GLfloat x1, GLfloat y1, GLfloat z1) {
		vertices[0] = x0;
		vertices[1] = y0;
		vertices[2] = z0;
		vertices[3] = x1;
		vertices[4] = y1;
		vertices[5] = z1;
	}

	void translate(float dx, float dy, float dz) {
		for (unsigned int i = 0; i < this->vertex_size / sizeof(float); i++) {
			if (i % 3 == 0) vertices[i] += dx;
			else if (i % 3 == 1) vertices[i] += dy;
			else if (i % 3 == 2) vertices[i] += dz;
		}
	};

	void scale(float s) {
		for (unsigned int i = 0; i < this->vertex_size / sizeof(float); i++)
			vertices[i] *= s;
	};

	void XYZscale(float Xscale, float Yscale, float Zscale) {
		for (unsigned int i = 0; i < this->vertex_size / sizeof(float); i++) {
			if (i % 3 == 0) vertices[i] *= Xscale;
			else if (i % 3 == 1) vertices[i] *= Yscale;
			else if (i % 3 == 2) vertices[i] *= Zscale;
		}
	};

	void Setcolor(GLfloat red, GLfloat green, GLfloat blue) {

		for (unsigned int i = 0; i < this->color_size / sizeof(float); i++) {
			if (i % 3 == 0)
				this->colors[i] = red;
			else if (i % 3 == 1)
				this->colors[i] = green;
			else if (i % 3 == 2)
				this->colors[i] = blue;
		}
	}

	void Update() {
		// copy vertex attrib data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertex_size + color_size, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_size, vertices);
		glBufferSubData(GL_ARRAY_BUFFER, vertex_size, color_size, colors);
	};

private:

	void initBuffers() {
		vertex_size = sizeof(vertices);
		color_size = sizeof(colors);
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		this->Update();

		// attribute position initialization
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);  // position attrib
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) vertex_size);	// color attrib
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	};
};

#endif _SPLINE_H_
