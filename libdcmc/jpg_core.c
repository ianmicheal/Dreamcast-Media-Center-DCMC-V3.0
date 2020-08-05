/* 
** 
** JPG-Core - A JPG Texture Loader
** (C) Josh "PH3NOM" Pearson 2011 
**
*/
#include <assert.h>
#include <string.h>

#include "jpg_core.h"
 
/* Load a JPEG into a TexStruct */
int jpeg_to_image( FILE * infile, TexStruct * texture)
{
	int		i;
	uint16		* ourbuffer;
	uint16		* temp_tex;
    int scale = 1;
    
	/* This struct contains the JPEG decompression parameters and pointers to
	 * working space (which is allocated as needed by the JPEG library).
	 */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
    
	/* More stuff */
	JSAMPARRAY	buffer;			/* Output row buffer */
	int		row_stride;		/* physical row width in output buffer */
    
	/* Step 1: allocate and initialize JPEG decompression object */
	/* We set up the normal JPEG error routines */
	cinfo.err = jpeg_std_error(&jerr);
    
	/* Now we can initialize the JPEG decompression object. */
	jpeg_create_decompress(&cinfo);
    
	/* Step 2: specify data source (eg, a file) */
	jpeg_stdio_src(&cinfo, (FILE*)infile);
    
	/* Step 3: read file parameters with jpeg_read_header() */
	(void)jpeg_read_header(&cinfo, TRUE);

	/* Step 4: Set Up Output 
	cinfo.out_color_space = JCS_YCbCr;
	cinfo.quantize_colors = FALSE;
	cinfo.dct_method = JDCT_FASTEST;
    */
    
	jpeg_calc_output_dimensions(&cinfo);

	/* Allocate the output buffers */
	texture->w = cinfo.image_width;
	texture->h = cinfo.image_height;
	
	//rv->data = (void *)( temp_tex = (uint16 *)malloc(rv->w * rv->h * 2) );
	texture->data = (void *)( temp_tex = (uint16 *)memalign(32, texture->w * texture->h * 2) );	
	ourbuffer = (uint16 *)malloc(texture->w * 2);
    
	/* Step 4: set parameters for decompression */
	cinfo.scale_denom = scale; /* must be 1, 2, 4, or 8 */

	/* Step 5: Start decompressor */
	(void)jpeg_start_decompress(&cinfo);
    
	/* JSAMPLEs per row in output buffer */
	row_stride = cinfo.output_width * cinfo.output_components;

	/* Make a one-row-high sample array that will go away when done with image */
	buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    
	/* Step 6: while (scan lines remain to be read) */
	/*           jpeg_read_scanlines(...); */
    
	/* Here we use the library's state variable cinfo.output_scanline as the
	 * loop counter, so that we don't have to keep track ourselves.
	 */
	while (cinfo.output_scanline < cinfo.output_height) {
		/* jpeg_read_scanlines expects an array of pointers to scanlines.
	 	* Here the array is only one element long, but you could ask for
	 	* more than one scanline at a time if that's more convenient.
	 	*/
		(void)jpeg_read_scanlines(&cinfo, buffer, 1);

		for(i=0; i<row_stride/3; i++)
			ourbuffer[i] = ((buffer[0][i*3]>>3)<<11) + ((buffer[0][i*3+1]>>2)<<5) + (buffer[0][i*3+2]>>3);

		memcpy(temp_tex + cinfo.image_width*(cinfo.output_scanline-1), ourbuffer, 2*row_stride/3); 
	}

	/* Step 7: Finish decompression */
	(void)jpeg_finish_decompress(&cinfo);
	/* We can ignore the return value since suspension is not possible
	 * with the stdio data source.
	 */
    
	/* Step 8: Release JPEG decompression object */
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_decompress(&cinfo);

	free(ourbuffer);

	/* And we're done! */
	return 0;
}
