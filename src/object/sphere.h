#pragma once

// Sphere description

#include <cmath>
#include "shader.h"

using namespace std;

class Sphere {

public:
	const float PI = 3.141592;
	const int SECTOR_NUM = 108; 		// 36
	const int STACK_NUM = 54; 			// 18
	const float THETA_STEP_ANGLE_SIZE = 2 * PI / SECTOR_NUM;
	const float PHI_STEP_ANGLE_SIZE = PI / STACK_NUM;
	float RADIUS = 1.0;

	GLfloat sphere_vertices[17985]; 	// 2109
	GLfloat sphere_normals[17985];
	GLfloat sphere_colors[23980];		// 2812
	GLfloat sphere_tex_coords[11990]; 	// 1406
	GLuint sphere_indices[34344]; 		//3672

	unsigned int sphere_vertex_size;
	unsigned int sphere_normal_size;
	unsigned int sphere_color_size;
	unsigned int sphere_tex_coord_size;
	unsigned int sphere_index_size;

	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;

	Sphere() {
		initVariables();
		initBuffers();
	};

	Sphere(float radius) {
		RADIUS = radius;
		initVariables();
		initBuffers();
	};

	void draw(Shader *shader) {
		shader->use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, (unsigned int) 34344, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	};

	void translate(float dx, float dy, float dz) {
		for (int i = 0; i < sphere_vertex_size; i++) {
			if (i % 3 == 0) sphere_vertices[i] += dx;
			else if (i % 3 == 1) sphere_vertices[i] += dy;
			else if (i % 3 == 2) sphere_vertices[i] += dz;
		}
	};

	void scale(float s) {
		for (int i = 0; i < this->sphere_vertex_size / sizeof(float); i++)
			sphere_vertices[i] *= s;
	};

private:
	void initVariables() {
		float theta = 0, phi = 0;
		float x = 0, y = 0, z = 0, xy = 0;
		int k = 0, l = 0;

		for (int i = 0; i <= STACK_NUM; i++) {
			phi = (PI / 2) - (PHI_STEP_ANGLE_SIZE * i);
			xy = this->RADIUS * cosf(phi);
			z = this->RADIUS * sinf(phi);

			for (int j = 0; j <= SECTOR_NUM; j++) {
				theta = THETA_STEP_ANGLE_SIZE * j;
				x = xy * cosf(theta);
				y = xy * sinf(theta);

				sphere_vertices[k] = x;
				sphere_vertices[k + 1] = y;
				sphere_vertices[k + 2] = z;

				sphere_normals[k] = x;
				sphere_normals[k + 1] = y;
				sphere_normals[k + 2] = z;

				k += 3;

				sphere_tex_coords[l] = (float)j / SECTOR_NUM;
				sphere_tex_coords[l + 1] = (float)i / STACK_NUM;

				l += 2;
			}
		}

		unsigned int v1, v2, m = 0;

		for (int i = 0; i < STACK_NUM; i++) {
			v1 = i * (SECTOR_NUM + 1);     // beginning of current stack
			v2 = v1 + SECTOR_NUM + 1;      // beginning of next stack

			for (int j = 0; j < SECTOR_NUM; j++, v1++, v2++) {
				// 2 triangles per sector excluding first and last stacks
				// k1 => k2 => k1+1
				if (i != 0) {
					sphere_indices[m] = v1;
					sphere_indices[m + 1] = v2;
					sphere_indices[m + 2] = v1 + 1;
					m += 3;
				}

				// k1+1 => k2 => k2+1
				if (i != (STACK_NUM - 1)) {
					sphere_indices[m] = v1 + 1;
					sphere_indices[m + 1] = v2;
					sphere_indices[m + 2] = v2 + 1;
					m += 3;
				}
			}
		}

		sphere_vertex_size = sizeof(sphere_vertices);
		sphere_normal_size = sizeof(sphere_normals);
		sphere_color_size = sizeof(sphere_colors);
		sphere_tex_coord_size = sizeof(sphere_tex_coords);
		sphere_index_size = sizeof(sphere_indices);
	}

	void initBuffers() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		// copy vertex attrib data to VBO
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sphere_vertex_size + sphere_normal_size + sphere_color_size + sphere_tex_coord_size, 0, GL_STATIC_DRAW);
		// Vertex
		glBufferSubData(GL_ARRAY_BUFFER, 0, sphere_vertex_size, sphere_vertices);
		// Normal
		glBufferSubData(GL_ARRAY_BUFFER, sphere_vertex_size, sphere_normal_size, sphere_normals);
		// 
		glBufferSubData(GL_ARRAY_BUFFER, sphere_vertex_size + sphere_normal_size, sphere_color_size, sphere_colors);
		glBufferSubData(GL_ARRAY_BUFFER, sphere_vertex_size + sphere_normal_size + sphere_color_size, sphere_tex_coord_size, sphere_tex_coords);

		// copy index data to EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere_index_size, sphere_indices, GL_STATIC_DRAW);

		// attribute position initialization
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);  // position attrib
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)sphere_vertex_size); // normal attrib
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(sphere_vertex_size + sphere_normal_size)); // color attrib
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)(sphere_vertex_size + sphere_normal_size + sphere_color_size)); // texture attrib
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

	};
};
