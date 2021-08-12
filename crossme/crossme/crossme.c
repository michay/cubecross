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
#define CUBE_SIDES 6
#define MAX_DEPTH 6
#define ARRAY_LENGTH 18
#define MAX_QUEUE_DEPTH (18 * 18 * 18 * 18 * 18 * 18 * 18)

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
   STICKER_TYPE_EDGE,
   STICKER_TYPE_CORNER,
   STICKER_TYPE_CENTER,
} StickerType_t;

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

typedef struct
{
   int* entries_p;
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
   CubeSide_t sides[CUBE_SIDES];
   int timestamp;
} Cube_t;


static void enqueue(Queue_t* queue_p, int value);
static int dequeue(Queue_t* queue_p);

static void init_cube_side(CubeSide_t* side_p, int color);
static void rotate_cube_single(Cube_t* cube_p, int rotation);
static void rotate_cube_side(Cube_t* cube_p, int side, int is_clockwise);
static void rotate_cube_string(Cube_t* cube_p, char* rotate_input_p);
static void print_cube_side(CubeSide_t* side_p);

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

   int i;
   int nodes_searched = 0;
   
   init_cube(&cube);

   print_cube(&cube);

   //rotate_cube_string(&cube, "F B' L2 U2 B' D2 B2 R2 U2 B2 D2 R' U F2 R F' L' U' R F' B2");
   //print_cube(&cube);


   //print_cube_side(&cube.sides[0]);

   /*
   // BFS for solve

   // Init queue
   queue.head = 0;
   queue.tail = 0;
   queue.count = 0;
   queue.entries_p = (int*)malloc(sizeof(int)*MAX_QUEUE_DEPTH);
   assert(queue.entries_p != NULL);

   for (i = 0; i < ARRAY_LENGTH; ++i)
      enqueue(&queue, 0);

   while (queue.count > 0)
   {
      int p = dequeue(&queue);
      nodes_searched++;

      if (p < MAX_DEPTH)
      {
         for (i = 0; i < ARRAY_LENGTH; ++i)
            enqueue(&queue, p + 1);
      }
   }
   printf("nodes_searched = %d", nodes_searched);
   */
}

static void init_cube(Cube_t* cube_p)
{
   CubeSide_t* back_side_p = &cube_p->sides[COLOR_BLUE];
   CubeSide_t* front_side_p = &cube_p->sides[COLOR_GREEN];
   CubeSide_t* left_side_p = &cube_p->sides[COLOR_ORANGE];
   CubeSide_t* up_side_p = &cube_p->sides[COLOR_WHITE];
   CubeSide_t* right_side_p = &cube_p->sides[COLOR_RED];
   CubeSide_t* down_side_p = &cube_p->sides[COLOR_YELLOW];
   Sticker_t* usticker_p;
   Sticker_t* bsticker_p;
   Sticker_t* lsticker_p;
   Sticker_t* rsticker_p;
   Sticker_t* dsticker_p;
   int i;

   CubeSide_t* side_p;

   memset(cube_p, 0, sizeof(Cube_t));
   
   side_p = &cube_p->sides;
   for (i = 0; i < CUBE_SIDES; ++i, ++side_p)
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
   CubeSide_t* back_side_p = &cube_p->sides[COLOR_BLUE];
   CubeSide_t* front_side_p = &cube_p->sides[COLOR_GREEN];
   CubeSide_t* left_side_p = &cube_p->sides[COLOR_ORANGE];
   CubeSide_t* up_side_p = &cube_p->sides[COLOR_WHITE];
   CubeSide_t* right_side_p = &cube_p->sides[COLOR_RED];
   CubeSide_t* down_side_p = &cube_p->sides[COLOR_YELLOW];

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
   for (int i = 0; i < CUBE_SIDES; ++i)
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
   is_anti_clockwise = (rotation & 1);
   num_repeats = (rotation & 2) ? 2 : 1;

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

static void enqueue(Queue_t* queue_p, int value)
{
   queue_p->entries_p[queue_p->head] = value;

   queue_p->head++;
   if (queue_p->head == MAX_QUEUE_DEPTH)
      queue_p->head = 0;


   queue_p->count++;
   assert(queue_p->count <= MAX_QUEUE_DEPTH);
}

static int dequeue(Queue_t* queue_p)
{
   int result;

   result = queue_p->entries_p[queue_p->tail];
   
   queue_p->tail++;
   if (queue_p->tail == MAX_QUEUE_DEPTH)
      queue_p->tail = 0;

   assert(queue_p->count > 0);
   queue_p->count--;

   return result;
}
