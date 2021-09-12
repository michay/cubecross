#include <math.h>
#include <string.h>
#include<malloc.h>
#include <stdio.h>
#include "globals.h"
#include "image_processing.h"

#define USE_XYZ

typedef enum
{
   RGB_RED = 2,
   RGB_GREEN = 1,
   RGB_BLUE = 0,
} RGB_t;

typedef enum
{
   XYZ_X = 0,
   XYZ_Y = 1,
   XYZ_Z = 2,
} XYZ_t;

typedef enum
{
   COLOR_CONVERT_RGB_TO_XYZ,
   COLOR_CONVERT_XYZ_TO_RGB,
} ColorConvertion_t;

#define NUM_KS (18)
#define BOX_SIZE_X (16/2)
#define BOX_SIZE_Y (16/2)

#define CENTER_SIZE_X (210)
#define CENTER_SIZE_Y (210)

#define M_FRAME_XY_TO_INDEX(_height_, _xoffset_, _yoffset_) (((_yoffset_) * (_height_) + (_xoffset_)) * 3)
#define M_KMEANS_CENTER_VALUE(_kcenter_p_) ((_kcenter_p_)->rgb[0] + (_kcenter_p_)->rgb[1] + (_kcenter_p_)->rgb[2])

typedef struct
{
   int self_index;
   int same_as;
   unsigned char rgb[3];
   unsigned char rfu0;
   double xyz[3];
   long long total_rgb[3];
   double total_xyz[3];
   int num_pixels;
} KMeansCenter_t;

typedef struct
{
   unsigned char avg_rgb[3];
   unsigned char rfu0;
   int threshold;
} FrameBoxResult_t;


__inline  void convert_pixel_rgb_to_xyz(unsigned char* pixel_p);
__inline  void convert_pixel_xyz_to_rgb(unsigned char* pixel_p);

__inline double pixel_rgb_to_xyz(unsigned char rgb);
__inline unsigned char pixel_xyz_to_rgb(double rgb);

static void add_kmeans_to_frame(void* frame_p, int frame_width, int frame_height, KMeansCenter_t** kmeans_p);

static void check_frame_box(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset, int sizex, int sizey, FrameBoxResult_t* result_p);
static void fill_box_color(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset, int sizex, int sizey, int r, int g, int b);

static void create_frame(void* frame_p, int frame_width, int frame_height, int frame_size);

static void median_filter(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset, int sizex, int sizey);
static unsigned char get_median(void* frame_p, int frame_width, int frame_height, int x, int y, int rgb);

static void kmeans(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset, int subframe_sizex, int subframe_sizey);
static int kmeans_calc_distances(void* frame_p, int frame_width, int frame_height, int x, int y, KMeansCenter_t* kcenters_p);
static int kmeans_calc_distances_xyz(void* frame_p, int frame_width, int frame_height, int x, int y, KMeansCenter_t* kcenters_p);
static int xyz_points_distance(double* xyz1_p, double* xyz2_p);
static int rgb_points_distance(unsigned char* rgb1_p, unsigned char* rgb2_p);

static void insertion_sort(unsigned char arr[], int n);
static void insertion_sort_kmeans(KMeansCenter_t* arr[], int n);

__declspec(dllexport) int modify_frame(void* frame_p, int width, int height)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int should_keep_box;
   FrameBoxResult_t avg_box;
   int xoffset = (width - CENTER_SIZE_X) / 2;
   int yoffset = (height - CENTER_SIZE_Y) / 2;
   //int distances[640][480][NUM_KS];

   //median_filter(frame_p, width, height);
   
   //median_filter(frame_p, width, height, xoffset, yoffset, CENTER_SIZE, CENTER_SIZE);
   kmeans(frame_p, width, height, xoffset, yoffset, CENTER_SIZE_X, CENTER_SIZE_Y);
   //kmeans(frame_p, width, height, 0, 0, width, height);

   /*
   for (int i = xoffset; i < xoffset + CENTER_SIZE; i += CENTER_SIZE/3)
   {
      for (int j = yoffset; j < yoffset + CENTER_SIZE; j += CENTER_SIZE / 3)
      {
         check_frame_box(frame_p, width, height, i, j, CENTER_SIZE / 3, CENTER_SIZE / 3, &avg_box);

         should_keep_box = TRUE;
         if (should_keep_box)
            fill_box_color(frame_p, width, height, i, j, CENTER_SIZE / 3, CENTER_SIZE / 3, avg_box.avg_rgb[RGB_RED], avg_box.avg_rgb[RGB_GREEN], avg_box.avg_rgb[RGB_BLUE]);
         else
            fill_box_color(frame_p, width, height, i, j, CENTER_SIZE / 3, CENTER_SIZE / 3, 191, 191, 191);
      }
   }
   //*/

   //create_frame(frame_p, width, height, CENTER_SIZE);
   return 0;
}

