#include "lima/Exceptions.h"

#include "lima/VideoUtils.h"
using namespace lima;

/** func tool to convert from color to Y only (luma) for greyscale image
 */
inline void _rgb555_2_yuv(const unsigned char *data,unsigned char *luma,
			   int column,int row)
{
  for(int aSize = column * row;aSize;--aSize,data += 2,++luma)
    {
      unsigned char red = (data[0] & 0x7c) >> 2;
      unsigned char green = ((data[0] & 0x03) << 3)  + ((data[1] & 0xe0) >> 5);
      unsigned char blue = data[1] & 0x1f;
      *luma = ((66 * red + 129 * green + 25 * blue) + 128) >> 8;
    }
}

inline void _rgb565_2_yuv(const unsigned char *data,unsigned char *luma,
			   int column,int row)
{
  for(int aSize = column * row;aSize;--aSize,data += 2,++luma)
    {
      unsigned char red = (data[0] & 0xf8) >> 3;
      unsigned char green = ((data[0] & 0x07) << 3)  + ((data[1] & 0xe0) >> 5);
      unsigned char blue = data[1] & 0x1f;
      *luma = ((66 * red + 129 * green + 25 * blue) + 128) >> 8;
    }
}

inline void _rgb_2_yuv(const unsigned char *data,unsigned char *luma,
			int column,int row,int bandes)
{
  for(int aSize = column * row;aSize;--aSize,data += bandes,++luma)
    *luma = ((66 * data[0] + 129 * data[1] + 25 * data[2]) + 128) >> 8;
}

inline void _bgr_2_yuv(const unsigned char *data,unsigned char *luma,
			int column,int row,int bandes)
{
  for(int aSize = column * row;aSize;--aSize,data += bandes,++luma)
    *luma = ((25 * data[0] + 129 * data[1] + 66 * data[2]) + 128) >> 8;
}

template<class xClass>
inline void _bayer_2_yuv(const xClass* bayer0,xClass* luma,
			 int column,int row,int blue,int start_with_green)
{
  int luma_step = column * sizeof(xClass);
  int bayer_step = column;
  xClass *luma0 = (xClass*)luma;
  memset( luma0, 0, luma_step);
  memset( luma0 + (row - 1)*bayer_step, 0, luma_step);
  luma0 += column + 1;
  row -= 2;
  column -= 2;

  for( ; row > 0;--row,bayer0 += bayer_step, luma0 += bayer_step )
    {
      int t0, t1;
      const xClass* bayer = bayer0;
      xClass* dst = luma0;
      const xClass* bayer_end = bayer + column;

      dst[-1] = 0;

      if(column <= 0 )
	continue;

      if( start_with_green )
        {
	  t0 = (bayer[1] + bayer[bayer_step*2+1] + 1) >> 1;
	  t1 = (bayer[bayer_step] + bayer[bayer_step+2] + 1) >> 1;
	  if(blue < -1)
	    *dst = (bayer[bayer_step+1] * 150 + t0 * 29 + t1 * 76) >> 8;
	  else
	    *dst = (bayer[bayer_step+1] * 150 + t1 * 29 + t0 * 76) >> 8;
	  ++bayer;
	  ++dst;
        }

      if( blue > 0 )
        {
	  for( ; bayer <= bayer_end - 2; bayer += 2)
            {
	      t0 = (bayer[0] + bayer[2] + bayer[bayer_step*2] +
		    bayer[bayer_step*2+2] + 2) >> 2;
	      t1 = (bayer[1] + bayer[bayer_step] +
		    bayer[bayer_step+2] + bayer[bayer_step*2+1]+2) >> 2;
	      *dst = (t0 * 76 + t1 * 150 + bayer[bayer_step+1] * 29) >> 8;
	      ++dst;

	      t0 = (bayer[2] + bayer[bayer_step*2+2] + 1) >> 1;
	      t1 = (bayer[bayer_step+1] + bayer[bayer_step+3] + 1) >> 1;
	      *dst = (t0 * 76 + bayer[bayer_step+2] * 150 + t1 * 29) >> 8;
	      ++dst;
            }
        }
      else
        {
	  for( ; bayer <= bayer_end - 2; bayer += 2)
            {
	      t0 = (bayer[0] + bayer[2] + bayer[bayer_step*2] +
		    bayer[bayer_step*2+2] + 2) >> 2;
	      t1 = (bayer[1] + bayer[bayer_step] +
		    bayer[bayer_step+2] + bayer[bayer_step*2+1]+2) >> 2;
	      *dst = (t0 * 29 + t1 * 150 + bayer[bayer_step+1] * 76) >> 8;
	      ++dst;

	      t0 = (bayer[2] + bayer[bayer_step*2+2] + 1) >> 1;
	      t1 = (bayer[bayer_step+1] + bayer[bayer_step+3] + 1) >> 1;
	      *dst = (t0 * 29 + bayer[bayer_step+2] * 150 + t1 * 76) >> 8;
	      ++dst;
            }
        }

      if( bayer < bayer_end )
        {
	  t0 = (bayer[0] + bayer[2] + bayer[bayer_step*2] +
		bayer[bayer_step*2+2] + 2) >> 2;
	  t1 = (bayer[1] + bayer[bayer_step] +
		bayer[bayer_step+2] + bayer[bayer_step*2+1]+2) >> 2;
	  if(blue > 0)
	    *dst = (t0 * 76 + t1 * 150 +  bayer[bayer_step+1] * 29) >> 8;
	  else
	    *dst = (t0 * 29 + t1 * 150 +  bayer[bayer_step+1] * 76) >> 8;
	  ++bayer;
	  ++dst;
        }

      blue = -blue;
      start_with_green = !start_with_green;
    }
}

