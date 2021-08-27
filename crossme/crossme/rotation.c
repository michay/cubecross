#include <stdio.h>
#include <string.h>
#include "cube.h"
#include "rotation.h"

void rotate_cube_single(Cube_t* cube_p, int rotation)
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

void rotate_cube_side(Cube_t* cube_p, int side, int is_clockwise)
{
   CubeSide_t* side_p = &cube_p->sides[side];
   static int clockwise_position_array[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };
   static int anti_cloclwise_position_array[] = { 2, 5, 8, 1, 4, 7, 0, 3, 6 };
   int* new_indices_array_p;
   int* inverse_indices_array_p;
   Sticker_t prev_stickers[CUBE_SIZE * CUBE_SIZE];
   Sticker_t* sticker_p;
   Sticker_t* new_pos_sticker_p;

   // select array to use
   new_indices_array_p = is_clockwise ? clockwise_position_array : anti_cloclwise_position_array;
   inverse_indices_array_p = is_clockwise ? anti_cloclwise_position_array : clockwise_position_array;

   // Copy previous stickers
   memcpy(&prev_stickers, &side_p->stickers, sizeof(prev_stickers));

   // Set timestamp for copy
   cube_p->timestamp++;

   // Setup new stickers
   for (int i = 0; i < CUBE_SIZE * CUBE_SIZE; ++i)
   {
      int copy_index = new_indices_array_p[i];
      side_p->stickers[i].previous.all = side_p->stickers[i].active.all;

      if (prev_stickers[copy_index].timestamp == cube_p->timestamp)
         side_p->stickers[i].active.all = prev_stickers[copy_index].previous.all;
      else
         side_p->stickers[i].active.all = prev_stickers[copy_index].active.all;

      side_p->stickers[i].timestamp = cube_p->timestamp;
   }

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
         new_link_p->connected_side_p->stickers[new_link_p->sticker_index].previous.all = new_link_p->connected_side_p->stickers[new_link_p->sticker_index].active.all;

         if (prev_link_p->connected_side_p->stickers[prev_link_p->sticker_index].timestamp == cube_p->timestamp)
            new_sticker_value_p = &prev_link_p->connected_side_p->stickers[prev_link_p->sticker_index].previous;
         else
            new_sticker_value_p = &prev_link_p->connected_side_p->stickers[prev_link_p->sticker_index].active;

         // Copy new value
         new_link_p->connected_side_p->stickers[new_link_p->sticker_index].active.all = new_sticker_value_p->all;
         new_link_p->connected_side_p->stickers[new_link_p->sticker_index].timestamp = cube_p->timestamp;
      }
   }
}

void rotate_cube_string(Cube_t* cube_p, char* rotate_input_p, int do_print)
{
   int rotation = -1;
   int is_new_rotation = TRUE;
   char* inp_p = rotate_input_p;

   if (do_print)
   {
      printf("Rotate [");
      print_orientation(cube_p);
      printf("]:\n  %s\n\n", rotate_input_p);
   }

   while (*inp_p)
   {
      if (*inp_p == ' ')
      {
         if (rotation != -1)
         {
            rotate_cube_single(cube_p, rotation);

            cube_p->synced_rotation.solution_array[cube_p->synced_rotation.solution_depth] = rotation;
            cube_p->synced_rotation.solution_depth++;
            cube_assert(cube_p->synced_rotation.solution_depth < MAX_ROTATION_ARRAY);

            is_new_rotation = TRUE;
            rotation = -1;
         }
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

void rotate_cube_array(Cube_t* cube_p, char* rotate_array, int start_offset, int rotations_count, int do_sync)
{
   char* rotate_p;

   rotate_p = rotate_array + start_offset;
   while (rotations_count)
   {
      rotate_cube_single(cube_p, *rotate_p);

      if (do_sync)
      {
         cube_p->synced_rotation.solution_array[cube_p->synced_rotation.solution_depth] = *rotate_p;
         cube_p->synced_rotation.solution_depth++;
         cube_assert(cube_p->synced_rotation.solution_depth < MAX_ROTATION_ARRAY);
      }

      rotations_count--;
      rotate_p++;
   }
}

void anti_rotate(Cube_t* cube_p, int rotation)
{
   int anti_rotation;

   anti_rotation = rotation;

   if (!M_IS_REPEAT_ROTATION(rotation))
   {
      M_TOGGLE_PRIME_INDICATION(anti_rotation);
   }

   rotate_cube_single(cube_p, anti_rotation);
}

void rotate_cube_axis(Cube_t* cube_p, int rotation)
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

static void print_cube_side(CubeSide_t* side_p)
{
   char colors_hash[] = { 'B', 'W', 'G', 'O', 'R', 'Y' };
   for (int i = 0; i < CUBE_SIZE; ++i)
   {
      for (int j = 0; j < CUBE_SIZE; ++j)
      {
#ifndef DEBUG_MODE
         printf("%c ", colors_hash[side_p->stickers[i * CUBE_SIZE + j].active.values.color]);
#else
         printf("%d ", side_p->stickers[i * CUBE_SIZE + j].active.unique_index);
#endif
      }
      printf("\n");
   }
   printf("\n");
}
