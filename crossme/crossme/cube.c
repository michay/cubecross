#include <stdio.h>
#include <string.h>
#include "cube.h"
#include "rotation.h"

static void init_cube_side(CubeSide_t* side_p, int color); 
static void link_up_down(CubeSide_t* up_side_p, CubeSide_t* down_side_p);
static void link_left_right(CubeSide_t* left_side_p, CubeSide_t* right_side_p);
static void link_two_stickers(Sticker_t* sticker1_p, CubeSide_t* side1_p, Sticker_t* sticker2_p, CubeSide_t* side2_p);

void init_cube(Cube_t* cube_p, int is_white_top)
{
   CubeSide_t* back_side_p;
   CubeSide_t* front_side_p;
   CubeSide_t* left_side_p;
   CubeSide_t* up_side_p;
   CubeSide_t* right_side_p;
   CubeSide_t* down_side_p;
   Sticker_t* usticker_p;
   Sticker_t* bsticker_p;
   Sticker_t* lsticker_p;
   Sticker_t* rsticker_p;
   Sticker_t* dsticker_p;
   int i;
   int fixed_index;

   CubeSide_t* side_p;

   memset(cube_p, 0, sizeof(Cube_t));

   side_p = &cube_p->sides[0];
   fixed_index = 0;
   for (i = 0; i < CUBE_SIDE_COUNT; ++i, ++side_p)
   {
      cube_p->sides_hash[i] = i;

      memset(side_p, 0, sizeof(CubeSide_t));
      init_cube_side(side_p, i);

      for (int s = 0; s < CUBE_SIZE * CUBE_SIZE; ++s)
      {
         side_p->stickers[s].fixed_index = fixed_index;
         cube_p->fixed_stickers[fixed_index] = &side_p->stickers[s];
         fixed_index++;
      }
   }

   back_side_p = get_cube_side(cube_p, CUBE_SIDE_BACK);
   front_side_p = get_cube_side(cube_p, CUBE_SIDE_FRONT);
   left_side_p = get_cube_side(cube_p, CUBE_SIDE_LEFT);
   up_side_p = get_cube_side(cube_p, CUBE_SIDE_UP);
   right_side_p = get_cube_side(cube_p, CUBE_SIDE_RIGHT);
   down_side_p = get_cube_side(cube_p, CUBE_SIDE_DOWN);

   link_up_down(front_side_p, down_side_p);
   link_up_down(up_side_p, front_side_p);

   for (i = 0; i < CUBE_SIZE; ++i)
   {
      usticker_p = &up_side_p->stickers[0 + i];
      bsticker_p = &back_side_p->stickers[2 - i];
      link_two_stickers(usticker_p, up_side_p, bsticker_p, back_side_p);

      bsticker_p = &back_side_p->stickers[8 - i];
      dsticker_p = &down_side_p->stickers[6 + i];
      link_two_stickers(bsticker_p, back_side_p, dsticker_p, down_side_p);
   }

   link_left_right(left_side_p, front_side_p);
   link_left_right(front_side_p, right_side_p);
   link_left_right(right_side_p, back_side_p);
   link_left_right(back_side_p, left_side_p);

   for (i = 0; i < CUBE_SIZE; ++i)
   {
      lsticker_p = &left_side_p->stickers[0 + i];
      usticker_p = &up_side_p->stickers[0 + i * 3];
      link_two_stickers(lsticker_p, left_side_p, usticker_p, up_side_p);

      usticker_p = &up_side_p->stickers[2 + i * 3];
      rsticker_p = &right_side_p->stickers[2 - i];
      link_two_stickers(usticker_p, up_side_p, rsticker_p, right_side_p);

      rsticker_p = &right_side_p->stickers[6 + i];
      dsticker_p = &down_side_p->stickers[2 + i * 3];
      link_two_stickers(rsticker_p, right_side_p, dsticker_p, down_side_p);

      dsticker_p = &down_side_p->stickers[0 + i * 3];
      lsticker_p = &left_side_p->stickers[8 - i];
      link_two_stickers(dsticker_p, down_side_p, lsticker_p, left_side_p);
   }

   if (!is_white_top)
   {
      rotate_cube_string(cube_p, "Z2", FALSE, TRUE);
   }
}

static void init_cube_side(CubeSide_t* side_p, int color)
{
   Sticker_t* sticker_p;

   side_p->color = color;

   sticker_p = side_p->stickers;
   for (int i = 0; i < CUBE_SIZE * CUBE_SIZE; ++i, ++sticker_p)
   {
      sticker_p->active.values.color = color;
      sticker_p->active.values.unique_index = (color + 1) * 10 + i;
   }
}