template<class xClass>
inline void _bayer_rg_2_yuv(const xClass* bayer0,xClass* luma,
			    int column,int row)
{
  _bayer_2_yuv<xClass>(bayer0,luma,column,row,1,0);
}

template<class xClass>
inline void _bayer_bg_2_yuv(const xClass* bayer0,xClass* luma,
			    int column,int row)
{
  _bayer_2_yuv<xClass>(bayer0,luma,column,row,-1,0);
}

inline void _yuv422packed_2_yuv(const unsigned char *data,unsigned char *luma,
			int column,int row)
{
  // format 4 bytes for 2 pixels U and V common for 2pixels: |U0|Y0|V0|Y1|  |U2|Y2|V2|Y3|
  // so Y (luma) is every 2 bytes
  long nbIter = column * row /2;
  --nbIter;
  for(const unsigned char *src = data+1; nbIter; --nbIter, src += 4, luma+=2)
    {
      luma[0] = src[0];
      luma[1] = src[2];
    }
}

void lima::data2Image(Data &aData,VideoImage &anImage)
{
  if(!aData.empty())
    {
      switch(aData.type)
	{
	case Data::UINT8:
	case Data::INT8:
	  anImage.mode = Y8;break;
	case Data::UINT16:
	case Data::INT16:
	  anImage.mode = Y16;break;
	case Data::UINT32:
	case Data::INT32:
	  anImage.mode = Y32;break;
	case Data::UINT64:
	case Data::INT64:
	  anImage.mode = Y64;break;
	case Data::FLOAT:
	case Data::DOUBLE:
	default:
	  throw LIMA_COM_EXC(Error, "Data type is not yet used for VideoImage");
	}
      anImage.alloc(aData.size());
      memcpy(anImage.buffer,aData.data(),aData.size());
      anImage.width = aData.dimensions[0];
      anImage.height = aData.dimensions[1];
      anImage.frameNumber = aData.frameNumber;
    }
}

/*
 * convert the video color image to Y (luma only) greyscale image
 */
