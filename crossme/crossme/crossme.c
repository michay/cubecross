#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <malloc.h>


//#define DEBUG_MODE

#define TRUE  1
#define FALSE 0

#define MAX_LINKED_STICKERS 2
#define CUBE_SIZE 3
#define MAX_DEPTH 8
#define ARRAY_LENGTH 18
#define MAX_QUEUE_DEPTH (18 * 18 * 18 * 18 * 18 * 18 * 18)

#define M_MIN(_val1_, _val2_) (((_val1_ )< (_val2_)) ? (_val1_)  : ( _val2_))
#define M_MAX(_val1_, _val2_) (((_val1_ )> (_val2_)) ? (_val1_)  : ( _val2_))

#define M_IS_PRIME_ROTATION(_rotation_) ((_rotation_) & 1)
#define M_TOGGLE_PRIME_INDICATION(_rotation_) ((_rotation_) ^= 1)
#define M_IS_REPEAT_ROTATION(_rotation_) ((_rotation_) & 2)
#define M_GET_ROTATIONS_SIDE(_rotation_) (_rotation_ >> 2)

typedef enum
{
   COLOR_BLUE = 0,
   COLOR_WHITE,
   COLOR_GREEN,
   COLOR_ORANGE ,
   COLOR_RED,
   COLOR_YELLOW,
   COLOR_COUNT,
} CubeColor_t;

typedef enum
{
   CUBE_SIDE_BACK = COLOR_BLUE,
   CUBE_SIDE_UP = COLOR_WHITE,
   CUBE_SIDE_FRONT = COLOR_GREEN,
   CUBE_SIDE_LEFT = COLOR_ORANGE,
   CUBE_SIDE_RIGHT = COLOR_RED,
   CUBE_SIDE_DOWN = COLOR_YELLOW,
   CUBE_SIDE_COUNT = 6,
} CubeSides_t;

typedef enum
{
   ROTATE_B = 0,
   ROTATE_B_PRIME,
   ROTATE_B2,

   ROTATE_U = 4,
   ROTATE_U_PRIME,
   ROTATE_U2,

   ROTATE_F = 8,
   ROTATE_F_PRIME,
   ROTATE_F2,

   ROTATE_L = 12,
   ROTATE_L_PRIME,
   ROTATE_L2,

   ROTATE_R = 16,
   ROTATE_R_PRIME,
   ROTATE_R2,

   ROTATE_D = 20,
   ROTATE_D_PRIME,
   ROTATE_D2,

} Rotate_t;

typedef enum
{
   STICKER_TYPE_EDGE,
   STICKER_TYPE_CORNER,
   STICKER_TYPE_CENTER,
} StickerType_t;

typedef struct
{
   unsigned char solutionDepth;
   char solutionArray[MAX_DEPTH];
} CrossSolution_t;

typedef struct
{
   CrossSolution_t* entries_p;
   int head;
   int tail;
   int count;
} Queue_t;

struct CubeSide_t;

typedef struct
{
   struct CubeSide_t* connected_side_p;
   int sticker_index;
} StickerLink_t;

typedef struct
{
   int color;  // CubeColor_t
   int unique_index;
} StickerValues_t;

typedef struct 
{
   StickerValues_t active;
   StickerValues_t previous;
   int timestamp;

   int sticker_type;  //StickerType_t
   StickerLink_t linked_stickers[MAX_LINKED_STICKERS];
   int linked_stickers_count;
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
   time_t start_solve_timestamp;
} Cube_t;


static CrossSolution_t* enqueue(Queue_t* queue_p);
static CrossSolution_t* dequeue(Queue_t* queue_p);

static void init_cube_side(CubeSide_t* side_p, int color);
static void rotate_cube_single(Cube_t* cube_p, int rotation);
static void rotate_cube_side(Cube_t* cube_p, int side, int is_clockwise);
static void rotate_cube_string(Cube_t* cube_p, char* rotate_input_p);
static void rotate_cube_array(Cube_t* cube_p, char* rotate_array, int start_offset, int rotations_count);
static void anti_rotate(Cube_t* cube_p, int rotation);
static void print_cube_side(CubeSide_t* side_p);

