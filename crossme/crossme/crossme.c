#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <malloc.h>
#include <windows.h>

//#define DEBUG_MODE
#define SOLVE_WITH_THREADS

#define TRUE  1
#define FALSE 0

#define MAX_LINKED_STICKERS 2
#define CUBE_SIZE 3
#define MAX_ROTATION_ARRAY 100
#define ARRAY_LENGTH 18
#define CROSS_MAX_SEARCH_DEPTH 8

#define MAX_CROSS_SOLUTIONS_THREAD (1)

#define M_MIN(_val1_, _val2_) (((_val1_ )< (_val2_)) ? (_val1_)  : ( _val2_))
#define M_MAX(_val1_, _val2_) (((_val1_ )> (_val2_)) ? (_val1_)  : ( _val2_))

#define M_IS_PRIME_ROTATION(_rotation_) ((_rotation_) & 1)
#define M_TOGGLE_PRIME_INDICATION(_rotation_) ((_rotation_) ^= 1)
#define M_IS_REPEAT_ROTATION(_rotation_) ((_rotation_) & 2)
#define M_GET_ROTATIONS_STEP(_rotation_) ((_rotation_) >> 2)
#define M_IS_AXIS_ROTATION(_rotation_) ((_rotation_) >= ROTATE_X)
#define M_GET_AXIS_TO_ROTATE(_rotation_) (((_rotation_) - ROTATE_X) >> 2)

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
   CUBE_SIDE_BACK,
   CUBE_SIDE_UP,
   CUBE_SIDE_FRONT,
   CUBE_SIDE_LEFT,
   CUBE_SIDE_RIGHT,
   CUBE_SIDE_DOWN,
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

   ROTATE_X = 24,
   ROTATE_X_PRIME,
   ROTATE_X2,

   ROTATE_Y = 28,
   ROTATE_Y_PRIME,
   ROTATE_Y2,

   ROTATE_Z = 32,
   ROTATE_Z_PRIME,
   ROTATE_Z2

} Rotate_t;

typedef enum
{
   STICKER_TYPE_EDGE,
   STICKER_TYPE_CORNER,
   STICKER_TYPE_CENTER,
} StickerType_t;

typedef enum
{
   AXIS_X = 0,
   AXIS_Y,
   AXIS_Z,
   AXIS_COUNT
} AxisRotation_t;

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

   CubeRotation_t active_rotation;  // rotation from last synced rotation
   CubeRotation_t synced_rotation; // last synced rotation

   int sides_hash[CUBE_SIDE_COUNT];

   // Cross solutions
   int num_found_cross_solutions;
   time_t start_solve_timestamp;
   int nodes_searched;
} Cube_t;

typedef struct
{
   Cube_t cube;
   int start_rotation;
} CubeThreadedSolution_t;


static void init_cube_side(CubeSide_t* side_p, int color);
static void rotate_cube_single(Cube_t* cube_p, int rotation);
static void rotate_cube_side(Cube_t* cube_p, int side, int is_clockwise);
static void rotate_cube_string(Cube_t* cube_p, char* rotate_input_p, int do_print);
static void rotate_cube_array(Cube_t* cube_p, char* rotate_array, int start_offset, int rotations_count);
static void anti_rotate(Cube_t* cube_p, int rotation);
static void rotate_cube_axis(Cube_t* cube_p, int rotation);
static void print_cube_side(CubeSide_t* side_p);

static int is_cross_solved(Cube_t* cube_p);
static void rotate_between_rotations(Cube_t* cube_p, CubeRotation_t* rotate_array1_p, CubeRotation_t* rotate_array2_p);
static int similar_between_rotations(CubeRotation_t* rotate_array1_p, CubeRotation_t* rotate_array2_p);
static void solve_cross(Cube_t* cube_p, CubeRotation_t* base_solution_p);
static int solve_cross_max_depth(Cube_t* cube_p, CubeRotation_t* base_solution_p, int max_depth);
static void print_solution(Cube_t* cube_p, CubeRotation_t* solution_p, int is_yellow_top);
static void print_orientation(Cube_t* cube_p);

DWORD WINAPI threaded_solve_cross(LPVOID lpParam);

void init_cube(Cube_t* cube_p, int is_white_top);
static void link_up_down(CubeSide_t* up_side_p, CubeSide_t* down_side_p);
static void link_left_right(CubeSide_t* left_side_p, CubeSide_t* right_side_p);
static void link_two_stickers(Sticker_t* sticker1_p, CubeSide_t* side1_p, Sticker_t* sticker2_p, CubeSide_t* side2_p);
static void print_cube(Cube_t* cube_p);
static void print_cube_links(Cube_t* cube_p);