void lima::image2YUV(const unsigned char *srcPt,int width,int height,VideoMode mode,
		     unsigned char *dst)
{

  switch(mode)
    {
    case Y8:
    case Y16:
    case Y32:
    case Y64:
      {
	int size = int((width * height * VideoImage::mode_depth(mode)) + .5);
	memcpy(dst,srcPt,size);
	break;
      }
    case I420:
    case YUV411:
    case YUV422:
    case YUV444:
      memcpy(dst,srcPt,width * height);
      break;
    case YUV422PACKED:
      _yuv422packed_2_yuv(srcPt,dst,width,height);
    case RGB555:
      _rgb555_2_yuv(srcPt,dst,width,height);
      break;
    case RGB565:
      _rgb565_2_yuv(srcPt,dst,width,height);
      break;
    case BAYER_RG8:
      _bayer_rg_2_yuv(srcPt,dst,width,height);
      break;
    case BAYER_RG16:
      _bayer_rg_2_yuv((unsigned short*)srcPt,(unsigned short*)dst,width,height);
      break;
    case BAYER_BG16:
      _bayer_bg_2_yuv((unsigned short*)srcPt,(unsigned short*)dst,width,height);
      break;
    case RGB32:
      _rgb_2_yuv(srcPt,dst,width,height,4);
      break;
    case BGR32:
      _bgr_2_yuv(srcPt,dst,width,height,4);
      break;
    case RGB24:
      _rgb_2_yuv(srcPt,dst,width,height,3);
      break;
    case BGR24:
      _bgr_2_yuv(srcPt,dst,width,height,3);
      break;
    default:
      throw LIMA_COM_EXC(Error,"Video mode not yet managed!");
    }
}

void _bayer_bg_2_rgb(const unsigned char * srcPt, unsigned char *dst, int width,int height)
{
	// convert to RGB
	unsigned char * rgb24_buffer = NULL;
	convertBayerToRgb24(srcPt, width, height, rgb24_buffer);

	if(rgb24_buffer)
	{
		const unsigned char * source = reinterpret_cast<const unsigned char *>(rgb24_buffer);
		unsigned char * dest = dst;

		int nb_pixels_to_treat = height * width;

		while(nb_pixels_to_treat--)
		{
			*dest++ = 0; // alpha
			*dest++ = *(source + 3); // B
			*dest++ = *(source + 2); // G
			*dest++ = *(source + 1); // R
			source += 4;
		}
		
		delete [] rgb24_buffer;
	}
}

/*
 * convert the video color image to RGB image
 */
bool lima::image2RGB(const unsigned char *srcPt,int width,int height,VideoMode mode, unsigned char *dst)
{
	bool result = false;

    switch(mode)
    {
    case BAYER_BG8:
      _bayer_bg_2_rgb(srcPt,dst,width,height);
      result = true;
      break;
    }

    return result;
}

#define BAYER_ENABLE_R_PIXEL
#define BAYER_ENABLE_G_PIXEL
#define BAYER_ENABLE_B_PIXEL

#define BAYER_ENABLE_RB_PROCESS
#define BAYER_ENABLE_MISSING_PIXELS_PROCESS
#define BAYER_ENABLE_BLUR_PROCESS