static void link_two_stickers(Sticker_t* sticker1_p, CubeSide_t* side1_p, Sticker_t* sticker2_p, CubeSide_t* side2_p)
{
   sticker1_p->linked_stickers[sticker1_p->linked_stickers_count].connected_side_p = side2_p;
   sticker1_p->linked_stickers[sticker1_p->linked_stickers_count].sticker_index = sticker2_p - side2_p->stickers;
   sticker1_p->linked_stickers_count++;
   cube_assert(sticker1_p->linked_stickers_count <= MAX_LINKED_STICKERS);

   sticker2_p->linked_stickers[sticker2_p->linked_stickers_count].connected_side_p = side1_p;
   sticker2_p->linked_stickers[sticker2_p->linked_stickers_count].sticker_index = sticker1_p - side1_p->stickers;
   sticker2_p->linked_stickers_count++;
   cube_assert(sticker2_p->linked_stickers_count <= MAX_LINKED_STICKERS);
}

static void link_up_down(CubeSide_t* up_side_p, CubeSide_t* down_side_p)
{
   Sticker_t* dsticker_p;
   Sticker_t* usticker_p;

   dsticker_p = &down_side_p->stickers[0];
   usticker_p = &up_side_p->stickers[6];
   for (int i = 0; i < CUBE_SIZE; ++i)
   {
      link_two_stickers(dsticker_p, down_side_p, usticker_p, up_side_p);
      dsticker_p++;
      usticker_p++;
   }
}

static void link_left_right(CubeSide_t* left_side_p, CubeSide_t* right_side_p)
{
   Sticker_t* lsticker_p;
   Sticker_t* rsticker_p;

   lsticker_p = &left_side_p->stickers[2];
   rsticker_p = &right_side_p->stickers[0];
   for (int i = 0; i < CUBE_SIZE; ++i)
   {
      link_two_stickers(lsticker_p, left_side_p, rsticker_p, right_side_p);
      lsticker_p += CUBE_SIZE;
      rsticker_p += CUBE_SIZE;
   }
}

void print_orientation(Cube_t* cube_p)
{
   static char* colors_hash[] = { "Blue", "White", "Green", "Orange", "Red", "Yellow" };
   CubeSide_t* top_side_p;
   CubeSide_t* front_side_p;

   top_side_p = get_cube_side(cube_p, CUBE_SIDE_UP);
   front_side_p = get_cube_side(cube_p, CUBE_SIDE_FRONT);
   printf("%s top, %s front", colors_hash[top_side_p->color], colors_hash[front_side_p->color]);
}

CubeSide_t* get_cube_side(Cube_t* cube_p, int cube_side)
{
   return &cube_p->sides[cube_p->sides_hash[cube_side]];
}

void print_cube(Cube_t* cube_p)
{
   char colors_hash[] = { 'B', 'W', 'G', 'O', 'R', 'Y' };
   CubeSide_t* back_side_p = get_cube_side(cube_p, CUBE_SIDE_BACK);
   CubeSide_t* front_side_p = get_cube_side(cube_p, CUBE_SIDE_FRONT);
   CubeSide_t* left_side_p = get_cube_side(cube_p, CUBE_SIDE_LEFT);
   CubeSide_t* up_side_p = get_cube_side(cube_p, CUBE_SIDE_UP);
   CubeSide_t* right_side_p = get_cube_side(cube_p, CUBE_SIDE_RIGHT);
   CubeSide_t* down_side_p = get_cube_side(cube_p, CUBE_SIDE_DOWN);

   printf("[W = White; G = Green; R = Red; O = Orange; B = Blue; Y = Yellow]\n");

   for (int i = 0; i < CUBE_SIZE; ++i)
   {
#ifndef DEBUG_MODE
      printf("       ");
#else
      printf("          ");
#endif

      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[up_side_p->stickers[i * CUBE_SIZE + j].active.values.color]);
#else
         printf("%d ", up_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf("\n");
   }
   printf("\n");

   for (int i = 0; i < CUBE_SIZE; ++i)
   {
      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[left_side_p->stickers[i * CUBE_SIZE + j].active.values.color]);
#else
         printf("%d ", left_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf(" ");

      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[front_side_p->stickers[i * CUBE_SIZE + j].active.values.color]);
#else
         printf("%d ", front_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf(" ");

      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[right_side_p->stickers[i * CUBE_SIZE + j].active.values.color]);
#else
         printf("%d ", right_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf(" ");

      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[back_side_p->stickers[i * CUBE_SIZE + j].active.values.color]);
#else
         printf("%d ", back_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }

      printf("\n");
   }
   printf("\n");

   for (int i = 0; i < CUBE_SIZE; ++i)
   {
#ifndef DEBUG_MODE
      printf("       ");
#else
      printf("          ");
#endif

      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[down_side_p->stickers[i * CUBE_SIZE + j].active.values.color]);
#else
         printf("%d ", down_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf("\n");
   }
   printf("\n");
}