static int is_cross_solved(Cube_t* cube_p);
static void rotate_between_rotations(Cube_t* cube_p, CrossSolution_t* rotate_array1_p, CrossSolution_t* rotate_array2_p);
static int similar_between_rotations(CrossSolution_t* rotate_array1_p, CrossSolution_t* rotate_array2_p);
static void print_solution(Cube_t* cube_p, CrossSolution_t* solution_p, int is_yellow_top);

static void init_cube(Cube_t* cube_p);
static void link_up_down(CubeSide_t* up_side_p, CubeSide_t* down_side_p);
static void link_left_right(CubeSide_t* left_side_p, CubeSide_t* right_side_p);
static void link_two_stickers(Sticker_t* sticker1_p, CubeSide_t* side1_p, Sticker_t* sticker2_p, CubeSide_t* side2_p);
static void print_cube(Cube_t* cube_p);
static void print_cube_links(Cube_t* cube_p);

int _tmain(int argc, _TCHAR* argv[])
{
   Queue_t queue;
   Cube_t cube;
   CrossSolution_t emptySolution;
   CrossSolution_t* prev_solution_p;
   CrossSolution_t* new_solution_p;
   CrossSolution_t* current_solution_p;
   char rotations_array[ARRAY_LENGTH];
   int max_allowed_depth;
   int i;
   int nodes_searched = 0;
   
   init_cube(&cube);

   rotate_cube_string(&cube, "U2 F B' U R2 B' L' D2 F R2 B D2 F' L2 D2 B2 R2 D2 F' U' B");

   print_cube(&cube);

   // BFS for solve

   // Init queue
   queue.head = 0;
   queue.tail = 0;
   queue.count = 0;
   queue.entries_p = (int*)malloc(sizeof(CrossSolution_t)*MAX_QUEUE_DEPTH);
   assert(queue.entries_p != NULL);

   for (i = 0; i < CUBE_SIDE_COUNT; ++i)
   {
      rotations_array[i * 3] = i * 4;
      rotations_array[i * 3 + 1] = i * 4 + 1;
      rotations_array[i * 3 + 2] = i * 4 + 2;
   }

   for (i = 0; i < ARRAY_LENGTH; ++i)
   {
      new_solution_p = enqueue(&queue);
      new_solution_p->solutionDepth = 1;
      new_solution_p->solutionArray[0] = rotations_array[i];
   }

   printf("\ncross solutions after Z2: [Yellow top, Green front]:\n");
   time(&cube.start_solve_timestamp);
   emptySolution.solutionDepth = 0;
   prev_solution_p = &emptySolution;
   max_allowed_depth = MAX_DEPTH;
   while (queue.count > 0)
   {
      current_solution_p = dequeue(&queue);
      rotate_between_rotations(&cube, prev_solution_p, current_solution_p);
      if (is_cross_solved(&cube))
      {
         print_solution(&cube, current_solution_p, TRUE);
         max_allowed_depth = M_MIN(max_allowed_depth, current_solution_p->solutionDepth + 1);
      }
      prev_solution_p = current_solution_p;
      nodes_searched++;

      if (current_solution_p->solutionDepth < max_allowed_depth - 1)
      {
         for (i = 0; i < ARRAY_LENGTH; ++i)
         {
            // Skip same side rotations [for example -> R R']
            if (M_GET_ROTATIONS_SIDE(rotations_array[i]) == M_GET_ROTATIONS_SIDE(current_solution_p->solutionArray[current_solution_p->solutionDepth - 1]))
               continue;

            new_solution_p = enqueue(&queue);
            
            memcpy(new_solution_p, current_solution_p, sizeof(CrossSolution_t));
            new_solution_p->solutionArray[new_solution_p->solutionDepth] = rotations_array[i];
            new_solution_p->solutionDepth++;
         }
      }
   }

   time_t total_time;
   time(&total_time);
   printf("combinations searched = %d; total time = %d seconds", nodes_searched, total_time - cube.start_solve_timestamp);
}

