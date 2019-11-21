#include "stdafx.h"
#include "ImageData.h"

ImageData::ImageData(std::string path)
{
	//_path = path.c_str();
	std::vector<char> writable(path.begin(), path.end());
	writable.push_back('\0');
	_path = &writable[0];
	read_JPEG_file(_path);
	calculateMeanHash();
}

//Match Algorithm
bool ImageData::match(ImageData otherImage) {
	return (_hash == otherImage.hash());
}

//Match Algorithm
bool ImageData::hashMatch(std::string otherImageHash) {
	std::stringstream ss;
	ss << _hash;
	std::string hash = ss.str();
	return hashCompare(hash, otherImageHash);
}

bool ImageData::hashCompare(std::string imageHash, std::string otherImageHash) {
	return (imageHash.compare(otherImageHash) == 0);
}



//aHash (also called Average Hash or Mean Hash). This approach crushes the image into a grayscale 8x8 image and sets the 64 bits in the hash based on whether the pixel's value is greater than the average color for the image.
std::string ImageData::calculateMeanHash() {
	//Open the image
	if (_buffer == nullptr)
	{
		read_JPEG_file(_path);
	}
	//Transform it to string
	std::string imageCode(reinterpret_cast<char*>(_buffer));
	//Calculate Hash
	std::size_t h = std::hash<std::string>()(imageCode);
	std::cout << "Hash  = " << h << '\n';
	std::stringstream ss;
	ss << h;
	std::string hash = ss.str();
	return hash;
}

//For example to display the row
void
put_scanline_someplace(JSAMPLE* ba, int row_stride)
{
	static int height;
	int i;

	for (i = 0; i < row_stride; i++)
		printf("%d ", ba[i]);

	printf("width: %3d height: %3d\n", row_stride, height++);
}


//https://raw.githubusercontent.com/libjpeg-turbo/libjpeg-turbo/master/example.txt

/*
* Here's the routine that will replace the standard error_exit method:
*/

void ImageData::my_error_exit(j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_ptr myerr = (my_error_ptr)cinfo->err;

	/* Always display the message. */
	/* We could postpone this until after returning, if we chose. */
	(*cinfo->err->output_message) (cinfo);

	/* Return control to the setjmp point */
	longjmp(myerr->setjmp_buffer, 1);
}


/*
* Sample routine for JPEG decompression.  We assume that the source file name
* is passed in.  We want to return 1 on success, 0 on error.
*/

bool ImageData::read_JPEG_file(char *filename)
{
	/* This struct contains the JPEG decompression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	*/
	struct jpeg_decompress_struct cinfo;

	return do_read_JPEG_file(&cinfo, filename);
}

bool ImageData::do_read_JPEG_file(struct jpeg_decompress_struct *cinfo, char *filename)
{
	/* We use our private extension JPEG error handler.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct my_error_mgr jerr;
	/* More stuff */
	FILE *infile;                 /* source file */
	JSAMPARRAY buffer;
	/* In this example we want to open the input file before doing anything else,
	* so that the setjmp() error recovery below can assume the file is open.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to read binary files.
	*/

	if ((infile = fopen(filename, "rb")) == NULL) {
		fprintf(stderr, "can't open %s\n", filename);
		return false;
	}

	/* Step 1: allocate and initialize JPEG decompression object */

	/* We set up the normal JPEG error routines, then override error_exit. */
	cinfo->err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	/* Establish the setjmp return context for my_error_exit to use. */
	if (setjmp(jerr.setjmp_buffer)) {
		/* If we get here, the JPEG code has signaled an error.
		* We need to clean up the JPEG object, close the input file, and return.
		*/
		jpeg_destroy_decompress(cinfo);
		fclose(infile);
		return false;
	}
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(cinfo);

	/* Step 2: specify data source (eg, a file) */

	jpeg_stdio_src(cinfo, infile);

	/* Step 3: read file parameters with jpeg_read_header() */

	(void)jpeg_read_header(cinfo, TRUE);
	/* We can ignore the return value from jpeg_read_header since
	*   (a) suspension is not possible with the stdio data source, and
	*   (b) we passed TRUE to reject a tables-only JPEG file as an error.
	* See libjpeg.txt for more info.
	*/

	/* Step 4: set parameters for decompression */

	/* We want to scale down the image so divide by the scale factor
	* and remove the color information for comparison
	*/
	cinfo->scale_num = 1;
	cinfo->scale_denom = scale_coeff;
	cinfo->out_color_space = JCS_GRAYSCALE;

	/* Step 5: Start decompressor */

	(void)jpeg_start_decompress(cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* We may need to do some setup of our own at this point before reading
	* the data.  After jpeg_start_decompress() we have the correct scaled
	* output image dimensions available, as well as the output colormap
	* if we asked for color quantization.
	* In this example, we need to make an output work buffer of the right size.
	*/
	/* JSAMPLEs per row in output buffer */
	_width = cinfo->output_width;
	_height = cinfo->output_height;
	_row_stride = cinfo->output_width * cinfo->output_components;
	std::cout << "Width : " << _width << " and Height : " << _height << " and row stride : " << _row_stride << '\n';

	/* Make a one-row-high sample array that will go away when done with image */
	_buffer = (*cinfo->mem->alloc_sarray)((j_common_ptr)cinfo, JPOOL_IMAGE, _row_stride, 1);
	if (!(_buffer)) {
		printf("failed to allocate %dMi bytes for image.\n", _row_stride * _width * _height / 1048576);
		return false;
	}

	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
	/* Here we use the library's state variable cinfo->output_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	*/
	try {
		while (cinfo->output_scanline < cinfo->output_height) {
			/* jpeg_read_scanlines expects an array of pointers to scanlines.
			* Here the array is only one element long, but you could ask for
			* more than one scanline at a time if that's more convenient.
			*/
			(void)jpeg_read_scanlines(cinfo, (JSAMPARRAY)_buffer, 1);
			/* Assume put_scanline_someplace wants a pointer and sample count. */
			//put_scanline_someplace((JSAMPLE*)_buffer[0], _row_stride);

		}
	}
	catch (int e) {
		std::cerr << "Cannot read the JPEG file." << std::endl;
		std::cerr << "An exception occurred. Exception Nb " << e << '\n';
		return false;
	}

	/* Step 7: Finish decompression */

	(void)jpeg_finish_decompress(cinfo);
	/* We can ignore the return value since suspension is not possible
	* with the stdio data source.
	*/

	/* Step 8: Release JPEG decompression object */

	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(cinfo);

	/* After finish_decompress, we can close the input file.
	* Here we postpone it until after no more JPEG errors are possible,
	* so as to simplify the setjmp error logic above.  (Actually, I don't
	* think that jpeg_destroy can do an error exit, but why assume anything...)
	*/
	fclose(infile);

	/* At this point you may want to check to see whether any corrupt-data
	* warnings occurred (test whether jerr.pub.num_warnings is nonzero).
	*/

	/* And we're done! */
	return true;
}