static void kmeans(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset, int subframe_sizex, int subframe_sizey)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   KMeansCenter_t k_positions[NUM_KS];
   KMeansCenter_t* k_positions_sorted[NUM_KS];
   int centers_diff = 500;
   unsigned char labels[640][480] = { 0 };

   raninit();
   for (int k = 0; k < NUM_KS; ++k)
   {
      /*
      int centerx = random_generate(subframe_sizex);
      int centery = random_generate(subframe_sizey);
      */
      int centerx = subframe_sizex / 3 * (k / 3) + subframe_sizex / 6;
      int centery = subframe_sizey / 3 * (k % 3) + subframe_sizex / 6;

      frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + centerx, yoffset + centery);
      for (int c = 0; c < 3; ++c)
         k_positions[k].rgb[c] = frame_data[frame_index + c];
   }

   for (int t = 0; t < 10 && centers_diff > 50; ++t)
   {
      for (int k = 0; k < NUM_KS; ++k)
      {
         // Reset num pixels
         k_positions[k].num_pixels = 0;
         for (int c = 0; c < 3; ++c)
         {
            k_positions[k].total_rgb[c] = 0;
            k_positions[k].total_xyz[c] = 0;
         }

         convert_pixel_rgb_to_xyz(k_positions[k].rgb, k_positions[k].xyz);
      }

      for (int x = xoffset; x < xoffset + subframe_sizex; ++x)
      {
         for (int y = yoffset; y < yoffset + subframe_sizey; ++y)
         {
            cube_assert((y*frame_height + x)*NUM_KS < 640 * 480 * NUM_KS);
#ifdef USE_XYZ
            int min_k = kmeans_calc_distances_xyz(frame_p, frame_width, frame_height, x, y, k_positions);
#else
            int min_k = kmeans_calc_distances(frame_p, frame_width, frame_height, x, y, k_positions);
#endif
            labels[x][y] = min_k;
         }
      }

      centers_diff = 0;

      int total_pixels = 0;
      for (int k = 0; k < NUM_KS; ++k)
      {
         total_pixels += k_positions[k].num_pixels;

         if (k_positions[k].num_pixels)
         {
#ifdef USE_XYZ
            for (int c = 0; c < 3; ++c)
            {
               k_positions[k].total_xyz[c] /= k_positions[k].num_pixels;
            }
            convert_pixel_xyz_to_rgb(k_positions[k].total_xyz, k_positions[k].rgb);
#else
            for (int c = 0; c < 3; ++c)
            {
               int new_color = (int)(k_positions[k].total_rgb[c] / k_positions[k].num_pixels);
               centers_diff += M_DIFF_ABS(new_color, k_positions[k].rgb[c]);
               k_positions[k].rgb[c] = (unsigned char)new_color;
            }
#endif
         }
      }
   }


   for (int k = 0; k < NUM_KS; ++k)
   {
      k_positions[k].self_index = k;
      k_positions[k].same_as = k;
      k_positions_sorted[k] = &k_positions[k];

      convert_pixel_rgb_to_xyz(k_positions[k].rgb, k_positions[k].xyz);
   }

   insertion_sort_kmeans(k_positions_sorted, NUM_KS);

   int diffs[NUM_KS][NUM_KS] = { 0 };
   for (int k = 0; k < NUM_KS-1; ++k)
   {
      for (int sk = k + 1; sk < NUM_KS; ++sk)
      {
         if (k_positions_sorted[sk]->same_as != k_positions_sorted[sk]->self_index)
            continue;

         int dist = 0;
#ifdef USE_XYZ
         dist = xyz_points_distance(k_positions_sorted[k]->xyz, k_positions_sorted[sk]->xyz);
         //dist = rgb_points_distance(k_positions_sorted[k]->rgb, k_positions_sorted[sk]->rgb);
#else
         dist += M_DIFF_ABS(k_positions_sorted[k]->rgb[RGB_RED], k_positions_sorted[sk]->rgb[RGB_RED]);
         dist += M_DIFF_ABS(k_positions_sorted[k]->rgb[RGB_GREEN], k_positions_sorted[sk]->rgb[RGB_GREEN]);
         dist += M_DIFF_ABS(k_positions_sorted[k]->rgb[RGB_BLUE], k_positions_sorted[sk]->rgb[RGB_BLUE]);
#endif

         if (dist < 20)
            k_positions_sorted[sk]->same_as = k_positions_sorted[k]->same_as;

         diffs[k][sk] = dist;
      }
   }

   static int rcolors[] = { 255, 0, 0, 255, 255, 0, 128, 128, 0, 0 };
   static int gcolors[] = { 0, 255, 0, 0, 255, 255, 128, 0, 128, 0};
   static int bcolors[] = { 0, 0, 255, 255, 0, 255, 128, 0, 0, 128 };

