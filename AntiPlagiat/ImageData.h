#pragma once

//#define _CRT_SECURE_NO_WARNINGS // disable _s warnings
//#pragma warning(suppress : 4996)


#include <jpeglib.h>
#include <setjmp.h>
#include <vector>
#include <stdio.h>
#include <cstddef>
#include <iostream>
#include <sstream>

class ImageData {

public:
	ImageData(std::string path);
	const static uint8_t scale_coeff = 7;
	const static uint8_t match_threshold = 90;
	bool hashMatch(std::string otherImageHash);
	static bool hashCompare(std::string imageHash, std::string otherImageHash);
	bool match(ImageData otherImage);
	uint16_t width() const { return _width; }
	uint16_t height() const { return _height; }
	std::string hash() const { return _hash; }

private:
	char * _path;
	uint16_t _width = 0, _height = 0, _row_stride = 0;
	//If we compare hash
	unsigned char *** _image = nullptr;
	unsigned char ** _buffer = nullptr;
	std::string _hash;

	std::string calculateMeanHash();

	//from libjpeg-turbo example
	struct my_error_mgr {
		struct jpeg_error_mgr pub;    /* "public" fields */
		jmp_buf setjmp_buffer;        /* for return to caller */
	};

	typedef struct my_error_mgr *my_error_ptr;
	struct jpeg_decompress_struct cinfo;
	static void my_error_exit(j_common_ptr cinfo);

	bool read_JPEG_file(char * filename);
	bool do_read_JPEG_file(struct jpeg_decompress_struct *cinfo, char *filename);
};