static CubeSide_t* get_cube_side(Cube_t* cube_p, int cube_side);
static int convert_rotation_to_side(Cube_t* cube_p, int rotation);

static void cube_assert(int condition);

Cube_t DLL_Cube;
__declspec(dllexport) void dll_init(int is_white_top);
__declspec(dllexport) void dll_rotate(char* rotate_input_p);
__declspec(dllexport) void dll_solve_cross(void);
__declspec(dllexport) void dll_print_cube(void);

#ifdef SOLVE_WITH_THREADS
CRITICAL_SECTION Critical_section;
#define ENTER_CRITICAL_SECTION() EnterCriticalSection(&Critical_section);
#define EXIT_CRITICAL_SECTION() LeaveCriticalSection(&Critical_section);
#else
#define ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()
#endif

int _tmain(int argc, _TCHAR* argv[])
{
   dll_init(TRUE);
   dll_rotate("R' F D2 L2 F2 L' D2 R U2 L2 D2 F2 R' U2 D R F' D R2 D' L'");
   dll_print_cube();
   dll_solve_cross();
}

__declspec(dllexport) void dll_init(int is_white_top)
{
   init_cube(&DLL_Cube, is_white_top);
}

__declspec(dllexport) void dll_rotate(char* rotate_input_p)
{
   rotate_cube_string(&DLL_Cube, rotate_input_p, TRUE);
}

__declspec(dllexport) void dll_solve_cross(void)
{
   int total_nodes_searched;

   printf("\ncross solutions: [");
   print_orientation(&DLL_Cube);
   printf("]:\n");

   time(&DLL_Cube.start_solve_timestamp);

#ifndef SOLVE_WITH_THREADS
   CubeRotation_t empty_solution = { 0 };
   solve_cross(&DLL_Cube, &empty_solution);
   total_nodes_searched = DLL_Cube.nodes_searched;
#else
   CubeThreadedSolution_t  thread_cubes[ARRAY_LENGTH];
   int dwThreadIdArray[ARRAY_LENGTH];
   HANDLE  thread_array[ARRAY_LENGTH];
   static char rotations_array[] = { ROTATE_B, ROTATE_B_PRIME, ROTATE_B2, ROTATE_U, ROTATE_U_PRIME, ROTATE_U2, ROTATE_F, ROTATE_F_PRIME, ROTATE_F2, ROTATE_L, ROTATE_L_PRIME, ROTATE_L2, ROTATE_R, ROTATE_R_PRIME, ROTATE_R2, ROTATE_D, ROTATE_D_PRIME, ROTATE_D2 };
   int did_init_cs;

   did_init_cs = InitializeCriticalSectionAndSpinCount(&Critical_section, 0x00000400);
   cube_assert(did_init_cs);

   for (int i = 0; i < ARRAY_LENGTH; i++)
   {
      init_cube(&thread_cubes[i].cube, TRUE);
      rotate_cube_array(&thread_cubes[i].cube, DLL_Cube.synced_rotation.solution_array, 0, DLL_Cube.synced_rotation.solution_depth);
      thread_cubes[i].start_rotation = rotations_array[i];
      time(&thread_cubes[i].cube.start_solve_timestamp);

      thread_array[i] = CreateThread(NULL, 0, threaded_solve_cross, &thread_cubes[i], 0, &dwThreadIdArray[i]);
      cube_assert(thread_array[i] != NULL);
   } 

   WaitForMultipleObjects(ARRAY_LENGTH, thread_array, TRUE, INFINITE);

   // Close all thread handles and free memory allocations.
   total_nodes_searched = 0;
   for (int i = 0; i < ARRAY_LENGTH; i++)
   {
      total_nodes_searched += thread_cubes[i].cube.nodes_searched;
      CloseHandle(thread_array[i]);
   }

   DeleteCriticalSection(&Critical_section);
#endif

   time_t total_time;
   time(&total_time);

   printf("\n%d combinations searched for %d seconds", total_nodes_searched, total_time - DLL_Cube.start_solve_timestamp);
}

DWORD WINAPI threaded_solve_cross(LPVOID lpParam)
{
   CubeThreadedSolution_t* threaded_cube_p = (CubeThreadedSolution_t*)lpParam;
   CubeRotation_t base_solution;

   base_solution.solution_array[0] = threaded_cube_p->start_rotation;
   base_solution.solution_depth = 1;

   solve_cross(&threaded_cube_p->cube, &base_solution);

   return FALSE;
}

