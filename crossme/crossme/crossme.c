#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <malloc.h>
#include <windows.h>
#include "globals.h"
#include "cube.h"
#include "rotation.h"

//#define DEBUG_MODE
#define SOLVE_WITH_THREADS

#define CUBE_INIT_YELLOW_TOP (FALSE)
#define CUBE_INIT_WHITE_TOP (TRUE)

#define M_ARE_ALL_PAIRS_SOLVED(_cube_p_) (((_cube_p_)->solved_pairs_bitmap) == ((1 << (PAIR_CORNER_COUNT)) - 1))

typedef enum
{
   PAIR_CORNER_BACK_LEFT = 0,
   PAIR_CORNER_BACK_RIGHT,
   PAIR_CORNER_FRONT_LEFT,
   PAIR_CORNER_FRONT_RIGHT,
   PAIR_CORNER_COUNT
} PairCorner_t;

typedef struct
{
   Cube_t cube;
   int start_rotation;
   int (*is_solved)(Cube_t*);
} CubeThreadedSolution_t;

typedef struct
{
   int max_solve_depth;

#ifdef SOLVE_WITH_THREADS
   CRITICAL_SECTION critical_section;
   int is_critical_scrtion_active;
#endif
} ThreadedSolve_t;

// Cube rotation

// Solving in general
static void rotate_between_rotations(Cube_t* cube_p, CubeRotation_t* rotate_array1_p, CubeRotation_t* rotate_array2_p);
static int similar_between_rotations(CubeRotation_t* rotate_array1_p, CubeRotation_t* rotate_array2_p);
static void print_solution(Cube_t* cube_p, CubeRotation_t* solution_p, int add_line_break);

// Solving framework
static void solve_cfop_part(Cube_t* cube_p, CubeRotation_t* base_solution_p, int(*is_solved)(Cube_t*));
static int solve_with_max_depth(Cube_t* cube_p, CubeRotation_t* base_solution_p, int max_depth, int(*is_solved)(Cube_t*));
static void solve_with_threads(int(*is_solved)(Cube_t*));
static int find_shortest_soluction(Cube_t* cube_p);

// Cross
static int is_cross_solved(Cube_t* cube_p);
DWORD WINAPI threaded_solve_part(LPVOID lpParam);
DWORD WINAPI threaded_solve_f2l(LPVOID lpParam);

// F2L
static int map_solved_pairs(Cube_t* cube_p);
static int are_more_pairs_solved(Cube_t* cube_p);
static int is_pair_solved(Cube_t* cube_p, int pair_corner);
static void print_solved_pairs(Cube_t* cube_p, int solved_bitmap);

ThreadedSolve_t Threaded_Solve;

Cube_t DLL_Cube;
__declspec(dllexport) void dll_init(int is_white_top);
__declspec(dllexport) void dll_rotate(char* rotate_input_p);
__declspec(dllexport) void dll_solve_cross(void);
__declspec(dllexport) void dll_solve_f2l(void);
__declspec(dllexport) void dll_print_cube(void);

#ifdef SOLVE_WITH_THREADS
#define ENTER_CRITICAL_SECTION() if (Threaded_Solve.is_critical_scrtion_active) {EnterCriticalSection(&Threaded_Solve.critical_section);}
#define EXIT_CRITICAL_SECTION() if (Threaded_Solve.is_critical_scrtion_active) {LeaveCriticalSection(&Threaded_Solve.critical_section);}
#else
#define ENTER_CRITICAL_SECTION()
#define EXIT_CRITICAL_SECTION()
#endif

int _tmain(int argc, _TCHAR* argv[])
{
   //time_t t1, t2;
   dll_init(CUBE_INIT_YELLOW_TOP);
   
   dll_rotate("D F' R2 F2 D2 R2 B2 D' L2 U2 L' F2 L B D2 L' D' B2");
   //dll_rotate("B");
   dll_print_cube();

   //dll_rotate("R  F  D  L  F  R");
   //printf("%d", is_cross_solved(&DLL_Cube));

   //dll_print_cube();
   dll_solve_cross();
   //dll_solve_f2l();
   //dll_print_cube();
}