static void init_cube(Cube_t* cube_p)
{
   CubeSide_t* back_side_p = &cube_p->sides[CUBE_SIDE_BACK];
   CubeSide_t* front_side_p = &cube_p->sides[CUBE_SIDE_FRONT];
   CubeSide_t* left_side_p = &cube_p->sides[CUBE_SIDE_LEFT];
   CubeSide_t* up_side_p = &cube_p->sides[CUBE_SIDE_UP];
   CubeSide_t* right_side_p = &cube_p->sides[CUBE_SIDE_RIGHT];
   CubeSide_t* down_side_p = &cube_p->sides[CUBE_SIDE_DOWN];
   Sticker_t* usticker_p;
   Sticker_t* bsticker_p;
   Sticker_t* lsticker_p;
   Sticker_t* rsticker_p;
   Sticker_t* dsticker_p;
   int i;

   CubeSide_t* side_p;

   memset(cube_p, 0, sizeof(Cube_t));
   
   side_p = &cube_p->sides;
   for (i = 0; i < CUBE_SIDE_COUNT; ++i, ++side_p)
   {
      memset(side_p, 0, sizeof(CubeSide_t));
      init_cube_side(side_p, i);
   }

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
      usticker_p = &up_side_p->stickers[0 + i*3];
      link_two_stickers(lsticker_p, left_side_p, usticker_p, up_side_p);

      usticker_p = &up_side_p->stickers[2 + i * 3];
      rsticker_p = &right_side_p->stickers[2 - i];
      link_two_stickers(usticker_p, up_side_p, rsticker_p, right_side_p);

      rsticker_p = &right_side_p->stickers[6 + i];
      dsticker_p = &down_side_p->stickers[2 + i*3];
      link_two_stickers(rsticker_p, right_side_p, dsticker_p, down_side_p);

      dsticker_p = &down_side_p->stickers[0 + i * 3];
      lsticker_p = &left_side_p->stickers[8 - i];
      link_two_stickers(dsticker_p, down_side_p, lsticker_p, left_side_p);
   }
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

static void link_two_stickers(Sticker_t* sticker1_p, CubeSide_t* side1_p, Sticker_t* sticker2_p, CubeSide_t* side2_p)
{
   sticker1_p->linked_stickers[sticker1_p->linked_stickers_count].connected_side_p = side2_p;
   sticker1_p->linked_stickers[sticker1_p->linked_stickers_count].sticker_index = sticker2_p - side2_p->stickers;
   sticker1_p->linked_stickers_count++;
   assert(sticker1_p->linked_stickers_count <= MAX_LINKED_STICKERS);

   sticker2_p->linked_stickers[sticker2_p->linked_stickers_count].connected_side_p = side1_p;
   sticker2_p->linked_stickers[sticker2_p->linked_stickers_count].sticker_index = sticker1_p - side1_p->stickers;
   sticker2_p->linked_stickers_count++;
   assert(sticker2_p->linked_stickers_count <= MAX_LINKED_STICKERS);
}

static void print_cube(Cube_t* cube_p)
{
   char colors_hash[] = { 'B', 'W', 'G', 'O', 'R', 'Y' };
   CubeSide_t* back_side_p = &cube_p->sides[CUBE_SIDE_BACK];
   CubeSide_t* front_side_p = &cube_p->sides[CUBE_SIDE_FRONT];
   CubeSide_t* left_side_p = &cube_p->sides[CUBE_SIDE_LEFT];
   CubeSide_t* up_side_p = &cube_p->sides[CUBE_SIDE_UP];
   CubeSide_t* right_side_p = &cube_p->sides[CUBE_SIDE_RIGHT];
   CubeSide_t* down_side_p = &cube_p->sides[CUBE_SIDE_DOWN];

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
         printf("%c ", colors_hash[up_side_p->stickers[i * CUBE_SIZE + j].active.color]);
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
         printf("%c ", colors_hash[left_side_p->stickers[i * CUBE_SIZE + j].active.color]);
#else
         printf("%d ", left_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf(" ");
      
      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[front_side_p->stickers[i * CUBE_SIZE + j].active.color]);
#else
         printf("%d ", front_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf(" ");

      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[right_side_p->stickers[i * CUBE_SIZE + j].active.color]);
#else
         printf("%d ", right_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf(" ");

      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[back_side_p->stickers[i * CUBE_SIZE + j].active.color]);
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
         printf("%c ", colors_hash[down_side_p->stickers[i * CUBE_SIZE + j].active.color]);
#else
         printf("%d ", down_side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf("\n");
   }
   printf("\n");
}