///////////////////////////////
void lima::copyBayerPixelsToFilterBuffer(const unsigned char * in_bayer_buffer, const int in_width, const int in_height, const int in_filter_scanline, unsigned char * in_filter_buffer)
{
    const unsigned char * source = in_bayer_buffer; // jump the top border and the left border to find the real position of the pixel(0,0)
    unsigned char *       dest   = in_filter_buffer + in_filter_scanline + 3; // jump the top border and the left border to find the real position of the pixel(0,0)

    // compute the number of BG or GR groups in the line
    int  column_groups_nb     = (in_width >> 1);
    bool line_has_last_column = ((in_width %  2) == 1);

    // copy the pixels groups 
    int x;
    int y = in_height;

    for(;;)
    {       
        // new BG line
        x = column_groups_nb;

        do
        {
            // B 
        #ifdef BAYER_ENABLE_B_PIXEL
            *dest = *source++;
        #else
            source++;
        #endif
            dest += 4; // move to the good RGB position, +3 to next start of RGB group and +1 for G position

            // G 
        #ifdef BAYER_ENABLE_G_PIXEL
            *dest = *source++;
        #else
            source++;
        #endif
            dest += 2; // move to the good RGB position, +3 to next start of RGB group and +1 for G position
        }
        while(--x);

        // last pixel ?
        if(line_has_last_column)
        {
            // B 
        #ifdef BAYER_ENABLE_B_PIXEL
            *dest = *source++;
        #else
            source++;
        #endif
            dest += 3; // move to the good RGB position
        }

        // last line ?
        if(--y == 0)
            break;

        // preparing the next line treatment
        dest += 6; // jump the right border of the current line and the left border of the next line to get the initial position in the next line

        // new GR line
        x = column_groups_nb;

        do
        {
            // G 
        #ifdef BAYER_ENABLE_G_PIXEL
            *(dest+1) = *source++;
        #else
            source++;
        #endif
            dest += 5; // move to the good RGB position, +3 to next start of RGB group and +2 for R position

            // R 
        #ifdef BAYER_ENABLE_R_PIXEL
            *dest = *source++;
        #else
            source++;
        #endif
            dest += 1; // move to the good RGB position, +1 to next start of RGB group
        }
        while(--x);

        // last pixel ?
        if(line_has_last_column)
        {
            // G 
        #ifdef BAYER_ENABLE_G_PIXEL
            *(dest+1) = *source++;
        #else
            source++;
        #endif
            dest += 3; // move to the good RGB position, +3 to next start of RGB group
        }

        // last line ?
        if(--y == 0)
            break;

        // preparing the next line treatment
        dest += 6; // jump the right border of the current line and the left border of the next line to get the initial position in the next line
    }
}

///////////////////////////////
// compute the B component of RGB(R) pixels 
// compute the R component of RGB(B) pixels 

void lima::computeRBPixelsInFilterBuffer(unsigned char * in_out_filter_buffer, const int in_width, const int in_height, const int in_filter_scanline)
{
    unsigned char * dest = in_out_filter_buffer + in_filter_scanline + 3; // jump the top border and the left border to find the real position of the pixel(0,0)
    unsigned char * previous_line_start;

    // compute the number of B or R in a line
    int line_nb_B_component =  (in_width >> 1) + (in_width %  2);
    int line_nb_R_component =  (in_width >> 1);

    // compute the offsets to get the corner pixels
    int up_left_corner_offset    = -in_filter_scanline - 3; 
    int up_right_corner_offset   = -in_filter_scanline + 3; 

    // copy the pixels groups 
    int x;
    int y = in_height;

    previous_line_start = dest;

    for(;;)
    {       
        // new BG line, B pixel, so compute the R component
        x = line_nb_B_component;

        // shift to R component position
        dest += 2;

        do
        {
            *dest = (*(dest + up_left_corner_offset) + *(dest - up_left_corner_offset) + *(dest + up_right_corner_offset) + *(dest - up_right_corner_offset)) >> 2; // div by 4
            dest += 6; // +3 to jump to start of RGB(G) and +3 for next start of RGB(B)
        }
        while(--x);

        // last line ?
        if(--y == 0)
            break;

        // preparing the next line treatment
        previous_line_start += in_filter_scanline;

        dest = previous_line_start + 3; // jumping the first RGB(G) to get the RGB(R) position

        // new GR line, R pixel, so compute the B component
        x = line_nb_R_component;

        do
        {
            *dest = (*(dest + up_left_corner_offset) + *(dest - up_left_corner_offset) + *(dest + up_right_corner_offset) + *(dest - up_right_corner_offset)) >> 2; // div by 4
            dest += 6; // +3 to jump to start of RGB(G) and +3 for next start of RGB(R)
        }
        while(--x);

        // last line ?
        if(--y == 0)
            break;

        // preparing the next line treatment
        previous_line_start += in_filter_scanline;
        dest = previous_line_start; // first RGB(B) position
    }
}