#if 1
   for (int i = 0; i < 3; ++i)
   {
      for (int j = 0; j < 3; ++j)
      {
         int k_inst[NUM_KS] = { 0 };
         for (int x = 0; x < subframe_sizex / 3; ++x)
         {
            for (int y = 0; y < subframe_sizey / 3; ++y)
            {
               int framex = xoffset + i * subframe_sizex / 3 + x;
               int framey = yoffset + j * subframe_sizey / 3 + y;
               frame_index = M_FRAME_XY_TO_INDEX(frame_width, framex, framey);
               int k_index = k_positions[labels[framex][framey]].same_as;
               k_inst[k_index]++;
            }
         }

         // find max k_inst
         int max_k = 0;
         for (int k = 0; k < NUM_KS; ++k)
         {
            if (k_inst[k] > k_inst[max_k])
            {
               max_k = k;
            }
         }

         for (int x = 0; x < subframe_sizex / 3; ++x)
         {
            for (int y = 0; y < subframe_sizey / 3; ++y)
            {
               frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + i * subframe_sizex / 3 + x, yoffset + j * subframe_sizey / 3 + y);
               frame_data[frame_index + RGB_RED] = k_positions[max_k].rgb[RGB_RED];
               frame_data[frame_index + RGB_GREEN] = k_positions[max_k].rgb[RGB_GREEN];
               frame_data[frame_index + RGB_BLUE] = k_positions[max_k].rgb[RGB_BLUE];
            }
         }
      }
   }
#else
   for (int x = xoffset; x < xoffset + subframe_sizex; ++x)
   {
      for (int y = yoffset; y < yoffset + subframe_sizey; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, x, y);
         /*
         frame_data[frame_index + RGB_RED] = rcolors[k_positions[labels[x][y]].same_as];
         frame_data[frame_index + RGB_GREEN] = gcolors[k_positions[labels[x][y]].same_as];
         frame_data[frame_index + RGB_BLUE] = bcolors[k_positions[labels[x][y]].same_as];
         //*/
         //*
         frame_data[frame_index + RGB_RED] = k_positions[k_positions[labels[x][y]].same_as].rgb[RGB_RED];
         frame_data[frame_index + RGB_GREEN] = k_positions[k_positions[labels[x][y]].same_as].rgb[RGB_GREEN];
         frame_data[frame_index + RGB_BLUE] = k_positions[k_positions[labels[x][y]].same_as].rgb[RGB_BLUE];
         //*/
      }
   }
#endif

   add_kmeans_to_frame(frame_p, frame_width, frame_height, k_positions_sorted);
}

static int kmeans_calc_distances(void* frame_p, int frame_width, int frame_height, int x, int y, KMeansCenter_t* kcenters_p)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   int rr, gg, bb;
   int min_dist = 0x7fffffff;
   int min_k = -1;

   cube_assert(x < frame_width && y < frame_height);
   frame_index = M_FRAME_XY_TO_INDEX(frame_width, x, y);
   
   rr = frame_data[frame_index + RGB_RED];
   gg = frame_data[frame_index + RGB_GREEN];
   bb = frame_data[frame_index + RGB_BLUE];

   for (int k = 0; k < NUM_KS; ++k)
   {
      int distance = 0;
      distance += M_DIFF_ABS(kcenters_p[k].rgb[RGB_RED], rr) * M_DIFF_ABS(kcenters_p[k].rgb[RGB_RED], rr);
      distance += M_DIFF_ABS(kcenters_p[k].rgb[RGB_GREEN], gg) * M_DIFF_ABS(kcenters_p[k].rgb[RGB_GREEN], gg);
      distance += M_DIFF_ABS(kcenters_p[k].rgb[RGB_BLUE], bb) * M_DIFF_ABS(kcenters_p[k].rgb[RGB_BLUE], bb);

      if (distance < min_dist)
      {
         min_dist = distance;
         min_k = k;
      }
   }
   cube_assert(0 <= min_k && min_k < NUM_KS);
   
   kcenters_p[min_k].total_rgb[RGB_RED] += rr;
   kcenters_p[min_k].total_rgb[RGB_GREEN] += gg;
   kcenters_p[min_k].total_rgb[RGB_BLUE] += bb;
   kcenters_p[min_k].num_pixels++;

   return min_k;
}

