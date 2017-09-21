#define GL_LITE_IMPLEMENTATION
#include "gl_lite.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GLFWwindow* window = NULL;
int gl_width = 1080;
int gl_height = 720;
float gl_aspect_ratio = (float)gl_width/gl_height;
bool gl_fullscreen = false;

#include "GameMaths.h"
#include "Input.h"
#include "Camera3D.h"
#include "init_gl.h"
#include "load_obj.h"
#include "Shader.h"
#include "load_texture.h"
#include "Lighting.h"

int main(){
	if(!init_gl(window, "Basic Lighting", gl_width, gl_height)){ return 1; }

	//Load a cube
	GLuint cube_vao;
	unsigned int cube_num_indices = 0;
	{
		float* vp = NULL;
		float* vn = NULL;
		float* vt = NULL;
		uint16_t* indices = NULL;
		unsigned int num_verts = 0;
		load_obj_indexed("cube.obj", &vp, &vt, &vn, &indices, &num_verts, &cube_num_indices, false);

		glGenVertexArrays(1, &cube_vao);
		glBindVertexArray(cube_vao);
		
		GLuint points_vbo;
		glGenBuffers(1, &points_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, points_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vp, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VP_ATTRIB_LOC);
		glVertexAttribPointer(VP_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vp);

		GLuint tex_coords_vbo;
		glGenBuffers(1, &tex_coords_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, tex_coords_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*2*sizeof(float), vt, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VT_ATTRIB_LOC);
		glVertexAttribPointer(VT_ATTRIB_LOC, 2, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vt);

		GLuint normals_vbo;
		glGenBuffers(1, &normals_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, normals_vbo);
		glBufferData(GL_ARRAY_BUFFER, num_verts*3*sizeof(float), vn, GL_STATIC_DRAW);
		glEnableVertexAttribArray(VN_ATTRIB_LOC);
		glVertexAttribPointer(VN_ATTRIB_LOC, 3, GL_FLOAT, GL_FALSE, 0, NULL);
		free(vn);

		GLuint index_vbo;
		glGenBuffers(1, &index_vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_vbo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_num_indices*sizeof(unsigned short), indices, GL_STATIC_DRAW);
		free(indices);
	}

	//Load a texture
	GLuint test_texture_id;
	assert(load_texture("test.png", &test_texture_id));

	init_camera(&g_camera, vec3(0,2,5), vec3(0,0,0));
	
    //Load shaders
	Shader blinn_phong_shader = init_shader("Blinn_Phong.vert", "Blinn_Phong.frag");
	GLuint diffuse_map_loc = glGetUniformLocation(blinn_phong_shader.id, "material.diffuse_map");
	GLuint normal_mat_loc  = glGetUniformLocation(blinn_phong_shader.id, "normal_mat");

	GLuint dir_light_dir_eye_loc   = glGetUniformLocation(blinn_phong_shader.id, "dir_light.dir_eye");
	GLuint dir_light_colour_loc    = glGetUniformLocation(blinn_phong_shader.id, "dir_light.colour");
	GLuint point_light_pos_eye_loc = glGetUniformLocation(blinn_phong_shader.id, "point_light.pos_eye");
	GLuint point_light_colour_loc  = glGetUniformLocation(blinn_phong_shader.id, "point_light.colour");

	g_dir_light.dir = vec3(0,1,-1);
	g_dir_light.colour = vec3(1,1,1);
	g_point_light.pos = vec3(4,4,-4);
	g_point_light.colour = vec3(1,1,1);
	
	check_gl_error();

    double curr_time = glfwGetTime(), prev_time, dt;
	//-------------------------------------------------------------------------------------//
	//-------------------------------------MAIN LOOP---------------------------------------//
	//-------------------------------------------------------------------------------------//
	while(!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Get dt
		prev_time = curr_time;
		curr_time = glfwGetTime();
		dt = curr_time - prev_time;
		if(dt > 0.1) dt = 0.1;
		
		//Get Input
		g_mouse.prev_xpos = g_mouse.xpos;
    	g_mouse.prev_ypos = g_mouse.ypos;
		glfwPollEvents();
		poll_joystick();
		
		//Check miscellaneous button presses
		{
			if(glfwGetKey(window, GLFW_KEY_ESCAPE)) {
				glfwSetWindowShouldClose(window, 1);
			}

			//M to toggle between mouse/arrow key controls for camera
			static bool m_was_pressed = false;
			if(glfwGetKey(window, GLFW_KEY_M)) {
				if(!m_was_pressed) { cam_mouse_controls = !cam_mouse_controls; }
				m_was_pressed = true;
			}
			else m_was_pressed = false;

			//Ctrl/Command-F to toggle fullscreen
			//Note: window_resize_callback takes care of resizing viewport/recalculating P matrix
			static bool F_was_pressed = false;
			if(glfwGetKey(window, GLFW_KEY_F)) {
				if(!F_was_pressed){
					if(glfwGetKey(window, CTRL_KEY_LEFT) || glfwGetKey(window, CTRL_KEY_RIGHT)){
						gl_fullscreen = !gl_fullscreen;
						static int old_win_x, old_win_y, old_win_w, old_win_h;
						if(gl_fullscreen){
							glfwGetWindowPos(window, &old_win_x, &old_win_y);
							glfwGetWindowSize(window, &old_win_w, &old_win_h);
							GLFWmonitor* mon = glfwGetPrimaryMonitor();
							const GLFWvidmode* vidMode = glfwGetVideoMode(mon);
							glfwSetWindowMonitor(window, mon, 0, 0, vidMode->width, vidMode->height, vidMode->refreshRate);
						}
						else glfwSetWindowMonitor(window, NULL, old_win_x, old_win_y, old_win_w, old_win_h, GLFW_DONT_CARE);
					}
				}
				F_was_pressed = true;
			}
			else F_was_pressed = false;
		}

		//Update camera
		update_camera_debug(&g_camera, dt);

		//Use shader
		{
			glUseProgram(blinn_phong_shader.id);

			//Per-shader data
			glUniformMatrix4fv(blinn_phong_shader.V_loc, 1, GL_FALSE, g_camera.V.m);
			glUniformMatrix4fv(blinn_phong_shader.P_loc, 1, GL_FALSE, g_camera.P.m);
			
			glUniform1i(diffuse_map_loc, 0); //Tell shader what texture slot to read diffuse map from
			
			//Draw cube
			{
				//Per-model lighting uniforms
				{
					vec3 dir_light_dir_eye = normalise(vec3(g_camera.V*vec4(g_dir_light.dir, 0)));
					glUniform3fv(dir_light_dir_eye_loc, 1, dir_light_dir_eye.v);
					glUniform3fv(dir_light_colour_loc, 1, g_dir_light.colour.v);
					
					vec3 point_light_pos_eye = vec3(g_camera.V*vec4(g_point_light.pos, 1));
					glUniform3fv(point_light_pos_eye_loc, 1, point_light_pos_eye.v);
					glUniform3fv(point_light_colour_loc, 1, g_point_light.colour.v);
				}
				
				//Per-model data (vao, textures, uniforms)
				glBindVertexArray(cube_vao);
				
				glActiveTexture(GL_TEXTURE0); //Activate a texture slot
				glBindTexture(GL_TEXTURE_2D, test_texture_id); //Bind our texture to that slot

				mat4 model_matrix = identity_mat4();
				glUniformMatrix4fv(blinn_phong_shader.M_loc, 1, GL_FALSE, model_matrix.m);
				glUniformMatrix3fv(normal_mat_loc, 1, GL_FALSE, mat3(transpose(inverse(g_camera.V * model_matrix))).m);
				
				glDrawElements(GL_TRIANGLES, cube_num_indices, GL_UNSIGNED_SHORT, 0);
			}
		}

		glfwSwapBuffers(window);

		check_gl_error();
	}//end main loop

    return 0;
}