///////////////////////////////

// compute the R and B components of RGB(G) pixels 
// compute the G component of RGB(B) and RGB(R) pixels 

void lima::computeMissingPixelsInFilterBuffer(unsigned char * in_out_filter_buffer, const int in_width, const int in_height, const int in_filter_scanline)
{
    unsigned char * dest = in_out_filter_buffer + in_filter_scanline + 3; // jump the top border and the left border to find the real position of the pixel(0,0)
    
    // compute the number of BG or GR groups in the line
    int  column_groups_nb     = (in_width >> 1);
    bool line_has_last_column = ((in_width %  2) == 1);

    // compute the offsets to get the corner pixels
    int up_offset    = -in_filter_scanline; 
    int left_offset  = -3          ; 

    // copy the pixels groups 
    int x;
    int y = in_height;

    for(;;)
    {       
        // new BG line
        x = column_groups_nb;

        do
        {
            // B -> compute the G value
            dest++; // move to the G position
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest += 2; // move to the next RGB position

            // G -> compute the R and B values
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest+= 2;
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest++; // move to the next RGB position
        }
        while(--x);

        // last pixel ?
        if(line_has_last_column)
        {
            // B -> compute the G value
            dest++; // move to the G position
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest += 2; // move to the next RGB position
        }

        // last line ?
        if(--y == 0)
            break;

        // preparing the next line treatment
        dest += 6; // jump the right border of the current line and the left border of the next line to get the initial position in the next line

        // new GR line
        x = column_groups_nb;

        do
        {
            // G -> compute the R and B values
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest+= 2;
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest++; // move to the next RGB position

            // R -> compute the G value
            dest++; // move to the G position
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest += 2; // move to the next RGB position
        }
        while(--x);

        // last pixel ?
        if(line_has_last_column)
        {
            // G -> compute the R and B values
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest+= 2;
            *dest = (*(dest + up_offset) + *(dest - up_offset) + *(dest + left_offset) + *(dest - left_offset)) >> 2; // div by 4
            dest++; // move to the next RGB position
        }

        // last line ?
        if(--y == 0)
            break;

        // preparing the next line treatment
        dest += 6; // jump the right border of the current line and the left border of the next line to get the initial position in the next line
    }
}

///////////////////////////////
void lima::copyFilterBufferToRgb24(const unsigned char * in_filter_buffer, const int in_width, const int in_height, const int in_filter_scanline, unsigned char * & out_rgb_buffer)
{
    const unsigned char * source  = in_filter_buffer + in_filter_scanline + 3; // jump the top border and the left border to find the real position of the pixel(0,0)
    unsigned char * dest          = out_rgb_buffer;
    const int       dest_scanline = in_width * 3;

    int y = in_height;

    do
    {       
        memcpy(dest, source, dest_scanline);
        dest   += dest_scanline     ;
        source += in_filter_scanline;
    }
    while(--y);
}

