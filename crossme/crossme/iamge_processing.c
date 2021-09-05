#include <string.h>
#include<malloc.h>
#include <stdio.h>
#include "globals.h"
#include "image_processing.h"

#define NUM_KS (10)
#define BOX_SIZE_X (16/4)
#define BOX_SIZE_Y (16/4)

#define M_FRAME_XY_TO_INDEX(_height_, _xoffset_, _yoffset_) (((_yoffset_) * (_height_) + (_xoffset_)) * 3)

typedef struct
{
   unsigned char rgb[3];
   unsigned char rfu0;
   long long total_rgb[3];
   int num_pixels;
} KMeansCenter_t;


static int check_frame_box(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset);
static void mark_frame_gray(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset);

static void median_filter(void* frame_p, int frame_width, int frame_height);
static unsigned char get_median(void* frame_p, int frame_width, int frame_height, int x, int y, int rgb);

static void kmeans(void* frame_p, int frame_width, int frame_height);
static int kmeans_calc_distances(void* frame_p, int frame_width, int frame_height, int x, int y, KMeansCenter_t* kcenters_p, int* distances_p);

static void insertion_sort(unsigned char arr[], int n);

__declspec(dllexport) int modify_frame(void* frame_p, int width, int height)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int should_keep_box;
   //int distances[640][480][NUM_KS];

   //median_filter(frame_p, width, height);
   kmeans(frame_p, width, height);
   /*
   for (int i = 0; i < width; i += BOX_SIZE_X)
   {
      for (int j = 0; j < height; j += BOX_SIZE_Y)
      {
         should_keep_box = check_frame_box(frame_p, width, height, i, j);

         if (!should_keep_box)
            mark_frame_gray(frame_p, width, height, i, j);
      }
   }*/

   return 0;
}

int* distances_p = NULL;

static void kmeans(void* frame_p, int frame_width, int frame_height)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   KMeansCenter_t k_positions[NUM_KS];
   int centers_diff = 500;
   unsigned char labels[640][480] = { 0 };

   if (distances_p == NULL)
   {
      distances_p = (int*)malloc(sizeof(int)* 640 * 480 * NUM_KS);
   }

   raninit();
   for (int k = 0; k < NUM_KS; ++k)
   {
      int centerx = random_generate(frame_width);
      int centery = random_generate(frame_height);

      frame_index = M_FRAME_XY_TO_INDEX(frame_width, centerx, centery);
      for (int c = 0; c < 3; ++c)
         k_positions[k].rgb[c] = frame_data[frame_index + c];
   }

   for (int t = 0; t < 10 && centers_diff > 100; ++t)
   {
      for (int k = 0; k < NUM_KS; ++k)
      {
         k_positions[k].num_pixels = 0;
         for (int c = 0; c < 3; ++c)
            k_positions[k].total_rgb[c] = 0;
      }

      for (int x = 0; x < frame_width; ++x)
      {
         for (int y = 0; y < frame_height; ++y)
         {
            cube_assert((y*frame_height + x)*NUM_KS < 640 * 480 * NUM_KS);
            int min_k = kmeans_calc_distances(frame_p, frame_width, frame_height, x, y, k_positions, &distances_p[(y*frame_height + x)*NUM_KS]);
            labels[x][y] = min_k;
         }
      }

      centers_diff = 0;

      int total_pixels = 0;
      for (int k = 0; k < NUM_KS; ++k)
      {
         total_pixels += k_positions[k].num_pixels;
         for (int c = 0; c < 3; ++c)
         {
            if (k_positions[k].num_pixels)
            {
               int new_color = k_positions[k].total_rgb[c] / k_positions[k].num_pixels;
               centers_diff += M_DIFF_ABS(new_color, k_positions[k].rgb[c]);
               k_positions[k].rgb[c] = (unsigned char)new_color;
            }
         }
      }
   }

   static int rcolors[] = { 255, 0, 0, 255, 255, 0, 128, 128, 0, 0 };
   static int gcolors[] = { 0, 255, 0, 0, 255, 255, 128, 0, 128, 0};
   static int bcolors[] = { 0, 0, 255, 255, 0, 255, 128, 0, 0, 128 };
   for (int x = 0; x < frame_width; ++x)
   {
      for (int y = 0; y < frame_height; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, x, y);
         /*frame_data[frame_index + 0] = rcolors[labels[x][y]];
         frame_data[frame_index + 1] = gcolors[labels[x][y]];
         frame_data[frame_index + 2] = bcolors[labels[x][y]];*/
         frame_data[frame_index + 0] = k_positions[labels[x][y]].rgb[0];
         frame_data[frame_index + 1] = k_positions[labels[x][y]].rgb[1];
         frame_data[frame_index + 2] = k_positions[labels[x][y]].rgb[2];
      }
   }
}

