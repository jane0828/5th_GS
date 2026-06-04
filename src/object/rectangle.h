#pragma once
/*
 * rectangle.h
 *
 *  Created on: Sep 20, 2019
 *      Author: Namgyu Kim
 */

#ifndef RECTANGLE_H_
#define RECTANGLE_H_

#include <cmath>
#include "shader.h"

using namespace std;

class Rectangle {

public:
	GLfloat rectangle_vertices[12] =
	{
			-1.0, -1.0, 0.0,	1.0, -1.0, 0.0,		-1.0, 1.0, 0.0,		1.0, 1.0, 0.0
	};

	GLfloat rectangle_tex_coords[8] =
	{
			0.0, 1.0,		1.0, 1.0,		0.0, 0.0,		1.0, 0.0
	};

	bool _fullscreen;

	unsigned int rectangle_vertex_size = 48;
	unsigned int rectangle_tex_coord_size = 32;
	unsigned int rectangle_index_size = 24;

	unsigned int VAO;
	unsigned int VBO;

	Rectangle()
	:_fullscreen(true)
	{
		InitBuffers();
	};

	Rectangle(bool fullscreen)
	:_fullscreen(fullscreen)
	{
		if(!fullscreen)
		{
			this->XYZScale(9.0 / 16.0, 1.0, 1.0);
			this->XYTranslate(0.0, 0.0);
			InitBuffers();
		}
	}

	void Draw(Shader *shader) {
		shader->use();
		glBindVertexArray(VAO);
		this->Update();
		glDrawArrays(GL_TRIANGLE_STRIP, 0, (unsigned int) 4);
		glBindVertexArray(0);
	};

	void Translate(float dx, float dy, float dz) {
		for (unsigned int i = 0; i < rectangle_vertex_size; i++) {
			if (i % 3 == 0) rectangle_vertices[i] += dx;
			else if (i % 3 == 1) rectangle_vertices[i] += dy;
			else if (i % 3 == 2) rectangle_vertices[i] += dz;
		}
	};

	float Xwidth() {
		return (rectangle_vertices[3] - rectangle_vertices[0] + rectangle_vertices[9] - rectangle_vertices[6])/2;
	}

	float Ywidth() {
		return (rectangle_vertices[7] - rectangle_vertices[1] + rectangle_vertices[10] - rectangle_vertices[4])/2;
	}

	void XYTranslate(float x, float y) {
		
		float x_width = this->Xwidth();
		float y_width = this->Ywidth();
		rectangle_vertices[0] = x - x_width / 2.0;
		rectangle_vertices[3] = x + x_width / 2.0;
		rectangle_vertices[6] = x - x_width / 2.0;
		rectangle_vertices[9] = x + x_width / 2.0;
		rectangle_vertices[1] = y - y_width / 2.0;
		rectangle_vertices[4] = y - y_width / 2.0;
		rectangle_vertices[7] = y + y_width / 2.0;
		rectangle_vertices[10] = y + y_width / 2.0;
	};

	void Scale(float s) {
		for (unsigned int i = 0; i < rectangle_vertex_size / sizeof(float); i++)
			rectangle_vertices[i] *= s;
	};

	void XYZScale(float Xscale, float Yscale, float Zscale) {
		for (unsigned int i = 0; i < rectangle_vertex_size / (3 * sizeof(float)); i++)
		{
			rectangle_vertices[3 * i] *= Xscale;
			rectangle_vertices[3 * i + 1] *= Yscale;
			rectangle_vertices[3 * i + 2] *= Zscale;
		}	
	}

	void Update() {
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, rectangle_vertex_size + rectangle_tex_coord_size, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, rectangle_vertex_size, rectangle_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, rectangle_vertex_size, rectangle_tex_coord_size, rectangle_tex_coords);
	}

private:
	void InitBuffers() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		// copy vertex attrib data to VBO
		this->Update();
		// attribute position initialization
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);  // position attrib
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *) rectangle_vertex_size);  // tex_coord attrib
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		glBindVertexArray(0);

	};
};

#endif /* CIRCLE_H_ */
