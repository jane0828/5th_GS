#pragma once
/*
 * cone.h
 *
 *  Created on: Oct 02, 2019
 *      Author: Namgyu Kim
 */

#ifndef CONE_H_
#define CONE_H_

#include <cmath>
#include "shader.h"

using namespace std;

class Cone {

public:
	const float PI = 3.141592;
	const int ARC_NUM = 1024;
	const float THETA_STEP_ANGLE_SIZE = 2 * PI / ARC_NUM;
	const int vertex_num = 3078;
	float RADIUS = 1.0;

	GLfloat cone_vertices[3078]; // 3 * (1025 + 1) (1025 for loop, 1 for cone's edge)
	GLfloat cone_previous_vertices[3078];


	unsigned int cone_vertex_size;
	unsigned int cone_previous_vertex_size;

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	Cone() {
		initVariables();
		initBuffers();
	};

	void draw(Shader *shader) {
		shader->use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, (unsigned int) 1026);
		glDrawArrays(GL_LINE_STRIP, 1, (unsigned int) 1025);
		glBindVertexArray(0);
	};

	void translate(float dx, float dy, float dz) {
		for (unsigned int i = 0; i < cone_vertex_size; i++) {
			if (i % 3 == 0) cone_vertices[i] += dx;
			else if (i % 3 == 1) cone_vertices[i] += dy;
			else if (i % 3 == 2) cone_vertices[i] += dz;
		}

		updatePreviousPos();
	};

	void scale(float s) {
		for (unsigned int i = 0; i < this->cone_vertex_size / sizeof(float); i++)
			cone_vertices[i] *= s;

		updatePreviousPos();
	};

private:
	void initVariables() {
		float theta = 0;
		float x = 0, z = 0;
		int k = 0;

		cone_vertices[k] = 0;
		cone_vertices[k + 1] = 0.5;
		cone_vertices[k + 2] = 0;

		k += 3;

		for (int i = 0; i <= ARC_NUM; i++) {

			theta = THETA_STEP_ANGLE_SIZE * i;
			x = this->RADIUS * cosf(theta);
			z = this->RADIUS * sinf(theta);

			cone_vertices[k] = x;
			cone_vertices[k + 1] = -0.5;
			cone_vertices[k + 2] = z;

			k += 3;
		}

		cone_vertex_size = sizeof(cone_vertices);

		updatePreviousPos();
		cone_previous_vertex_size = sizeof(cone_previous_vertices);
}

	void initBuffers() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		// copy vertex attrib data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, cone_vertex_size + cone_previous_vertex_size, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, cone_vertex_size, cone_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, cone_vertex_size, cone_previous_vertex_size, cone_previous_vertices);


		// attribute position initialization
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);  // cur position attrib
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) cone_vertex_size);  // cur position attrib
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		glBindVertexArray(0);
	};

	void updatePreviousPos() {
			copy(&cone_vertices[3], cone_vertices + vertex_num - 3, cone_previous_vertices);
			copy(cone_vertices, cone_vertices + 3, &cone_previous_vertices[vertex_num - 3]);
	};
};

#endif /* CONE_H_ */
