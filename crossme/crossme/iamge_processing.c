#include "globals.h"
#include "image_processing.h"


#define BOX_SIZE_X (16)
#define BOX_SIZE_Y (16)

#define M_FRAME_XY_TO_INDEX(_height_, _xoffset_, _yoffset_) (((_yoffset_) * (_height_) + (_xoffset_)) * 3)



static int check_frame_box(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset);
static void mark_frame_gray(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset);

__declspec(dllexport) int modify_frame(void* frame_p, int width, int height)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int should_keep_box;

   for (int i = 0; i < width; i += BOX_SIZE_X)
   {
      for (int j = 0; j < height; j += BOX_SIZE_Y)
      {
         should_keep_box = check_frame_box(frame_p, width, height, i, j);

         if (!should_keep_box)
            mark_frame_gray(frame_p, width, height, i, j);
      }
   }

   return 0;
}

static int check_frame_box(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   int is_cube_frame = FALSE;

   int total_r = 0;
   int total_g = 0;
   int total_b = 0;
   int diff_from_avg = 0;

   for (int x = 0; x < BOX_SIZE_X; ++x)
   {
      for (int y = 0; y < BOX_SIZE_Y; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + x, yoffset + y);
         total_r += frame_data[frame_index + 0];
         total_g += frame_data[frame_index + 1];
         total_b += frame_data[frame_index + 2];
      }
   }

   total_r /= (BOX_SIZE_X * BOX_SIZE_Y);
   total_g /= (BOX_SIZE_X * BOX_SIZE_Y);
   total_b /= (BOX_SIZE_X * BOX_SIZE_Y);

   for (int x = 0; x < BOX_SIZE_X; ++x)
   {
      for (int y = 0; y < BOX_SIZE_Y; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + x, yoffset + y);
         
         diff_from_avg += M_DIFF_ABS(frame_data[frame_index + 0], total_r) * M_DIFF_ABS(frame_data[frame_index + 0], total_r);
         diff_from_avg += M_DIFF_ABS(frame_data[frame_index + 1], total_g) * M_DIFF_ABS(frame_data[frame_index + 1], total_g);
         diff_from_avg += M_DIFF_ABS(frame_data[frame_index + 2], total_b) * M_DIFF_ABS(frame_data[frame_index + 2], total_b);

         if (x == 0 || y == 0)
         {
            frame_data[frame_index + 0] = 255;
            frame_data[frame_index + 1] = 0;
            frame_data[frame_index + 2] = 0;
         }
      }
   }

   //diff_from_avg /= (BOX_SIZE_X * BOX_SIZE_Y);

   return diff_from_avg < 1000*100/4;
}

static void mark_frame_gray(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;

   for (int x = 1; x < BOX_SIZE_X; ++x)
   {
      for (int y = 1; y < BOX_SIZE_Y; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + x, yoffset + y);
         if (frame_index >= frame_width * frame_height * 3)
         {
            frame_index = frame_index;
         }
         frame_data[frame_index + 0] = 191;
         frame_data[frame_index + 1] = 191;
         frame_data[frame_index + 2] = 191;
      }
   }
}