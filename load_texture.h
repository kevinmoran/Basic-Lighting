#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TEX_PATH "Textures/"

bool load_texture (const char* file_name, GLuint* tex) {
	char tex_file_path[64];
	sprintf(tex_file_path, "%s%s", TEX_PATH, file_name);
	
	int image_width, image_height, num_channels;
	int force_channels = 4;
	unsigned char* image_data = stbi_load(tex_file_path, &image_width, &image_height, &num_channels, force_channels);

	if (!image_data) {
		fprintf (stderr, "ERROR: could not load %s\n", file_name);
		return false;
	}

	// Power-of-2 check
	if ((image_width & (image_width - 1)) != 0 || (image_height & (image_height - 1)) != 0) {
		fprintf(stderr, "WARNING: texture %s is not power-of-2 dimensions\n", file_name);
	}
	
	//Flip image vertically (OpenGL expects y=0 at bottom of image)
	{
		int width_in_bytes = image_width * 4;
		int half_height = image_height / 2;
		
		for (int row = 0; row < half_height; row++) {
			unsigned char *top = image_data + row * width_in_bytes;
			unsigned char *bottom = image_data + (image_height - row - 1) * width_in_bytes;

			for (int col = 0; col < width_in_bytes; col++) {
				unsigned char temp = *top;
				*top = *bottom;
				*bottom = temp;
				top++;
				bottom++;
			}
		}
	}

	//Generate the texture 
	glGenTextures (1, tex);
	glActiveTexture (GL_TEXTURE0);
	glBindTexture (GL_TEXTURE_2D, *tex);
	glTexImage2D (
		GL_TEXTURE_2D,  //target
		0,			    //level-of-detail number
		GL_RGBA,	    //internal format
		image_width,	//width
		image_height,	//height
		0,			   	//border (must be zero, legacy)
		GL_RGBA,	   	//format
		GL_UNSIGNED_BYTE, //type
		image_data		//data
	);

	stbi_image_free(image_data);

	glGenerateMipmap (GL_TEXTURE_2D);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	GLfloat max_aniso = 0.0f;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
	// set the maximum!
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_aniso);
	return true;
}
