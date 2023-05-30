#include "textures.hpp"

/****************** TEXTURES **************/
void texture_t::create_from_file(const char *fname)
{
	int bytes_read = 0;
	char img_size[2];
	FILE* file = nullptr;
	if (fname){
		file = fopen(fname, "r");
		if (file == nullptr){
			printf("texture_t::create: cannot open file: %s\n", fname);
			return;
		}
		/* reading a header, that contains width and height of hte image */
		bytes_read = fread(img_size, 1, 2, file);
		if (bytes_read != 2){
			printf("texture_t::create: empty file %s\n", fname);
		}
		_width = img_size[0];
		_height = img_size[1];
		uint32_t file_size = _width * _height * sizeof(color16_t);
		_buffer = (color16_t*)heap_caps_malloc(file_size, MALLOC_CAP_DMA);
		/* reading 16bit color array */
		bytes_read = fread(_buffer, 1, file_size, file);
		if (bytes_read != file_size){
			printf("texture_t::create: wrong file or image side size - %d not %lu\n", bytes_read, file_size);
			fclose(file);
			return;
		}
	} else {
		puts("texture_t::create: empty filename");
		return;
	}
	fclose(file);
}

//to remake!!!!!!!!!
color16_t texture_t::get_pixel(uint8_t x, uint8_t y) const
{
	return _buffer[y * _width + x];
}

color16_t texture_t::get_pixel_normalized(float xnorm, float ynorm) const
{
	/* getiing rid of the integer part allows to repeat texture */
	xnorm -= floorf(xnorm);
	ynorm -= floorf(ynorm);

	uint8_t x = (uint8_t)(xnorm * _width);
	uint8_t y = (uint8_t)(ynorm * _height);
	return _buffer[y * _width + x];
}

void texture_t::create_generated(const uint8_t width, const uint8_t height, texture_t_generator_fn func)
{
	_width = width;
	_height = height;
	_buffer = (color16_t*)heap_caps_malloc(height * width * PIXEL_SIZE, MALLOC_CAP_DMA);
	if (_buffer == nullptr) {
		puts("texture_t: cannot allocate memory");
		return;
	}
	func(_buffer, width, height);
	return; 
}

void texture_t::textures_load(texture_t *textures[], int num)
{
	for (int i = 0; i != num; i++){
		textures[i] = new texture_t;
	}	
	/* solid objects	 */
	textures[0]->create_from_file("/spiffs/onyx_16b.bin");
	textures[1]->create_from_file("/spiffs/concrete_16b.bin");
	textures[2]->create_from_file("/spiffs/iron_16b.bin");
	textures[3]->create_from_file("/spiffs/stone2_16b.bin");
	textures[4]->create_from_file("/spiffs/stone1_16b.bin");
	textures[5]->create_from_file("/spiffs/brick2_16b.bin");
	textures[6]->create_from_file("/spiffs/wood_16b.bin");
	textures[7]->create_from_file("/spiffs/door_16b.bin");
	/* dynamic objects */
	textures[8]->create_from_file("/spiffs/zombie_16b.bin");
	textures[9]->create_from_file("/spiffs/scorpion_16b.bin");
	textures[10]->create_from_file("/spiffs/heart_16b.bin");
	textures[11]->create_from_file("/spiffs/fireball_16b.bin");
	textures[12]->create_from_file("/spiffs/ammo_16b.bin");
	textures[13]->create_from_file("/spiffs/barrel_16b.bin");
	textures[14]->create_from_file("/spiffs/lamp1_16b.bin");
	textures[15]->create_from_file("/spiffs/burer64_16b.bin");
}

/* red bricks */
void texture_t::Generate_Tex_1(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h)
{
	for (int j = 0; j != tex_h; j++){
		for (int i = 0; i != tex_w; i++){
			color16_t color = cRED;
			color = color * (i % 8 && j % 8);
			tex[j * tex_w + i] = color;
		}
	}
}

/* y gradient */
void texture_t::Generate_Tex_2(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h)
{
	for (int j = 0; j != tex_h; j++){
		for (int i = 0; i != tex_w; i++){
			color16_t color = cBLUE;
			color = (color << 8)|(color >> 8);
			color = ((j * color) / tex_h);
			color = (color << 8)|(color >> 8);
			tex[j * tex_w + i] = color;
		}
	}
}

/* x gradient */
void texture_t::Generate_Tex_3(color16_t* tex, const uint8_t tex_w, const uint8_t tex_h)
{
	for (int j = 0; j != tex_h; j++){
		for (int i = 0; i != tex_w; i++){
			color16_t color = cBLUE;
			color = (color << 8)|(color >> 8);
			color = ((i * color) / tex_h);
			color = (color << 8)|(color >> 8);
			tex[j * tex_w + i] = color;
		}
	}
}