__declspec(dllexport) void dll_init(int is_white_top)
{
   init_cube(&DLL_Cube, is_white_top);
}

__declspec(dllexport) void dll_rotate(char* rotate_input_p)
{
   rotate_cube_string(&DLL_Cube, rotate_input_p, TRUE, TRUE);
}

__declspec(dllexport) void dll_solve_cross(void)
{
   int selected_solution;
   int apply_first_one = TRUE;

   if (is_cross_solved(&DLL_Cube))
   {
      printf("Cross already solved\n");
      return;
   }

   printf("\ncross solutions: [");
   print_orientation(&DLL_Cube);
   printf("]:\n");

   time(&DLL_Cube.start_solve_timestamp);
   DLL_Cube.nodes_searched = 0;

#ifndef SOLVE_WITH_THREADS
   CubeRotation_t empty_solution = { 0 };
   solve_cfop_part(&DLL_Cube, &empty_solution, is_cross_solved);
#else
   solve_with_threads(is_cross_solved);
#endif

   if (apply_first_one)
   {
      cube_assert(DLL_Cube.num_found_solutions > 0);
      selected_solution = find_shortest_soluction(&DLL_Cube);
      print_solution(&DLL_Cube, &DLL_Cube.found_solutions[selected_solution], TRUE);
      rotate_cube_array(&DLL_Cube, DLL_Cube.found_solutions[selected_solution].solution_array, 0, DLL_Cube.found_solutions[selected_solution].solution_depth, TRUE);
   }

   time_t total_time;
   time(&total_time);
   printf("\n%d combinations searched for %d seconds", DLL_Cube.nodes_searched, total_time - DLL_Cube.start_solve_timestamp);
}

__declspec(dllexport) void dll_solve_f2l(void)
{
   int total_nodes_searched = 0;
   int selected_solution;
   int prev_solved_pairs;

   printf("\n\nF2L solutions: [");
   print_orientation(&DLL_Cube);
   printf("]:\n");

   DLL_Cube.solved_pairs_bitmap = map_solved_pairs(&DLL_Cube);
   if (M_ARE_ALL_PAIRS_SOLVED(&DLL_Cube))
   {
      printf("F2L already solved");
   }

   time(&DLL_Cube.start_solve_timestamp);

   CubeRotation_t empty_solution = { 0 };

   while (!M_ARE_ALL_PAIRS_SOLVED(&DLL_Cube))
   {
      printf("Solving pair:\n");

      // Find solutions
#ifndef SOLVE_WITH_THREADS
      empty_solution.solution_depth = 0;
      solve_cfop_part(&DLL_Cube, &empty_solution, are_more_pairs_solved);
#else
      solve_with_threads(are_more_pairs_solved);
#endif

      // Apply
      cube_assert(DLL_Cube.num_found_solutions > 0);
      selected_solution = find_shortest_soluction(&DLL_Cube);

      rotate_cube_array(&DLL_Cube, DLL_Cube.found_solutions[selected_solution].solution_array, 0, DLL_Cube.found_solutions[selected_solution].solution_depth, TRUE);
      prev_solved_pairs = DLL_Cube.solved_pairs_bitmap;
      DLL_Cube.solved_pairs_bitmap = map_solved_pairs(&DLL_Cube);

      print_solution(&DLL_Cube, &DLL_Cube.found_solutions[selected_solution], FALSE);
      print_solved_pairs(&DLL_Cube, DLL_Cube.solved_pairs_bitmap ^ prev_solved_pairs);
      printf("\n\n");

      total_nodes_searched += DLL_Cube.nodes_searched;
   }

   time_t total_time;
   time(&total_time);

   printf("\n%d combinations searched for %d seconds", total_nodes_searched, total_time - DLL_Cube.start_solve_timestamp);
}

