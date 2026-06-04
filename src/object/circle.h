#pragma once
/*
 * circle.h
 *
 *  Created on: Sep 06, 2019
 *      Author: Namgyu Kim
 */

#ifndef CIRCLE_H_
#define CIRCLE_H_

#include <cmath>
#include "shader.h"
#define _USE_MATH_DEFINES
using namespace std;

class Circle {

private:
	const int ARC_NUM = 1024;
	const float THETA_STEP_ANGLE_SIZE = 2 * M_PI / ARC_NUM;
	const int vertex_num = 3 * ARC_NUM;
	float RADIUS = 1.0;

	GLfloat vertices[3072];  // 1024 points * 3 axis
	GLfloat colors[3072];
	GLfloat texcoords[2048];

	unsigned int vertex_size;
	unsigned int color_size;
	unsigned int texcoord_size;

	unsigned int VAO;
	unsigned int VBO;

public:
	Circle() {
		initVariables();
		initBuffers();
	};

	void drawLoop(Shader *shader) {
		shader->use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE_LOOP, 0, (unsigned int) ARC_NUM);
		glBindVertexArray(0);
	};

	void drawFan(Shader *shader) {
		shader->use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (unsigned int) ARC_NUM);
		glBindVertexArray(0);
	};

	void translate(float dx, float dy, float dz) {
		for (unsigned int i = 0; i < vertex_size / sizeof(float); i++) {
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
		for (unsigned int i = 0; i < vertex_size / sizeof(float); i++) {
			if (i % 3 == 0) vertices[i] *= Xscale;
			else if (i % 3 == 1) vertices[i] *= Yscale;
			else if (i % 3 == 2) vertices[i] *= Zscale;
		}
	};

	void Setradius(float Rad)
	{
		this->RADIUS = Rad;
		initVariables();
	};

	void Scaleradius(float s)
	{
		this->RADIUS *= s;
		initVariables();
	};

	void Mercator(float latitude, float longitude)
	{
		float theta = 0, theta_lon = 0, theta_lat = 0;
		float x = 0, y = 0;
		int k = 0;
		for(int i = 0; i < ARC_NUM; i++)
		{
			theta_lon = THETA_STEP_ANGLE_SIZE * i;
			theta_lat = THETA_STEP_ANGLE_SIZE * i;
			y = this->RADIUS * sinf(theta_lat);
			x = this->RADIUS * cosf(theta_lon) / cosf(latitude + this->RADIUS * sinf(theta_lon));
			vertices[k] = x;
			vertices[k + 1] = y;
			vertices[k + 2] = 0;
			k += 3;
		}
		this->XYZscale(0.5f, 1.0f, 1.0f);
		this->translate(longitude / (M_PI), latitude / (M_PI * 0.5), 0.0f);

		k = 0;
		// for(int i = 0; i < ARC_NUM; i++)
		// {
		// 	if(vertices[k] > 1.0f)
		// 	{
		// 		vertices[k] = 1.0f;
		// 		if(latitude > 0)
		// 			vertices[k + 1] = 1.0f;
		// 		else
		// 			vertices[k + 1] = -1.0f;
		// 	}
				
		// 	else if (vertices[k] < -1.0f)
		// 	{
		// 		vertices[k] = -1.0f;
		// 		if(latitude > 0)
		// 			vertices[k + 1] = 1.0f;
		// 		else
		// 			vertices[k + 1] = -1.0f;
		// 	}
				
		// 	k += 3;
		// }
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
	};

	void Update() {
		// copy vertex attrib data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, vertex_size + color_size, 0, GL_STATIC_DRAW);
		// Vertex
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertex_size, vertices);
		// Color
		glBufferSubData(GL_ARRAY_BUFFER, vertex_size, color_size, colors);
	};

private:
	void initVariables() {
		float theta = 0;
		float x = 0, y = 0;
		int k = 0;
		for (int i = 0; i < ARC_NUM; i++)
		{
			theta = THETA_STEP_ANGLE_SIZE * i;
			x = this->RADIUS * cosf(theta);
			y = this->RADIUS * sinf(theta);

			vertices[k] = x;
			vertices[k + 1] = y;
			vertices[k + 2] = 0;
			k += 3;
		}
		

		vertex_size = sizeof(vertices);
		color_size = sizeof(colors);
	};

	void initBuffers() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		this->Update();
		

		// attribute position initialization
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);  // position attrib
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) vertex_size);  // color attrib
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	};
};

#endif /* CIRCLE_H_ */