static int kmeans_calc_distances_xyz(void* frame_p, int frame_width, int frame_height, int x, int y, KMeansCenter_t* kcenters_p)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   double xx, yy, zz;
   int min_dist = 0;
   int min_k = -1;
   double xyz[3];

   frame_index = M_FRAME_XY_TO_INDEX(frame_width, x, y);
   convert_pixel_rgb_to_xyz(&frame_data[frame_index], xyz);

   xx = xyz[XYZ_X];
   yy = xyz[XYZ_Y];
   zz = xyz[XYZ_Z];

   for (int k = 0; k < NUM_KS; ++k)
   {
      double distance = 0;
      distance += M_DIFF_ABS(kcenters_p[k].xyz[XYZ_X], xx) * M_DIFF_ABS(kcenters_p[k].rgb[XYZ_X], xx);
      distance += M_DIFF_ABS(kcenters_p[k].xyz[XYZ_Y], yy) * M_DIFF_ABS(kcenters_p[k].rgb[XYZ_Y], yy);
      distance += M_DIFF_ABS(kcenters_p[k].xyz[XYZ_Z], zz) * M_DIFF_ABS(kcenters_p[k].rgb[XYZ_Z], zz);

      if (k == 0 || distance < min_dist)
      {
         min_dist = distance;
         min_k = k;
      }
   }
   cube_assert(0 <= min_k && min_k < NUM_KS);

   kcenters_p[min_k].total_xyz[XYZ_X] += xx;
   kcenters_p[min_k].total_xyz[XYZ_Y] += yy;
   kcenters_p[min_k].total_xyz[XYZ_Z] += zz;
   kcenters_p[min_k].num_pixels++;

   return min_k;
}

static int xyz_points_distance(double* xyz1_p, double* xyz2_p)
{
   /*double x1;
   double x2;
   double y1;
   double y2;

   x1 = xyz1_p[XYZ_X] / (xyz1_p[XYZ_X] + xyz1_p[XYZ_Y] + xyz1_p[XYZ_Z]);
   x2 = xyz2_p[XYZ_X] / (xyz2_p[XYZ_X] + xyz2_p[XYZ_Y] + xyz2_p[XYZ_Z]);

   y1 = xyz1_p[XYZ_Y] / (xyz1_p[XYZ_X] + xyz1_p[XYZ_Y] + xyz1_p[XYZ_Z]);
   y2 = xyz2_p[XYZ_Y] / (xyz2_p[XYZ_X] + xyz2_p[XYZ_Y] + xyz2_p[XYZ_Z]);

   return (int)(10000*((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2)));*/

   int xdiff;
   int ydiff;
   int zdiff;

   xdiff = xyz1_p[XYZ_X] - xyz2_p[XYZ_X];
   ydiff = xyz1_p[XYZ_Y] - xyz2_p[XYZ_Y];
   zdiff = xyz1_p[XYZ_Z] - xyz2_p[XYZ_Z];

   return 10 * (xdiff*xdiff + ydiff*ydiff + zdiff*zdiff);
}

static int rgb_points_distance(unsigned char* rgb1_p, unsigned char* rgb2_p)
{
   int rdiff;
   int gdiff;
   int bdiff;
   int rmean;
   long long result;

   rmean = (rgb1_p[RGB_RED] + rgb2_p[RGB_RED]) / 2;
   rdiff = rgb1_p[RGB_RED] - rgb2_p[RGB_RED];
   gdiff = rgb1_p[RGB_GREEN] - rgb2_p[RGB_GREEN];
   bdiff = rgb1_p[RGB_BLUE] - rgb2_p[RGB_BLUE];

/*   result = ((512 + rmean)*rdiff*rdiff) >> 8;
   result += 4 * gdiff * gdiff;
   result += ((767 - rmean)*bdiff*bdiff) >> 8;
   return result;*/

   result = rdiff*rdiff + gdiff*gdiff + bdiff*bdiff;
   return result;
}