///////////////////////////////
void lima::computeBlurInFilterBuffer(unsigned char * & in_out_filter_buffer, const int in_width, const int in_height, unsigned char * out_rgb_buffer)
{
    const int filter_width    = in_width  + 2; // add left and right borders to avoid to process special cases
    const int filter_height   = in_height + 2; // add top and bottom borders to avoid to process special cases
    const int filter_scanline = filter_width * 3; // line size
    const int dest_scanline   = in_width * 3;

    unsigned char *       dest   = out_rgb_buffer; 
    const unsigned char * source = in_out_filter_buffer + filter_scanline + 3; // jump the top border and the left border to find the real position of the pixel(0,0)

    // compute the number of BG or GR groups in the line
    int  column_groups_nb     = (in_width >> 1);
    bool line_has_last_column = ((in_width %  2) == 1);

    // compute the offsets to get the corner pixels
    int up_left_corner_offset  = -filter_scanline - 3; 
    int up_right_corner_offset = -filter_scanline + 3; 
    int up_offset              = -filter_scanline; 
    int left_offset            = -3; 

    // copy the pixels groups 
    int x;
    int y = in_height;

    #define COMPUTE_BLUR(d,s) *d = (((*s) << 2) + (*(s + left_offset) + *(s - left_offset) + *(s + up_offset) + *(s - up_offset))) >> 3

    for(;;)
    {       
        // new BG line
        x = column_groups_nb;

        do
        {
            // B -> compute the B value
            COMPUTE_BLUR(dest, source);

            dest   += 4; // move to the G position
            source += 4; // move to the G position

            // G -> compute the G value
            COMPUTE_BLUR(dest, source);

            dest   += 2; // move to the next RGB position
            source += 2; // move to the next RGB position
        }
        while(--x);

        // last pixel ?
        if(line_has_last_column)
        {
            // B -> compute the B value
            COMPUTE_BLUR(dest, source);

            dest   += 3; // move to the next RGB position
            source += 3; // move to the next RGB position
        }

        // last line ?
        if(--y == 0)
            break;

        // preparing the next line treatment
        source += 6; // jump the right border of the current line and the left border of the next line to get the initial position in the next line

        // new GR line
        x = column_groups_nb;

        do
        {
            dest   ++; // move to the G position
            source ++; // move to the G position

            // G -> compute the G value
            COMPUTE_BLUR(dest, source);

            dest   += 4; // move to the R position
            source += 4; // move to the R position

            COMPUTE_BLUR(dest, source);

            dest   ++; // move to the next RGB position
            source ++; // move to the next RGB position
        }
        while(--x);

        // last pixel ?
        if(line_has_last_column)
        {
            dest   ++; // move to the G position
            source ++; // move to the G position

            // G -> compute the G value
            COMPUTE_BLUR(dest, source);

            dest   += 2; // move to the next RGB position
            source += 2; // move to the next RGB position
        }

        // last line ?
        if(--y == 0)
            break;

        // preparing the next line treatment
        source += 6; // jump the right border of the current line and the left border of the next line to get the initial position in the next line
    }
}

///////////////////////////////
void lima::convertBayerToRgb24(const unsigned char * in_bayer_buffer, const int in_width, const int in_height, unsigned char * & out_rgb_buffer)
{
    if((in_width >= 2) && (in_height > 0))
    {
        // creating a temp buffer to process bilinear filtering
        const int filter_width    = in_width     + 2; // add left and right borders to avoid to process special cases
        const int filter_height   = in_height    + 2; // add top and bottom borders to avoid to process special cases
        const int filter_scanline = filter_width * 3; // line size
	
        unsigned char * filter_buffer = new unsigned char[filter_scanline * filter_height];
        memset(filter_buffer, 0, filter_scanline * filter_height);

        copyBayerPixelsToFilterBuffer(in_bayer_buffer, in_width, in_height, filter_scanline, filter_buffer);

    #ifdef BAYER_ENABLE_RB_PROCESS
        computeRBPixelsInFilterBuffer(filter_buffer  , in_width, in_height, filter_scanline);
    #endif

    #ifdef BAYER_ENABLE_MISSING_PIXELS_PROCESS
        computeMissingPixelsInFilterBuffer(filter_buffer  , in_width, in_height, filter_scanline);
    #endif

        // creating the final image in RGB color format
        out_rgb_buffer = new unsigned char[in_width * in_height * 3];

        copyFilterBufferToRgb24(filter_buffer  , in_width, in_height, filter_scanline, out_rgb_buffer);

    #ifdef BAYER_ENABLE_BLUR_PROCESS
        computeBlurInFilterBuffer(filter_buffer  , in_width, in_height, out_rgb_buffer);
    #endif

        delete [] filter_buffer;
    }
}

