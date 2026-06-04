#pragma once
/*
 * track.h
 *
 *  Created on: Sep 06, 2019
 *      Author: Namgyu Kim
 */

#ifndef TRACK_H_
#define TRACK_H_

#include <cmath>
#include "shader.h"

using namespace std;

class Track {

public:
	const float PI = 3.141592;
	const int ARC_NUM = 1024;
	const float THETA_STEP_ANGLE_SIZE = 2.0 * PI / ARC_NUM;
	const int vertex_num = 3 * ARC_NUM;
	const float RADIUS = 1.0;

	GLfloat track_vertices[3072]; // 1024 points * 3 axis
	GLfloat track_previous_vertices[3072];
	GLfloat track_time_slices[1024];

	unsigned int track_vertex_size;
	unsigned int track_previous_vertex_size;
	unsigned int track_time_slice_size;

	unsigned int VAO;
	unsigned int VBO;

	Track() {
		initVariables();
		initBuffers();
	};

	void draw(Shader *shader) {
		shader->use();
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE_STRIP, 0, (unsigned int) ARC_NUM);
		glBindVertexArray(0);
	}

	void translate(float dx, float dy, float dz) {
		for (unsigned int i = 0; i < track_vertex_size; i++) {
			if (i % 3 == 0) track_vertices[i] += dx;
			else if (i % 3 == 1) track_vertices[i] += dy;
			else if (i % 3 == 2) track_vertices[i] += dz;
		}

		updatePreviousPos();
	};

	void scale(float s) {
		for (unsigned int i = 0; i < this->track_vertex_size / sizeof(float); i++)
			track_vertices[i] *= s;

		updatePreviousPos();
	};

private:
	void initVariables() {
		float theta = 0;
		float x = 0, y = 0;
		int k = 0;

		for (int i = 0; i < ARC_NUM; i++) {

			theta = THETA_STEP_ANGLE_SIZE * i;
			x = this->RADIUS * cosf(theta);
			y = this->RADIUS * sinf(theta);

			track_vertices[k] = x;
			track_vertices[k + 1] = y;
			track_vertices[k + 2] = 0;

			track_time_slices[i] = (float)i;

			k += 3;
		}

		track_vertex_size = sizeof(track_vertices);
		track_time_slice_size = sizeof(track_time_slices);

		updatePreviousPos();
		track_previous_vertex_size = sizeof(track_previous_vertices);

}

	void initBuffers() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		glBindVertexArray(VAO);

		// copy vertex attrib data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, track_vertex_size + track_previous_vertex_size + track_time_slice_size, 0, GL_STATIC_DRAW);
		glBufferSubData(GL_ARRAY_BUFFER, 0, track_vertex_size, track_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, track_vertex_size, track_previous_vertex_size, track_previous_vertices);
		glBufferSubData(GL_ARRAY_BUFFER, track_vertex_size + track_previous_vertex_size, track_time_slice_size, track_time_slices);

		// attribute position initialization
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);  // cur position attrib
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) track_vertex_size);  // previous position attrib
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void *) (track_vertex_size + track_previous_vertex_size));  // time slices
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
//		glBindVertexArray(0);
	};

	void updatePreviousPos() {
		copy(track_vertices, track_vertices + vertex_num - 3, &track_previous_vertices[3]);
		copy(&track_vertices[vertex_num - 3], track_vertices + vertex_num, track_previous_vertices);
	};
};

#endif /* TRACK_H_ */