__declspec(dllexport) void dll_print_cube(void)
{
   print_cube(&DLL_Cube);
}

static void solve_cross(Cube_t* cube_p, CubeRotation_t* base_solution_p)
{
   int did_solve;
   int max_depth = CROSS_MAX_SEARCH_DEPTH;

   for (int depth = 1; depth < max_depth; ++depth)
   {
      did_solve = solve_cross_max_depth(cube_p, base_solution_p, depth);

      if (did_solve > 0)
      {
         max_depth = M_MIN(max_depth, depth + 2);
      }

      if (cube_p->num_found_cross_solutions >= MAX_CROSS_SOLUTIONS_THREAD)
      {
         break;
      }
   }

   rotate_between_rotations(cube_p, &cube_p->active_rotation, base_solution_p);
}

static int solve_cross_max_depth(Cube_t* cube_p, CubeRotation_t* base_solution_p, int max_depth)
{
   static char rotations_array[] = { ROTATE_B, ROTATE_B_PRIME, ROTATE_B2, ROTATE_U, ROTATE_U_PRIME, ROTATE_U2, ROTATE_F, ROTATE_F_PRIME, ROTATE_F2, ROTATE_L, ROTATE_L_PRIME, ROTATE_L2, ROTATE_R, ROTATE_R_PRIME, ROTATE_R2, ROTATE_D, ROTATE_D_PRIME, ROTATE_D2 };
   int result = FALSE;
   int did_solve;
   char last_rotation;

   if (base_solution_p->solution_depth == max_depth)
   {
      rotate_between_rotations(cube_p, &cube_p->active_rotation, base_solution_p);
      memcpy(&cube_p->active_rotation, base_solution_p, sizeof(CubeRotation_t));
      cube_p->nodes_searched++;
      
      if (is_cross_solved(cube_p))
      {
         cube_p->num_found_cross_solutions++;
         print_solution(cube_p, base_solution_p, FALSE);
         return 1;
      }

      return 0;
   }

   // Get last one
   last_rotation = base_solution_p->solution_depth ? base_solution_p->solution_array[base_solution_p->solution_depth - 1] : -1;

   // Advance number of entries - only last one will be changed
   base_solution_p->solution_depth++;
   for (int i = 0; i < ARRAY_LENGTH; ++i)
   {
      // Skip same side rotations [for example -> R R']
      if (M_GET_ROTATIONS_STEP(rotations_array[i]) == M_GET_ROTATIONS_STEP(last_rotation))
         continue;

      base_solution_p->solution_array[base_solution_p->solution_depth - 1] = rotations_array[i];
      did_solve = solve_cross_max_depth(cube_p, base_solution_p, max_depth);
      result = result || did_solve;

      if (cube_p->num_found_cross_solutions >= MAX_CROSS_SOLUTIONS_THREAD)
      {
         break;
      }
   }
   base_solution_p->solution_depth--;

   return result;
}

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

   CubeSide_t* side_p;

   memset(cube_p, 0, sizeof(Cube_t));
   
   side_p = &cube_p->sides[0];
   for (i = 0; i < CUBE_SIDE_COUNT; ++i, ++side_p)
   {
      cube_p->sides_hash[i] = i;
      
      memset(side_p, 0, sizeof(CubeSide_t));
      init_cube_side(side_p, i);
   }

   if (!is_white_top)
   {
      rotate_cube_string(cube_p, "Z2", FALSE);
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
   cube_assert(sticker1_p->linked_stickers_count <= MAX_LINKED_STICKERS);

   sticker2_p->linked_stickers[sticker2_p->linked_stickers_count].connected_side_p = side1_p;
   sticker2_p->linked_stickers[sticker2_p->linked_stickers_count].sticker_index = sticker1_p - side1_p->stickers;
   sticker2_p->linked_stickers_count++;
   cube_assert(sticker2_p->linked_stickers_count <= MAX_LINKED_STICKERS);
}

