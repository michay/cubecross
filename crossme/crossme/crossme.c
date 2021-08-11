#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <malloc.h>


#define DEBUG_MODE

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
   int sticker_type;  //StickerType_t
   StickerLink_t linked_stickers[MAX_LINKED_STICKERS];
   int linked_stickers_count;
} Sticker_t;

typedef struct
{
   int color; // CubeColor_t
   Sticker_t stickers[CUBE_SIZE * CUBE_SIZE];
} CubeSide_t;

typedef struct
{
   CubeSide_t sides[CUBE_SIDES];
} Cube_t;


static void enqueue(Queue_t* queue_p, int value);
static int dequeue(Queue_t* queue_p);

static void init_cube_side(CubeSide_t* side_p, int color);
static void rotate_cube_side(CubeSide_t* side_p, int is_clockwise);
static void print_cube_side(CubeSide_t* side_p);

static void init_cube(Cube_t* cube_p);
static void link_up_down(CubeSide_t* up_side_p, CubeSide_t* down_side_p);

int _tmain(int argc, _TCHAR* argv[])
{
   Queue_t queue;
   Cube_t cube;

   int i;
   int nodes_searched = 0;
   
   init_cube(&cube);

   print_cube_side(&cube.sides[0]);

   rotate_cube_side(&cube.sides[0], FALSE);

   print_cube_side(&cube.sides[0]);

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
   CubeSide_t* side_p;

   side_p = &cube_p->sides;
   for (int i = 0; i < CUBE_SIDES; ++i, ++side_p)
   {
      memset(side_p, 0, sizeof(CubeSide_t));
      init_cube_side(side_p, i);
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
      dsticker_p->linked_stickers[dsticker_p->linked_stickers_count].connected_side_p = up_side_p;
      dsticker_p->linked_stickers[dsticker_p->linked_stickers_count].sticker_index = usticker_p - up_side_p->stickers;
      
      usticker_p->linked_stickers[usticker_p->linked_stickers_count].connected_side_p = down_side_p;
      usticker_p->linked_stickers[usticker_p->linked_stickers_count].sticker_index = dsticker_p - down_side_p->stickers;

      dsticker_p++;
      up_side_p++;

      dsticker_p->linked_stickers_count++;
      assert(dsticker_p->linked_stickers_count < MAX_LINKED_STICKERS);

      usticker_p->linked_stickers_count++;
      assert(usticker_p->linked_stickers_count < MAX_LINKED_STICKERS);
   }
}

static void init_cube_side(CubeSide_t* side_p, int color)
{
   Sticker_t* sticker_p;

   side_p->color = color;
   
   sticker_p = side_p->stickers;
   for (int i = 0; i < CUBE_SIZE * CUBE_SIZE; ++i, ++sticker_p)
   {
      sticker_p->color = color;
      sticker_p->unique_index = (color + 1) * 10 + i;

      if (i == 4)
         sticker_p->sticker_type = STICKER_TYPE_CENTER;
      else if (i %2 == 0)
         sticker_p->sticker_type = STICKER_TYPE_CORNER;
      else 
         sticker_p->sticker_type = STICKER_TYPE_EDGE;
   }
}

static void rotate_cube_side(CubeSide_t* side_p, int is_clockwise)
{
   int clowise_position_array[] = { 6, 3, 0, 7, 4, 1, 8, 5, 2 };
   int anti_clowise_position_array[] = { 2, 5, 8, 1, 4, 7, 0, 3, 6 };
   int* new_indices_array_p;
   Sticker_t prev_stickers[CUBE_SIZE * CUBE_SIZE];

   // select array to use
   new_indices_array_p = is_clockwise ? clowise_position_array : anti_clowise_position_array;

   // Copy previous stickers
   memcpy(&prev_stickers, &side_p->stickers, sizeof(prev_stickers));

   // Setup new stickers
   for (int i = 0; i < CUBE_SIZE * CUBE_SIZE; ++i)
   {
      int copy_index = new_indices_array_p[i];
      memcpy(&side_p->stickers[i], &prev_stickers[copy_index], sizeof(Sticker_t));
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
         printf("%c ", colors_hash[side_p->stickers[i * CUBE_SIZE + j].color]);
#else
         printf("%d ", side_p->stickers[i * CUBE_SIZE + j].unique_index);
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