static void print_cube_links(Cube_t* cube_p)
{
   for (int i = 0; i < CUBE_SIDE_COUNT; ++i)
   {
      for (int j = 0; j < CUBE_SIZE*CUBE_SIZE; ++j)
      {
         Sticker_t* sticker_p = &cube_p->sides[i].stickers[j];
         printf("%d: ", sticker_p->active.unique_index);
         
         for (int k = 0; k < sticker_p->linked_stickers_count; ++k)
            printf("%d, ", sticker_p->linked_stickers[k].connected_side_p->stickers[sticker_p->linked_stickers[k].sticker_index].active.unique_index);

         printf("\n");
      }
      printf("\n");
   }
}

static void init_cube_side(CubeSide_t* side_p, int color)
{
   Sticker_t* sticker_p;

   side_p->color = color;
   
   sticker_p = side_p->stickers;
   for (int i = 0; i < CUBE_SIZE * CUBE_SIZE; ++i, ++sticker_p)
   {
      sticker_p->active.color = color;
      sticker_p->active.unique_index = (color + 1) * 10 + i;

      if (i == 4)
         sticker_p->sticker_type = STICKER_TYPE_CENTER;
      else if (i %2 == 0)
         sticker_p->sticker_type = STICKER_TYPE_CORNER;
      else 
         sticker_p->sticker_type = STICKER_TYPE_EDGE;
   }
}

static void rotate_cube_single(Cube_t* cube_p, int rotation)
{
   int rotation_side;
   int is_anti_clockwise;
   int num_repeats;

   rotation_side = (rotation >> 2);
   is_anti_clockwise = M_IS_PRIME_ROTATION(rotation);
   num_repeats = M_IS_REPEAT_ROTATION(rotation) ? 2 : 1;

   for (int i = 0; i < num_repeats; ++i)
   {
      rotate_cube_side(cube_p, rotation_side, !is_anti_clockwise);
   }
}

static void rotate_cube_side(Cube_t* cube_p, int side, int is_clockwise)
{
   CubeSide_t* side_p = &cube_p->sides[side];
   int clowise_position_array[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };
   int anti_clowise_position_array[] = { 2, 5, 8, 1, 4, 7, 0, 3, 6 };
   int* new_indices_array_p;
   int* inverse_indices_array_p;
   Sticker_t prev_stickers[CUBE_SIZE * CUBE_SIZE];
   Sticker_t* sticker_p;
   Sticker_t* new_pos_sticker_p;

   // select array to use
   new_indices_array_p = is_clockwise ? clowise_position_array : anti_clowise_position_array;
   inverse_indices_array_p = is_clockwise ? anti_clowise_position_array : clowise_position_array;

   // Copy previous stickers
   memcpy(&prev_stickers, &side_p->stickers, sizeof(prev_stickers));

   // Setup new stickers
   for (int i = 0; i < CUBE_SIZE * CUBE_SIZE; ++i)
   {
      int copy_index = new_indices_array_p[i];
      memcpy(&side_p->stickers[i], &prev_stickers[copy_index], sizeof(Sticker_t));
   }

   // Set timestamp for copy
   cube_p->timestamp++;

   // Rotate linked stickers
   for (int i = 0; i < CUBE_SIZE * CUBE_SIZE; ++i)
   {
      sticker_p = &side_p->stickers[i];
      new_pos_sticker_p = &side_p->stickers[inverse_indices_array_p[i]];
      assert(sticker_p->linked_stickers_count == new_pos_sticker_p->linked_stickers_count);

      for (int j = 0; j < sticker_p->linked_stickers_count; ++j)
      {
         StickerLink_t* prev_link_p = &sticker_p->linked_stickers[sticker_p->linked_stickers_count - j - 1];
         StickerLink_t* new_link_p = &new_pos_sticker_p->linked_stickers[j];
         StickerValues_t* new_sticker_value_p;

         // Save previous value
         memcpy(&new_link_p->connected_side_p->stickers[new_link_p->sticker_index].previous, &new_link_p->connected_side_p->stickers[new_link_p->sticker_index].active, sizeof(StickerValues_t));

         if (prev_link_p->connected_side_p->stickers[prev_link_p->sticker_index].timestamp == cube_p->timestamp)
            new_sticker_value_p = &prev_link_p->connected_side_p->stickers[prev_link_p->sticker_index].previous;
         else
            new_sticker_value_p = &prev_link_p->connected_side_p->stickers[prev_link_p->sticker_index].active;

         // Copy new value
         memcpy(&new_link_p->connected_side_p->stickers[new_link_p->sticker_index].active, new_sticker_value_p, sizeof(StickerValues_t));
         new_link_p->connected_side_p->stickers[new_link_p->sticker_index].timestamp = cube_p->timestamp;
      }
   }
}

