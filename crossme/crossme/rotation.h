#pragma once

#include "globals.h"
#include "cube.h"

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
   AXIS_X = 0,
   AXIS_Y,
   AXIS_Z,
   AXIS_COUNT
} AxisRotation_t;

#define M_IS_PRIME_ROTATION(_rotation_) ((_rotation_) & 1)
#define M_TOGGLE_PRIME_INDICATION(_rotation_) ((_rotation_) ^= 1)
#define M_IS_REPEAT_ROTATION(_rotation_) ((_rotation_) & 2)
#define M_GET_ROTATIONS_STEP(_rotation_) ((_rotation_) >> 2)
#define M_IS_AXIS_ROTATION(_rotation_) ((_rotation_) >= ROTATE_X)
#define M_GET_AXIS_TO_ROTATE(_rotation_) (((_rotation_) - ROTATE_X) >> 2)

void rotate_cube_string(Cube_t* cube_p, char* rotate_input_p, int do_print, int do_sync);
void rotate_cube_array(Cube_t* cube_p, char* rotate_array, int start_offset, int rotations_count, int do_sync);
void anti_rotate(Cube_t* cube_p, int rotation);
void rotate_cube_axis(Cube_t* cube_p, int rotation);
void print_cube_side(CubeSide_t* side_p);