static void solve_with_threads(int(*is_solved)(Cube_t*))
{
#ifdef SOLVE_WITH_THREADS
   CubeThreadedSolution_t  thread_cubes[ARRAY_LENGTH];
   int dwThreadIdArray[ARRAY_LENGTH];
   HANDLE  thread_array[ARRAY_LENGTH];
   static char rotations_array[] = { ROTATE_B, ROTATE_B_PRIME, ROTATE_B2, ROTATE_U, ROTATE_U_PRIME, ROTATE_U2, ROTATE_F, ROTATE_F_PRIME, ROTATE_F2, ROTATE_L, ROTATE_L_PRIME, ROTATE_L2, ROTATE_R, ROTATE_R_PRIME, ROTATE_R2, ROTATE_D, ROTATE_D_PRIME, ROTATE_D2 };
   int did_init_cs;

   did_init_cs = InitializeCriticalSectionAndSpinCount(&Threaded_Solve.critical_section, 0x00000400);
   Threaded_Solve.is_critical_scrtion_active = TRUE;
   cube_assert(did_init_cs);

   for (int i = 0; i < ARRAY_LENGTH; i++)
   {
      init_cube(&thread_cubes[i].cube, TRUE);
      rotate_cube_array(&thread_cubes[i].cube, DLL_Cube.synced_rotation.solution_array, 0, DLL_Cube.synced_rotation.solution_depth, FALSE);
      thread_cubes[i].cube.solved_pairs_bitmap = map_solved_pairs(&thread_cubes[i].cube);
      cube_assert(thread_cubes[i].cube.solved_pairs_bitmap == DLL_Cube.solved_pairs_bitmap);
      thread_cubes[i].start_rotation = rotations_array[i];
      thread_cubes[i].is_solved = is_solved;
      time(&thread_cubes[i].cube.start_solve_timestamp);

      thread_array[i] = CreateThread(NULL, 0, threaded_solve_part, &thread_cubes[i], 0, &dwThreadIdArray[i]);
      cube_assert(thread_array[i] != NULL);
   }

   WaitForMultipleObjects(ARRAY_LENGTH, thread_array, TRUE, INFINITE);

   // Close all thread handles and free memory allocations.
   DLL_Cube.nodes_searched = 0;
   DLL_Cube.num_found_solutions = 0;
   for (int i = 0; i < ARRAY_LENGTH; i++)
   {
      DLL_Cube.nodes_searched += thread_cubes[i].cube.nodes_searched;

      if (thread_cubes[i].cube.num_found_solutions > 0)
      {
         memcpy(&DLL_Cube.found_solutions[DLL_Cube.num_found_solutions], &thread_cubes[i].cube.found_solutions[0], sizeof(CubeRotation_t));
         DLL_Cube.num_found_solutions++;
         cube_assert(DLL_Cube.num_found_solutions <= MAX_ALLOWED_SOLUTIONS);
      }

      CloseHandle(thread_array[i]);
   }

   Threaded_Solve.is_critical_scrtion_active = FALSE;
   DeleteCriticalSection(&Threaded_Solve.critical_section);
#endif
}

static int find_shortest_soluction(Cube_t* cube_p)
{
   int result = -1;
   int min_depth = CROSS_MAX_SEARCH_DEPTH + 1;
   CubeRotation_t* solution_p;

   solution_p = &cube_p->found_solutions[0];
   for (int i = 0; i < cube_p->num_found_solutions; ++i, ++solution_p)
   {
      if (solution_p->solution_depth < min_depth)
      {
         min_depth = solution_p->solution_depth;
         result = i;
      }
   }

   cube_assert(result != -1);
   return result;
}

static int map_solved_pairs(Cube_t* cube_p)
{
   int result = 0;

   for (int i = 0; i < PAIR_CORNER_COUNT; ++i)
   {
      if (is_pair_solved(cube_p, i))
      {
         result |= (1 << i);
      }
   }

   return result;
}

static int are_more_pairs_solved(Cube_t* cube_p)
{
   int current_solved_pairs;
   int prev_solved_pairs;
   int are_prev_still_solved;
   int are_new_pairs_solved;

   if (!is_cross_solved(cube_p))
   {
      return FALSE;
   }

   prev_solved_pairs = cube_p->solved_pairs_bitmap;
   current_solved_pairs = map_solved_pairs(cube_p);

   are_prev_still_solved = (prev_solved_pairs & current_solved_pairs) == prev_solved_pairs;
   are_new_pairs_solved = prev_solved_pairs != current_solved_pairs;

   return are_prev_still_solved && are_new_pairs_solved;
}