static int kmeans_calc_distances(void* frame_p, int frame_width, int frame_height, int x, int y, KMeansCenter_t* kcenters_p, int* distances_p)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   int result = 0;
   int rr, gg, bb;
   int min_dist = 0x7fffffff;
   int min_k = -1;

   cube_assert(x < frame_width && y < frame_height);
   frame_index = M_FRAME_XY_TO_INDEX(frame_width, x, y);
   rr = frame_data[frame_index + 0];
   gg = frame_data[frame_index + 1];
   bb = frame_data[frame_index + 2];

   for (int k = 0; k < NUM_KS; ++k)
   {
      distances_p[k] = 0;
      distances_p[k] += M_DIFF_ABS(kcenters_p[k].rgb[0], rr) * M_DIFF_ABS(kcenters_p[k].rgb[0], rr);
      distances_p[k] += M_DIFF_ABS(kcenters_p[k].rgb[1], gg) * M_DIFF_ABS(kcenters_p[k].rgb[1], gg);
      distances_p[k] += M_DIFF_ABS(kcenters_p[k].rgb[2], bb) * M_DIFF_ABS(kcenters_p[k].rgb[2], bb);

      if (distances_p[k] < min_dist)
      {
         min_dist = distances_p[k];
         min_k = k;
      }
   }
   cube_assert(0 <= min_k && min_k < NUM_KS);
   
   kcenters_p[min_k].total_rgb[0] += rr;
   kcenters_p[min_k].total_rgb[1] += gg;
   kcenters_p[min_k].total_rgb[2] += bb;
   kcenters_p[min_k].num_pixels++;

   return min_k;
}

static void median_filter(void* frame_p, int frame_width, int frame_height)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   static char new_image[640 * 480 * 3];
   
   for (int x = 0; x < frame_width; ++x)
   {
      for (int y = 0; y < frame_height; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, x, y);
         for (int c = 0; c < 3; ++c)
            new_image[frame_index + c] = get_median(frame_p, frame_width, frame_height, x, y, c);
      }
   }

   memcpy(frame_p, new_image, frame_width*frame_height * 3);
}

static unsigned char get_median(void* frame_p, int frame_width, int frame_height, int x, int y, int rgb)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   int nx;
   int ny;
   int vvv = 0;
   unsigned char window[9];
   unsigned char window_index;

   window_index = 0;
   for (int xoffset = -1; xoffset < 2; ++xoffset)
   {
      for (int yoffset = -1; yoffset < 2; ++yoffset)
      {
         nx = x + xoffset;
         ny = y + yoffset;
         if ((nx < 0 || nx >= frame_width) || (ny < 0 || ny >= frame_height))
         {
            vvv = 0;
         }
         else
         {
            frame_index = M_FRAME_XY_TO_INDEX(frame_width, nx, ny);
            vvv = frame_data[frame_index + rgb];
         }

         window[window_index] = vvv;
         window_index++;
      }
   }

   insertion_sort(window, window_index - 1);
   return window[4];
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

         /*if (x == 0 || y == 0)
         {
            frame_data[frame_index + 0] = 255;
            frame_data[frame_index + 1] = 0;
            frame_data[frame_index + 2] = 0;
         }
         else*/
         {
            frame_data[frame_index + 0] = total_r;
            frame_data[frame_index + 1] = total_g;
            frame_data[frame_index + 2] = total_b;
         }
      }
   }

   //diff_from_avg /= (BOX_SIZE_X * BOX_SIZE_Y);

   return diff_from_avg < 500000*100/4;
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


static void insertion_sort(unsigned char arr[], int n)
{
   int i, key, j;
   for (i = 1; i < n; i++)
   {
      key = arr[i];
      j = i - 1;

      /* Move elements of arr[0..i-1], that are
      greater than key, to one position ahead
      of their current position */
      while (j >= 0 && arr[j] > key)
      {
         arr[j + 1] = arr[j];
         j = j - 1;
      }
      arr[j + 1] = key;
   }
}

