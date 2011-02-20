#include "stdafx.h"
#include "png_utils.h"
#include <png.h>

namespace png_utils
{

void PNGAPI user_error_fn(png_structp png, png_const_charp sz) { }
void PNGAPI user_warning_fn(png_structp png, png_const_charp sz) { }

typedef struct _png_read {
  std::vector<unsigned char>* buffer_ptr;
  size_t offset;
}png_read;

void png_read_func(png_structp png_ptr, png_bytep data, png_size_t length)
{
  png_read* pr = (png_read*)png_get_io_ptr(png_ptr);
  memcpy(data, &(*(pr->buffer_ptr))[pr->offset], length);
  pr->offset += length;
}

HBITMAP LoadBuffer(std::vector<unsigned char>& input_png)
{
  HBITMAP hbm = NULL;

  if (input_png.size() <= 8)
    return hbm;

  bool retVal = false;
  int size = input_png.size();

  if (png_sig_cmp(&input_png[0], 0, 8))
    return NULL;

  // now allocate stuff
  png_structp png_ptr =
    png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, user_error_fn, user_warning_fn);
  if (!png_ptr)
    return NULL;
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr,
    (png_infopp)NULL, (png_infopp)NULL);
    return NULL;
  }
   
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr,
    (png_infopp)NULL);
    return NULL;
  }
   
  // png_init_io(png_ptr, fp);
  png_read pr = {&input_png, 0};
  png_set_read_fn(png_ptr, (void*)&pr, png_read_func);
   
  // should really use png_set_rows() to allocate space first, rather than doubling up
   
  png_read_png(png_ptr, info_ptr, 
    PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_BGR, 
    NULL);
   
  png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);//new png_bytep[info_ptr->height];
   
  // now for a tonne of ugly DIB setup crap
   
  int width = info_ptr->width;
  int height = info_ptr->height;
  int bpp = info_ptr->channels * 8;
  int memWidth = (width * (bpp >> 3) + 3) & ~3;
   
  LPBITMAPINFO lpbi = (LPBITMAPINFO) new char[sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD))];
   
  // create a greyscale palette
  for (int a_i = 0; a_i < 256; a_i++)
  {
    lpbi->bmiColors[a_i].rgbRed = (BYTE)a_i;
    lpbi->bmiColors[a_i].rgbGreen = (BYTE)a_i;
    lpbi->bmiColors[a_i].rgbBlue = (BYTE)a_i;
    lpbi->bmiColors[a_i].rgbReserved = 0;
  }
   
  lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  lpbi->bmiHeader.biWidth = width;
  lpbi->bmiHeader.biHeight = -height; // must be negative for top down
  lpbi->bmiHeader.biPlanes = 1;
  lpbi->bmiHeader.biBitCount = bpp;
  lpbi->bmiHeader.biCompression = BI_RGB;
  lpbi->bmiHeader.biSizeImage = memWidth * height;
  lpbi->bmiHeader.biXPelsPerMeter = 0;
  lpbi->bmiHeader.biYPelsPerMeter = 0;
  lpbi->bmiHeader.biClrUsed = 0;
  lpbi->bmiHeader.biClrImportant = 0;
   
  BYTE * pixelData;
  hbm = CreateDIBSection(NULL, lpbi, DIB_RGB_COLORS, (void **)&pixelData, NULL, 0 );
  if (hbm && pixelData)
  {
    // now copy the rows
    for (int i = 0; i < height; i++)
      memcpy(pixelData + memWidth * i, row_pointers[i], width * info_ptr->channels);
  }

  // premultiply bitmap data for fast displaying
  if (bpp == 32)
  {
    for (int y = 0; y < height; y++)
    {
      BYTE *pPixel = (BYTE *) pixelData + width * 4 * y;
      for (int x = 0; x < width; x++)
      {
        pPixel[0] = pPixel[0] * pPixel[3] / 255; 
        pPixel[1] = pPixel[1] * pPixel[3] / 255; 
        pPixel[2] = pPixel[2] * pPixel[3] / 255; 
        pPixel += 4;
      }
    }
  }
   
  delete (char*) lpbi;
   
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);

  return hbm;
}

HBITMAP LoadFile(const wchar_t* filename)
{
  HBITMAP hbm = NULL;

  bool retVal = false;
  int size = 0;
  // check the header first
  FILE* fp = NULL;
  if (_wfopen_s(&fp, filename, L"rb") != 0)
    return NULL;

  BYTE header[8];
  fread(header, 1, 8, fp);
  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  fclose(fp);

  if (png_sig_cmp(header, 0, 8))
    return NULL;

  // now allocate stuff
  png_structp png_ptr =
  png_create_read_struct(PNG_LIBPNG_VER_STRING,
  NULL, user_error_fn, user_warning_fn);
  if (!png_ptr)
    return NULL;
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr,
    (png_infopp)NULL, (png_infopp)NULL);
    return NULL;
  }
   
  png_infop end_info = png_create_info_struct(png_ptr);
  if (!end_info)
  {
    png_destroy_read_struct(&png_ptr, &info_ptr,
    (png_infopp)NULL);
    return NULL;
  }
   
  if (_wfopen_s(&fp, filename, L"rb") != 0)
    return NULL;

  png_init_io(png_ptr, fp);
   
  // should really use png_set_rows() to allocate space first, rather than doubling up
   
  png_read_png(png_ptr, info_ptr, 
    PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_BGR, 
    NULL);
   
  fclose(fp);
   
  png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);//new png_bytep[info_ptr->height];
   
  // now for a tonne of ugly DIB setup crap
   
  int width = info_ptr->width;
  int height = info_ptr->height;
  int bpp = info_ptr->channels * 8;
  int memWidth = (width * (bpp >> 3) + 3) & ~3;
   
  LPBITMAPINFO lpbi = (LPBITMAPINFO) new char[sizeof(BITMAPINFOHEADER) + (256 * sizeof(RGBQUAD))];
   
  // create a greyscale palette
  for (int a_i = 0; a_i < 256; a_i++)
  {
    lpbi->bmiColors[a_i].rgbRed = (BYTE)a_i;
    lpbi->bmiColors[a_i].rgbGreen = (BYTE)a_i;
    lpbi->bmiColors[a_i].rgbBlue = (BYTE)a_i;
    lpbi->bmiColors[a_i].rgbReserved = 0;
  }
   
  lpbi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  lpbi->bmiHeader.biWidth = width;
  lpbi->bmiHeader.biHeight = -height; // must be negative for top down
  lpbi->bmiHeader.biPlanes = 1;
  lpbi->bmiHeader.biBitCount = bpp;
  lpbi->bmiHeader.biCompression = BI_RGB;
  lpbi->bmiHeader.biSizeImage = memWidth * height;
  lpbi->bmiHeader.biXPelsPerMeter = 0;
  lpbi->bmiHeader.biYPelsPerMeter = 0;
  lpbi->bmiHeader.biClrUsed = 0;
  lpbi->bmiHeader.biClrImportant = 0;
   
  BYTE * pixelData;
  hbm = CreateDIBSection(NULL, lpbi, DIB_RGB_COLORS, (void **)&pixelData, NULL, 0 );
  if (hbm && pixelData)
  {
    // now copy the rows
    for (int i = 0; i < height; i++)
      memcpy(pixelData + memWidth * i, row_pointers[i], width * info_ptr->channels);
  }
   
  delete (char*) lpbi;
   
  png_destroy_read_struct(&png_ptr, &info_ptr, &end_info);
   
  return hbm;
}

} // namespace png_utils