static void add_kmeans_to_frame(void* frame_p, int frame_width, int frame_height, KMeansCenter_t** kmeans_p)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   int kmeans_height = 30;
   int kmeans_width = 30;
   int xoffset = 0;
   int yoffset = frame_height - kmeans_height - 1;
   
   for (int k = 0; k < NUM_KS; ++k)
   {
      xoffset = k * kmeans_width;

      for (int x = xoffset; x < xoffset + kmeans_width; ++x)
      {
         for (int y = yoffset; y < yoffset + kmeans_height; ++y)
         {
            frame_index = M_FRAME_XY_TO_INDEX(frame_width, x, y);
            
            frame_data[frame_index + RGB_RED] = kmeans_p[k]->rgb[RGB_RED];
            frame_data[frame_index + RGB_GREEN] = kmeans_p[k]->rgb[RGB_GREEN];
            frame_data[frame_index + RGB_BLUE] = kmeans_p[k]->rgb[RGB_BLUE];

            if (x == xoffset || x == xoffset + kmeans_width - 1 || y == yoffset || y == yoffset + kmeans_height - 1)
            {
               frame_data[frame_index + RGB_RED] = (kmeans_p[k]->same_as == kmeans_p[k]->self_index) ? 255 : 0;
               frame_data[frame_index + RGB_GREEN] = 0;
               frame_data[frame_index + RGB_BLUE] = 0;
            }
         }
      }
   }
}

static void median_filter(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset, int sizex, int sizey)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   static char new_image[640 * 480 * 3];

   memcpy(new_image, frame_p, frame_width*frame_height * 3);
   
   for (int x = xoffset; x < xoffset + sizex; ++x)
   {
      for (int y = yoffset; y < yoffset + sizey; ++y)
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

static void check_frame_box(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset, int sizex, int sizey, FrameBoxResult_t* result_p)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   int is_cube_frame = FALSE;

   int total_r = 0;
   int total_g = 0;
   int total_b = 0;
   int diff_from_avg = 0;

   for (int x = 0; x < sizex; ++x)
   {
      for (int y = 0; y < sizey; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + x, yoffset + y);
         total_r += frame_data[frame_index + RGB_RED];
         total_g += frame_data[frame_index + RGB_GREEN];
         total_b += frame_data[frame_index + RGB_BLUE];
      }
   }

   total_r /= (sizex * sizey);
   total_g /= (sizex * sizey);
   total_b /= (sizex * sizey);

   for (int x = 0; x < sizex; ++x)
   {
      for (int y = 0; y < sizey; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + x, yoffset + y);
         
         diff_from_avg += M_DIFF_ABS(frame_data[frame_index + RGB_RED], total_r) * M_DIFF_ABS(frame_data[frame_index + RGB_RED], total_r);
         diff_from_avg += M_DIFF_ABS(frame_data[frame_index + RGB_GREEN], total_g) * M_DIFF_ABS(frame_data[frame_index + RGB_GREEN], total_g);
         diff_from_avg += M_DIFF_ABS(frame_data[frame_index + RGB_BLUE], total_b) * M_DIFF_ABS(frame_data[frame_index + RGB_BLUE], total_b);

         /*if (x == 0 || y == 0)
         {
         frame_data[frame_index + RGB_RED  ] = 255;
         frame_data[frame_index + RGB_GREEN] = 0;
         frame_data[frame_index + RGB_BLUE ] = 0;
         }*/
         /*else
         {
         frame_data[frame_index + RGB_RED  ] = total_r;
         frame_data[frame_index + RGB_GREEN] = total_g;
         frame_data[frame_index + RGB_BLUE ] = total_b;
         }*/
      }
   }

   //diff_from_avg /= (BOX_SIZE_X * BOX_SIZE_Y);

   result_p->avg_rgb[RGB_RED] = total_r;
   result_p->avg_rgb[RGB_GREEN] = total_g;
   result_p->avg_rgb[RGB_BLUE] = total_b;
   result_p->threshold = diff_from_avg;
}

