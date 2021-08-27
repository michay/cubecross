#pragma once

#include "globals.h"
#include <time.h>

#define MAX_LINKED_STICKERS (2)
#define CUBE_SIZE (3)
#define MAX_ROTATION_ARRAY (100)
#define ARRAY_LENGTH (18)

typedef enum
{
   COLOR_BLUE = 0,
   COLOR_WHITE,
   COLOR_GREEN,
   COLOR_ORANGE,
   COLOR_RED,
   COLOR_YELLOW,
   COLOR_COUNT,
} CubeColor_t;

typedef enum
{
   CUBE_SIDE_BACK,
   CUBE_SIDE_UP,
   CUBE_SIDE_FRONT,
   CUBE_SIDE_LEFT,
   CUBE_SIDE_RIGHT,
   CUBE_SIDE_DOWN,
   CUBE_SIDE_COUNT = 6,
} CubeSides_t;

typedef struct
{
   unsigned char solution_depth;
   char solution_array[MAX_ROTATION_ARRAY];
} CubeRotation_t;

struct CubeSide_t;

typedef struct
{
   struct CubeSide_t* connected_side_p;
   int sticker_index;
} StickerLink_t;

typedef union
{
   struct
   {
      short color;
      short unique_index;
   } values;
   int all;
} StickerValues_t;

typedef struct
{
   StickerValues_t active;
   StickerValues_t previous;
   int timestamp;

   StickerLink_t linked_stickers[MAX_LINKED_STICKERS];
   int linked_stickers_count;

   int fixed_index;
} Sticker_t;

typedef struct CubeSide_t
{
   int color; // CubeColor_t
   Sticker_t stickers[CUBE_SIZE * CUBE_SIZE];
} CubeSide_t;

typedef struct
{
   CubeSide_t sides[CUBE_SIDE_COUNT];
   int timestamp;

   Sticker_t* fixed_stickers[CUBE_SIZE * CUBE_SIZE * CUBE_SIDE_COUNT];
   

   CubeRotation_t synced_rotation; // last synced rotation
   CubeRotation_t active_rotation;  // rotation from last synced rotation

   // X/Y/Z rotation
   int sides_hash[CUBE_SIDE_COUNT];

   // Found solutions
   time_t start_solve_timestamp;
   CubeRotation_t found_solutions[MAX_ALLOWED_SOLUTIONS];
   int num_found_solutions;
   int nodes_searched;

   // F2L
   int solved_pairs_bitmap;
} Cube_t;


void init_cube(Cube_t* cube_p, int is_white_top);
void print_cube_links(Cube_t* cube_p);
CubeSide_t* get_cube_side(Cube_t* cube_p, int cube_side);
void print_cube(Cube_t* cube_p);
void print_orientation(Cube_t* cube_p);