static void print_cube(Cube_t* cube_p)
{
   char colors_hash[] = { 'B', 'W', 'G', 'O', 'R', 'Y' };
   CubeSide_t* back_side_p = get_cube_side(cube_p, CUBE_SIDE_BACK);
   CubeSide_t* front_side_p = get_cube_side(cube_p, CUBE_SIDE_FRONT);
   CubeSide_t* left_side_p = get_cube_side(cube_p, CUBE_SIDE_LEFT);
   CubeSide_t* up_side_p = get_cube_side(cube_p, CUBE_SIDE_UP);
   CubeSide_t* right_side_p = get_cube_side(cube_p, CUBE_SIDE_RIGHT);
   CubeSide_t* down_side_p = get_cube_side(cube_p, CUBE_SIDE_DOWN);

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

   if (M_IS_AXIS_ROTATION(rotation))
   {
      rotate_cube_axis(cube_p, rotation);
   }
   else
   {
      rotation_side = cube_p->sides_hash[rotation_side];
      for (int i = 0; i < num_repeats; ++i)
      {
         rotate_cube_side(cube_p, rotation_side, !is_anti_clockwise);
      }
   }
}

static void rotate_cube_axis(Cube_t* cube_p, int rotation)
{
   int num_repeats;
   int rotate_axis;
   int swap_array[CUBE_SIDE_COUNT];
   int prev_hash[CUBE_SIDE_COUNT];

   rotate_axis = M_GET_AXIS_TO_ROTATE(rotation);
   if (M_IS_PRIME_ROTATION(rotation))
      num_repeats = 3;
   else
      num_repeats = M_IS_REPEAT_ROTATION(rotation) ? 2 : 1;


   if (rotate_axis == AXIS_X)
   {
      swap_array[CUBE_SIDE_FRONT] = CUBE_SIDE_UP;
      swap_array[CUBE_SIDE_UP] = CUBE_SIDE_BACK;
      swap_array[CUBE_SIDE_BACK] = CUBE_SIDE_DOWN;
      swap_array[CUBE_SIDE_DOWN] = CUBE_SIDE_FRONT;

      swap_array[CUBE_SIDE_LEFT] = CUBE_SIDE_LEFT;
      swap_array[CUBE_SIDE_RIGHT] = CUBE_SIDE_RIGHT;
   }
   else if (rotate_axis == AXIS_Y)
   {
      swap_array[CUBE_SIDE_FRONT] = CUBE_SIDE_LEFT;
      swap_array[CUBE_SIDE_LEFT] = CUBE_SIDE_BACK;
      swap_array[CUBE_SIDE_BACK] = CUBE_SIDE_RIGHT;
      swap_array[CUBE_SIDE_RIGHT] = CUBE_SIDE_FRONT;

      swap_array[CUBE_SIDE_UP] = CUBE_SIDE_UP;
      swap_array[CUBE_SIDE_DOWN] = CUBE_SIDE_DOWN;
   }
   else if (rotate_axis == AXIS_Z)
   {
      swap_array[CUBE_SIDE_UP] = CUBE_SIDE_RIGHT;
      swap_array[CUBE_SIDE_RIGHT] = CUBE_SIDE_DOWN;
      swap_array[CUBE_SIDE_DOWN] = CUBE_SIDE_LEFT;
      swap_array[CUBE_SIDE_LEFT] = CUBE_SIDE_UP;

      swap_array[CUBE_SIDE_FRONT] = CUBE_SIDE_FRONT;
      swap_array[CUBE_SIDE_BACK] = CUBE_SIDE_BACK;
   }
   else
   {
      cube_assert(FALSE);
   }

   for (int r = 0; r < num_repeats; ++r)
   {
      memcpy(prev_hash, cube_p->sides_hash, sizeof(cube_p->sides_hash));
      for (int i = 0; i < CUBE_SIDE_COUNT; ++i)
      {
         if (swap_array[i] == -1)
            continue;
         
         cube_p->sides_hash[swap_array[i]] = prev_hash[i];
      }
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
      cube_assert(sticker_p->linked_stickers_count == new_pos_sticker_p->linked_stickers_count);

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

static void rotate_cube_string(Cube_t* cube_p, char* rotate_input_p, int do_print)
{
   int rotation = -1;
   int is_new_rotation = TRUE;
   char* inp_p = rotate_input_p;

   if (do_print)
   {
      printf("Rotate [");
      print_orientation(cube_p);
      printf("]:\n  %s\n\n", rotate_input_p);
      printf("[W = White; G = Green; R = Red; O = Orange; B = Blue; Y = Yellow]\n");
   }

   while (*inp_p)
   {
      if (*inp_p == ' ')
      {
         cube_assert(rotation != -1);
         rotate_cube_single(cube_p, rotation);

         cube_p->synced_rotation.solution_array[cube_p->synced_rotation.solution_depth] = rotation;
         cube_p->synced_rotation.solution_depth++;
         cube_assert(cube_p->synced_rotation.solution_depth < MAX_ROTATION_ARRAY);

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
      else if (*inp_p == 'X') rotation = ROTATE_X;
      else if (*inp_p == 'Y') rotation = ROTATE_Y;
      else if (*inp_p == 'Z') rotation = ROTATE_Z;
      else
      {
         cube_assert(FALSE);
      }

      inp_p++;
   }

   cube_assert(rotation != -1);
   rotate_cube_single(cube_p, rotation);

   cube_p->synced_rotation.solution_array[cube_p->synced_rotation.solution_depth] = rotation;
   cube_p->synced_rotation.solution_depth++;
   cube_assert(cube_p->synced_rotation.solution_depth < MAX_ROTATION_ARRAY);
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

   side_p = get_cube_side(cube_p, CUBE_SIDE_DOWN);
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
      side_p = get_cube_side(cube_p, cross_sides[i]);
      sticker_p = &side_p->stickers[1].active;
      if (sticker_p->color != side_p->color)
      {
         result = FALSE;
         break;
      }
   }

   return result;
}

static void rotate_between_rotations(Cube_t* cube_p, CubeRotation_t* rotate_array1_p, CubeRotation_t* rotate_array2_p)
{
   int common_rotations;
   int anti_count;
   int for_count;

   common_rotations = similar_between_rotations(rotate_array1_p, rotate_array2_p);
   anti_count = rotate_array1_p->solution_depth - common_rotations;
   for_count = rotate_array2_p->solution_depth - common_rotations;

   for (int i = 0; i < anti_count; ++i)
   {
      anti_rotate(cube_p, rotate_array1_p->solution_array[rotate_array1_p->solution_depth - 1 - i]);
   }

   rotate_cube_array(cube_p, rotate_array2_p->solution_array, common_rotations, for_count);
}

static int similar_between_rotations(CubeRotation_t* rotate_array1_p, CubeRotation_t* rotate_array2_p)
{
   // rotation1 is current rotation
   // will return how much anti - rotation is required for rotation2
   int loop_size;
   int i;
   char* rotate1_p;
   char* rotate2_p;

   loop_size = M_MIN(rotate_array1_p->solution_depth, rotate_array2_p->solution_depth);
   rotate1_p = &rotate_array1_p->solution_array[0];
   rotate2_p = &rotate_array2_p->solution_array[0];
   for (i = 0; i < loop_size; ++i)
   {
      if (*rotate1_p != *rotate2_p)
         break;

      rotate1_p++;
      rotate2_p++;
   }

   return i;
}

static void print_solution(Cube_t* cube_p, CubeRotation_t* solution_p, int is_yellow_top)
{
   char* rotate_p;
   int rotation_step;
   int rotation_side;
   int padding = 0;

   ENTER_CRITICAL_SECTION();

   printf("length %d; ", solution_p->solution_depth);

   rotate_p = &solution_p->solution_array[0];
   for (int i = 0; i < solution_p->solution_depth; ++i, ++rotate_p)
   {
      for (int p = 0; p < padding; ++p)
         printf(" ");

      rotation_step = M_GET_ROTATIONS_STEP(*rotate_p);
      rotation_side = convert_rotation_to_side(cube_p, rotation_step);
      
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

      padding = (M_IS_PRIME_ROTATION(*rotate_p) || M_IS_REPEAT_ROTATION(*rotate_p)) ? 1 : 2;
   }

   for (int p = 0; p < padding-1; ++p)
      printf(" ");

   printf("\n");

   EXIT_CRITICAL_SECTION();
}

static void print_orientation(Cube_t* cube_p)
{
   static char* colors_hash[] = { "Blue", "White", "Green", "Orange", "Red", "Yellow" };
   CubeSide_t* top_side_p;
   CubeSide_t* front_side_p;

   top_side_p = get_cube_side(cube_p, CUBE_SIDE_UP);
   front_side_p = get_cube_side(cube_p, CUBE_SIDE_FRONT);
   printf("%s top, %s front", colors_hash[top_side_p->color], colors_hash[front_side_p->color]);
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

static CubeSide_t* get_cube_side(Cube_t* cube_p, int cube_side)
{
   return &cube_p->sides[cube_p->sides_hash[cube_side]];
}

static int convert_rotation_to_side(Cube_t* cube_p, int rotation)
{
   //static int hashtable = {}
   return rotation;
}

static void cube_assert(int condition)
{
   if (!condition)
   {
      printf("Error!!!");
   }

   assert(condition);
}