static void fill_box_color(void* frame_p, int frame_width, int frame_height, int xoffset, int yoffset, int sizex, int sizey, int r, int g, int b)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;

   for (int x = 1; x < sizex; ++x)
   {
      for (int y = 1; y < sizey; ++y)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + x, yoffset + y);
         if (frame_index >= frame_width * frame_height * 3)
         {
            frame_index = frame_index;
         }
         frame_data[frame_index + RGB_RED] = r;
         frame_data[frame_index + RGB_GREEN] = g;
         frame_data[frame_index + RGB_BLUE] = b;
      }
   }
}

static void create_frame(void* frame_p, int frame_width, int frame_height, int frame_size)
{
   unsigned char* frame_data = (unsigned char*)frame_p;
   int frame_index;
   int xoffset = (frame_width - frame_size) / 2;
   int yoffset = (frame_height - frame_size) / 2;

   for (int i = 0; i < frame_size; ++i)
   {
      for (int x = 0; x < 4; ++x)
      {
         frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + i, yoffset + frame_size*x/3);
         frame_data[frame_index + RGB_RED] = 255;
         frame_data[frame_index + RGB_GREEN] = 0;
         frame_data[frame_index + RGB_BLUE] = 0;

         frame_index = M_FRAME_XY_TO_INDEX(frame_width, xoffset + frame_size*x/3, yoffset + i);
         frame_data[frame_index + RGB_RED] = 255;
         frame_data[frame_index + RGB_GREEN] = 0;
         frame_data[frame_index + RGB_BLUE] = 0;
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

static void insertion_sort_kmeans(KMeansCenter_t* arr[], int n)
{
   int i, j;
   KMeansCenter_t* key;
   
   for (i = 1; i < n; i++)
   {
      key = arr[i];
      j = i - 1;

      /* Move elements of arr[0..i-1], that are
      greater than key, to one position ahead
      of their current position */
      while (j >= 0 && M_KMEANS_CENTER_VALUE(arr[j]) < M_KMEANS_CENTER_VALUE(key))
      {
         arr[j + 1] = arr[j];
         j = j - 1;
      }
      arr[j + 1] = key;
   }
}

__inline void convert_pixel_rgb_to_xyz(unsigned char* rgb_pixel_p, double* xyz_pixel_p)
{
   double r;
   double g;
   double b;

   r = pixel_rgb_to_xyz(rgb_pixel_p[RGB_RED]);
   g = pixel_rgb_to_xyz(rgb_pixel_p[RGB_GREEN]);
   b = pixel_rgb_to_xyz(rgb_pixel_p[RGB_BLUE]);

   xyz_pixel_p[XYZ_X] = r * 0.4124 + g * 0.3576 + b * 0.1805;
   xyz_pixel_p[XYZ_Y] = r * 0.2126 + g * 0.7152 + b * 0.0722;
   xyz_pixel_p[XYZ_Z] = r * 0.0193 + g * 0.1192 + b * 0.9505;
}

__inline void convert_pixel_xyz_to_rgb(double* xyz_pixel_p, unsigned char* rgb_pixel_p)
{
   double x;
   double y;
   double z;
   double r;
   double g;
   double b;

   x = xyz_pixel_p[XYZ_X] / 100.0;
   y = xyz_pixel_p[XYZ_Y] / 100.0;
   z = xyz_pixel_p[XYZ_Z] / 100.0;

   r = x *  3.2406 + y * -1.5372 + z * -0.4986;
   g = x * -0.9689 + y *  1.8758 + z *  0.0415;
   b = x *  0.0557 + y * -0.2040 + z *  1.0570;

   rgb_pixel_p[RGB_RED] = pixel_xyz_to_rgb(r);
   rgb_pixel_p[RGB_GREEN] = pixel_xyz_to_rgb(g);
   rgb_pixel_p[RGB_BLUE] = pixel_xyz_to_rgb(b);
}

__inline double pixel_rgb_to_xyz(unsigned char rgb)
{
   double result;

   result = (double)rgb / 255.0;

   if (result > 0.04045)
      result = pow((result + 0.055) / 1.055, 2.4);
   else
      result = result / 12.92;

   return result * 100;
}

__inline unsigned char pixel_xyz_to_rgb(double rgb)
{
   double result;

   if (rgb > 0.0031308)
      result = 1.055 * pow(rgb, (1 / 2.4)) - 0.055;
   else
      result = 12.92 * rgb;

   return (unsigned char)(result * 255);
}