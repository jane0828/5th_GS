#pragma once
//
//  Arcball.h
//
//  Created by Saburo Okita on 12/03/14.
//  Copyright (c) 2014 Saburo Okita. All rights reserved.
//

#ifndef __Arcball__Arcball__
#define __Arcball__Arcball__ 

#define GLM_ENABLE_EXPERIMENTAL

#include <iostream>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_inverse.hpp>

class Arcball {
private:
    int _windowWidth;
    int _windowHeight;
    float _roll_speed;
    float _angle ;
    int _mouse_event;
    glm::vec3 _prev_pos;
    glm::vec3 _curr_pos;
    glm::vec3 _cam_axis;
    glm::mat4 _rotate_coord;
    
    bool _x_axis;
    bool _y_axis;
    
public:
    
    /**
     * Constructor.
     * @param roll_speed: the speed of rotation
     */
    Arcball(int window_width, int window_height, GLfloat roll_speed, bool x_axis, bool y_axis)
	: _windowWidth(window_width), _windowHeight(window_height),
	  _roll_speed(roll_speed), _angle(0.0), _mouse_event(0),
	  _cam_axis(0.0f, 1.0f, 0.0f), _rotate_coord(1.0f), _x_axis(x_axis), _y_axis(y_axis)
	{
    }
    
    /**
     * Init: all initialization
     */
    
    void UpdateWindowSize(int window_width, int window_height)
    {
    	_windowWidth  = window_width;
		_windowHeight = window_height;
    }

    /**
     * Convert the mouse cursor coordinate on the window (i.e. from (0,0) to (windowWidth, windowHeight))
     * into normalized screen coordinate (i.e. (-1, -1) to (1, 1)
     */
    glm::vec3 toScreenCoord( double x, double y ) {
        glm::vec3 coord(0.0f);
        
        if( _x_axis )
            coord.x =  (2 * (float)x / _windowWidth ) - 1.0f;
        
        if( _y_axis )
            coord.y = -((2 * (float)y / _windowHeight) -1.0f);
        
        // Find nearest axis from camera, and use rather than just x, y

        /* Clamp it to border of the windows, comment these codes to allow rotation when cursor is not over window */
//        coord.x = glm::clamp( coord.x, -1.0f, 1.0f );
//        coord.y = glm::clamp( coord.y, -1.0f, 1.0f );
        
        float length_squared = coord.x * coord.x + coord.y * coord.y;
        if( length_squared <= 1.0 )
            coord.z = sqrt( 1.0 - length_squared );
        else
            coord = glm::normalize( coord );

        glm::vec4 real_coord = glm::vec4(coord, 0.0f) * _rotate_coord;

        return glm::vec3(real_coord.x, real_coord.y, real_coord.z);
    }
    
    /**
     * Check whether we should start the mouse event
     * Event 0: when no tracking occured
     * Event 1: at the start of tracking, recording the first cursor pos
     * Event 2: tracking of subsequent cursor movement
     */
    void mouseButtonCallback( GLFWwindow * window, int button, int action, int mods ){
        _mouse_event = ( action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT );
    }
    
    void cursorCallback( GLFWwindow *window, double x, double y ){
        if( _mouse_event == 0 )
        {
        	// rotateCoord is rotation transformation matrix
        	// which accumulates all rotation so far.
        	// It is always single 4x4 matrix, computation load is very low
        	_rotate_coord = glm::rotate(_rotate_coord, glm::degrees(_angle) * _roll_speed, _cam_axis);
        	return;
        }
        else if( _mouse_event == 1 ) {
            // Start of trackball, remember the first position
            _prev_pos     = toScreenCoord( x, y );
            _mouse_event  = 2;
            return;
        }
        
        // Jobs of mouseEvent == 2
        // Transform width, height screen positions to x, y, z in 3D space
        _curr_pos  = toScreenCoord( x, y );
        
        // Calculate rotation angle in radians, and clamp it between 0 and 90 degrees
        float dot = (float)glm::dot(_prev_pos, _curr_pos);
        _angle = acos(((1.0f < dot) ? 1.0f : dot));
        
        // Although left mouse button clicked, an object could not rotate at all
        // in that case, curPos and _prev_pos are same and its cross product is just zero.
        if (_curr_pos != _prev_pos)
        	_cam_axis  = glm::cross( _prev_pos, _curr_pos ); // Do cross product to get the rotation axis
    }
    
    /**
     * Create rotation matrix
     * multiply this matrix with view (or model) matrix to rotate the camera (or model)
     */
    glm::mat4 createRotationMatrix() {
        return glm::rotate( glm::degrees(_angle) * _roll_speed, _cam_axis );
    }

    void setAngle(float angle) {
    	_angle = angle;
    }

    void setRotateCoord(glm::mat4 mat) {
    	_rotate_coord = mat;
    }
};

#endif /* defined(__Arcball__Arcball__) */