static void rotate_cube_string(Cube_t* cube_p, char* rotate_input_p)
{
   int rotation = -1;
   int is_new_rotation = TRUE;
   char* inp_p = rotate_input_p;

   printf("Rotate [White top, Green front]: %s\n\n", rotate_input_p);
   printf("[W = White; G = Green; R = Red; O = Orange; B = Blue; Y = Yellow]\n");

   while (*inp_p)
   {
      if (*inp_p == ' ')
      {
         assert(rotation != -1);
         rotate_cube_single(cube_p, rotation);
         
         is_new_rotation = TRUE;
         rotation = -1;
      }
      else if (*inp_p == '\'')
         rotation += 1;
      else if (*inp_p == '2')
         rotation += 2;
      else if (*inp_p == 'R') rotation = ROTATE_R;
      else if (*inp_p == 'L') rotation = ROTATE_L;
      else if (*inp_p == 'U') rotation = ROTATE_U;
      else if (*inp_p == 'F') rotation = ROTATE_F;
      else if (*inp_p == 'D') rotation = ROTATE_D;
      else if (*inp_p == 'B') rotation = ROTATE_B;
      else
      {
         assert(FALSE);
      }

      inp_p++;
   }

   assert(rotation != -1);
   rotate_cube_single(cube_p, rotation);
}

static void rotate_cube_array(Cube_t* cube_p, char* rotate_array, int start_offset, int rotations_count)
{
   char* rotate_p;

   rotate_p = rotate_array + start_offset;
   while (rotations_count)
   {
      rotate_cube_single(cube_p, *rotate_p);
      rotations_count--;
      rotate_p++;
   }
}

static void anti_rotate(Cube_t* cube_p, int rotation)
{
   int anti_rotation;

   anti_rotation = rotation;

   if (!M_IS_REPEAT_ROTATION(rotation))
   {
      M_TOGGLE_PRIME_INDICATION(anti_rotation);
   }

   rotate_cube_single(cube_p, anti_rotation);
}

static int is_cross_solved(Cube_t* cube_p)
{
   int result = TRUE;
   static int cross_indices[] = { 1, 3, 5, 7 };
   int cross_sides[] = { CUBE_SIDE_LEFT, CUBE_SIDE_RIGHT, CUBE_SIDE_FRONT, CUBE_SIDE_BACK };
   CubeSide_t* side_p;
   StickerValues_t* sticker_p;

   side_p = &cube_p->sides[CUBE_SIDE_UP];
   for (int i = 0; i < 4; ++i)
   {
      sticker_p = &side_p->stickers[cross_indices[i]].active;
      if (sticker_p->color != side_p->color)
      {
         result = FALSE;
         break;
      }
   }

   for (int i = 0; i < 4; ++i)
   {
      side_p = &cube_p->sides[cross_sides[i]];
      sticker_p = &side_p->stickers[1].active;
      if (sticker_p->color != side_p->color)
      {
         result = FALSE;
         break;
      }
   }

   return result;
}