static int is_pair_solved(Cube_t* cube_p, int pair_corner)
{
   int result = TRUE;
   static int pairs_hash[] = { 0, 2, 6, 8 };
   static int paired_couple = 0;
   CubeSide_t* side_p;
   Sticker_t* sticker_p;
   StickerLink_t* linked_sticker_p;
   CubeSide_t* linked_side_p;

   side_p = get_cube_side(cube_p, CUBE_SIDE_DOWN);
   if (side_p->color == COLOR_WHITE)
   {
      paired_couple = 3;
   }
   else
   {
      cube_assert(FALSE);
   }

   sticker_p = &side_p->stickers[pairs_hash[pair_corner]];
   cube_assert(sticker_p->linked_stickers_count == 2);
   if (sticker_p->active.values.color != side_p->color)
   {
      result = FALSE;
   }

   for (int i = 0; result && i < sticker_p->linked_stickers_count; ++i)
   {
      linked_sticker_p = &sticker_p->linked_stickers[i];
      linked_side_p = linked_sticker_p->connected_side_p;

      if (linked_side_p->stickers[linked_sticker_p->sticker_index].active.values.color != linked_side_p->color)
      {
         result = FALSE;
         break;
      }

      cube_assert(0 <= linked_sticker_p->sticker_index + paired_couple && linked_sticker_p->sticker_index + paired_couple < (CUBE_SIZE * CUBE_SIZE));
      if (linked_side_p->stickers[linked_sticker_p->sticker_index + paired_couple].active.values.color != linked_side_p->color)
      {
         result = FALSE;
         break;
      }
   }

   return result;
}

static void print_solved_pairs(Cube_t* cube_p, int solved_bitmap)
{
   char* colors_hash[] = { "Blue", "White", "Green", "Orange", "Red", "Yellow" };
   static int pairs_hash[] = { 0, 2, 6, 8 };
   CubeSide_t* side_p;
   Sticker_t* sticker_p;
   StickerLink_t* linked_sticker_p;
   CubeSide_t* linked_side_p;
   int is_first = TRUE;

   if (solved_bitmap)
      printf(" [");

   for (int p = 0; p < PAIR_CORNER_COUNT; ++p)
   {
      if (solved_bitmap & (1 << p))
      {
         if (!is_first)
            printf(", ");

         side_p = get_cube_side(cube_p, CUBE_SIDE_DOWN);
         sticker_p = &side_p->stickers[pairs_hash[p]];
         cube_assert(sticker_p->linked_stickers_count == 2);

         linked_sticker_p = &sticker_p->linked_stickers[0];
         linked_side_p = linked_sticker_p->connected_side_p;
         printf("%s/", colors_hash[linked_side_p->color]);

         linked_sticker_p = &sticker_p->linked_stickers[1];
         linked_side_p = linked_sticker_p->connected_side_p;
         printf("%s", colors_hash[linked_side_p->color]);

         is_first = FALSE;
      }
   }

   if (solved_bitmap)
      printf("]");
}

DWORD WINAPI threaded_solve_part(LPVOID lpParam)
{
   CubeThreadedSolution_t* threaded_cube_p = (CubeThreadedSolution_t*)lpParam;
   CubeRotation_t base_solution;

   base_solution.solution_array[0] = threaded_cube_p->start_rotation;
   base_solution.solution_depth = 1;

   solve_cfop_part(&threaded_cube_p->cube, &base_solution, threaded_cube_p->is_solved);

   return FALSE;
}

DWORD WINAPI threaded_solve_f2l(LPVOID lpParam)
{
   CubeThreadedSolution_t* threaded_cube_p = (CubeThreadedSolution_t*)lpParam;
   CubeRotation_t base_solution;

   base_solution.solution_array[0] = threaded_cube_p->start_rotation;
   base_solution.solution_depth = 1;

   solve_cfop_part(&threaded_cube_p->cube, &base_solution, are_more_pairs_solved);

   return FALSE;
}