static void rotate_between_rotations(Cube_t* cube_p, CrossSolution_t* rotate_array1_p, CrossSolution_t* rotate_array2_p)
{
   int common_rotations;
   int anti_count;
   int for_count;

   common_rotations = similar_between_rotations(rotate_array1_p, rotate_array2_p);
   anti_count = rotate_array1_p->solutionDepth - common_rotations;
   for_count = rotate_array2_p->solutionDepth - common_rotations;

   for (int i = 0; i < anti_count; ++i)
   {
      anti_rotate(cube_p, rotate_array1_p->solutionArray[rotate_array1_p->solutionDepth - 1 - i]);
   }

   rotate_cube_array(cube_p, rotate_array2_p->solutionArray, common_rotations, for_count);
}

static int similar_between_rotations(CrossSolution_t* rotate_array1_p, CrossSolution_t* rotate_array2_p)
{
   // rotation1 is current rotation
   // will return how much anti - rotation is required for rotation2
   int loop_size;
   int i;
   char* rotate1_p;
   char* rotate2_p;

   loop_size = M_MIN(rotate_array1_p->solutionDepth, rotate_array2_p->solutionDepth);
   rotate1_p = &rotate_array1_p->solutionArray[0];
   rotate2_p = &rotate_array2_p->solutionArray[0];
   for (i = 0; i < loop_size; ++i)
   {
      if (*rotate1_p != *rotate2_p)
         break;

      rotate1_p++;
      rotate2_p++;
   }

   return i;
}

static void print_solution(Cube_t* cube_p, CrossSolution_t* solution_p, int is_yellow_top)
{
   char* rotate_p;
   int rotation_side;

   printf("   length %d; ", solution_p->solutionDepth);
   rotate_p = &solution_p->solutionArray[0];
   for (int i = 0; i < solution_p->solutionDepth; ++i, ++rotate_p)
   {
      rotation_side = M_GET_ROTATIONS_SIDE(*rotate_p);
      
      if (rotation_side == CUBE_SIDE_BACK) printf("B");
      else if (rotation_side == CUBE_SIDE_FRONT) printf("F");
      else if (rotation_side == CUBE_SIDE_UP) 
      {
         if (!is_yellow_top) printf("U");
         else printf("D");
      }
      else if (rotation_side == CUBE_SIDE_DOWN) 
      {
         if (!is_yellow_top) printf("D");
         else printf("U");
      }
      else if (rotation_side == CUBE_SIDE_LEFT)
      {
         if (!is_yellow_top) printf("L");
         else printf("R");
      }
      else if (rotation_side == CUBE_SIDE_RIGHT) 
      {
         if (!is_yellow_top) printf("R");
         else printf("L");
      }

      if (M_IS_PRIME_ROTATION(*rotate_p)) printf("'");
      else if (M_IS_REPEAT_ROTATION(*rotate_p)) printf("2");

      printf(" ");
   }

   time_t current_time;
   time(&current_time);
   printf("[%d seconds]\n", current_time - cube_p->start_solve_timestamp);
}

static void print_cube_side(CubeSide_t* side_p)
{
   char colors_hash[] = { 'B', 'W', 'G', 'O', 'R', 'Y' };
   for (int i = 0; i < CUBE_SIZE; ++i)
   {
      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[side_p->stickers[i * CUBE_SIZE + j].active.color]);
#else
         printf("%d ", side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf("\n");
   }
   printf("\n");
}

static CrossSolution_t* enqueue(Queue_t* queue_p)
{
   CrossSolution_t* new_entry_p;

   new_entry_p = &queue_p->entries_p[queue_p->head];

   queue_p->head++;
   if (queue_p->head == MAX_QUEUE_DEPTH)
      queue_p->head = 0;


   queue_p->count++;
   assert(queue_p->count <= MAX_QUEUE_DEPTH);

   return new_entry_p;
}

static CrossSolution_t* dequeue(Queue_t* queue_p)
{
   CrossSolution_t* old_entry_p;

   old_entry_p = &queue_p->entries_p[queue_p->tail];
   
   queue_p->tail++;
   if (queue_p->tail == MAX_QUEUE_DEPTH)
      queue_p->tail = 0;

   assert(queue_p->count > 0);
   queue_p->count--;

   return old_entry_p;
}