__declspec(dllexport) void dll_print_cube(void)
{
   print_cube(&DLL_Cube);
}

static void solve_cfop_part(Cube_t* cube_p, CubeRotation_t* base_solution_p, int(*is_solved)(Cube_t*))
{
   int did_solve;

   Threaded_Solve.max_solve_depth = CROSS_MAX_SEARCH_DEPTH;

   cube_p->active_rotation.solution_depth = 0;
   cube_p->num_found_solutions = 0;
   for (int depth = 1; depth < Threaded_Solve.max_solve_depth; ++depth)
   {
      did_solve = solve_with_max_depth(cube_p, base_solution_p, depth, is_solved);

      if (did_solve > 0)
      {
         Threaded_Solve.max_solve_depth = M_MIN(Threaded_Solve.max_solve_depth, depth);
      }

      if (cube_p->num_found_solutions >= MAX_SOLUTIONS_THREAD)
      {
         break;
      }
   }

   rotate_between_rotations(cube_p, &cube_p->active_rotation, base_solution_p);
}

static int solve_with_max_depth(Cube_t* cube_p, CubeRotation_t* base_solution_p, int max_depth, int(*is_solved)(Cube_t*))
{
   static char rotations_array[] = { ROTATE_B, ROTATE_B_PRIME, ROTATE_B2, ROTATE_U, ROTATE_U_PRIME, ROTATE_U2, ROTATE_F, ROTATE_F_PRIME, ROTATE_F2, ROTATE_L, ROTATE_L_PRIME, ROTATE_L2, ROTATE_R, ROTATE_R_PRIME, ROTATE_R2, ROTATE_D, ROTATE_D_PRIME, ROTATE_D2 };
   int result = FALSE;
   int did_solve;
   char last_rotation;
   int rotation_step;
   int last_rotation_step;

   if (base_solution_p->solution_depth == max_depth)
   {
      rotate_between_rotations(cube_p, &cube_p->active_rotation, base_solution_p);
      memcpy(&cube_p->active_rotation, base_solution_p, sizeof(CubeRotation_t));
      cube_p->nodes_searched++;
      
      if (is_solved(cube_p))
      {
         cube_assert(cube_p->num_found_solutions < MAX_SOLUTIONS_THREAD);
         memcpy(&cube_p->found_solutions[cube_p->num_found_solutions], base_solution_p, sizeof(CubeRotation_t));
         cube_p->num_found_solutions++;
#ifdef DEBUG_MODE
         print_solution(cube_p, base_solution_p, TRUE);
#endif
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
      rotation_step = M_GET_ROTATIONS_STEP(rotations_array[i]);
      last_rotation_step = M_GET_ROTATIONS_STEP(last_rotation);
      if (rotation_step == last_rotation_step)
         continue;

      if (rotation_step == M_GET_ROTATIONS_STEP(ROTATE_R) && last_rotation_step == M_GET_ROTATIONS_STEP(ROTATE_L))
         continue;
      if (rotation_step == M_GET_ROTATIONS_STEP(ROTATE_D) && last_rotation_step == M_GET_ROTATIONS_STEP(ROTATE_U))
         continue;
      if (rotation_step == M_GET_ROTATIONS_STEP(ROTATE_F) && last_rotation_step == M_GET_ROTATIONS_STEP(ROTATE_B))
         continue;


      if (is_solved == are_more_pairs_solved)
      {
         if(M_GET_ROTATIONS_STEP(rotations_array[i]) == M_GET_ROTATIONS_STEP(ROTATE_D))
            continue;

         /*
         if (M_GET_ROTATIONS_STEP(rotations_array[i]) == M_GET_ROTATIONS_STEP(ROTATE_B))
            rotations_array[i] += ROTATE_Y - ROTATE_B;
         */
      }

      if (max_depth > Threaded_Solve.max_solve_depth)
      {
         break;
      }

      base_solution_p->solution_array[base_solution_p->solution_depth - 1] = rotations_array[i];
      did_solve = solve_with_max_depth(cube_p, base_solution_p, max_depth, is_solved);
      result = result || did_solve;

      if (cube_p->num_found_solutions >= MAX_SOLUTIONS_THREAD)
      {
         break;
      }
   }
   base_solution_p->solution_depth--;

   return result;
}


static void print_cube_links(Cube_t* cube_p)
{
   for (int i = 0; i < CUBE_SIDE_COUNT; ++i)
   {
      for (int j = 0; j < CUBE_SIZE*CUBE_SIZE; ++j)
      {
         Sticker_t* sticker_p = &cube_p->sides[i].stickers[j];
         printf("%d: ", sticker_p->active.values.unique_index);
         
         for (int k = 0; k < sticker_p->linked_stickers_count; ++k)
            printf("%d, ", sticker_p->linked_stickers[k].connected_side_p->stickers[sticker_p->linked_stickers[k].sticker_index].active.values.unique_index);

         printf("\n");
      }
      printf("\n");
   }
}

static int is_cross_solved(Cube_t* cube_p)
{
   int result = TRUE;
   static int cross_indices[] = { 1, 3, 5, 7 };
   CubeSide_t* side_p;
   CubeSide_t* linked_side_p;
   Sticker_t* sticker_p;
   StickerLink_t* linked_sticker_p;

   side_p = get_cube_side(cube_p, CUBE_SIDE_DOWN);
   for (int i = 0; i < 4; ++i)
   {
      // Check same as center
      sticker_p = &side_p->stickers[cross_indices[i]];
      if (sticker_p->active.values.color != side_p->color)
      {
         result = FALSE;
         break;
      }

      // Check linked stikcer same as its center
      cube_assert(sticker_p->linked_stickers_count == 1);
      linked_sticker_p = &sticker_p->linked_stickers[0];
      linked_side_p = linked_sticker_p->connected_side_p;
      if (linked_side_p->stickers[linked_sticker_p->sticker_index].active.values.color != linked_side_p->color)
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

   rotate_cube_array(cube_p, rotate_array2_p->solution_array, common_rotations, for_count, FALSE);
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

static void print_solution(Cube_t* cube_p, CubeRotation_t* solution_p, int add_line_break)
{
   char* rotate_p;
   int rotation_side;
   int padding = 0;

   ENTER_CRITICAL_SECTION();

   printf("   ");
#ifdef DEBUG_MODE
   printf("length %d; ", solution_p->solution_depth);
#endif

   rotate_p = &solution_p->solution_array[0];
   for (int i = 0; i < solution_p->solution_depth; ++i, ++rotate_p)
   {
      for (int p = 0; p < padding; ++p)
         printf(" ");

      rotation_side = M_GET_ROTATIONS_STEP(*rotate_p);
      
      if (rotation_side == CUBE_SIDE_BACK) printf("B");
      else if (rotation_side == CUBE_SIDE_FRONT) printf("F");
      else if (rotation_side == CUBE_SIDE_UP)  printf("U");
      else if (rotation_side == CUBE_SIDE_DOWN) printf("D");
      else if (rotation_side == CUBE_SIDE_LEFT) printf("L");
      else if (rotation_side == CUBE_SIDE_RIGHT) printf("R");
      else if (rotation_side == CUBE_SIDE_DOWN) printf("D");
      else if (rotation_side == CUBE_SIDE_LEFT) printf("L");
      else if (rotation_side == CUBE_SIDE_RIGHT) printf("R");

      if (M_IS_PRIME_ROTATION(*rotate_p)) printf("'");
      else if (M_IS_REPEAT_ROTATION(*rotate_p)) printf("2");

      padding = (M_IS_PRIME_ROTATION(*rotate_p) || M_IS_REPEAT_ROTATION(*rotate_p)) ? 1 : 2;
   }

   for (int p = 0; p < padding-1; ++p)
      printf(" ");

   if (add_line_break)
      printf("\n");

   EXIT_CRITICAL_SECTION();
}
