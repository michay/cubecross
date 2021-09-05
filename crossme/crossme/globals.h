#pragma once

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#define M_DIFF_ABS(_val_1_, _val_2_) ((_val_1_) > (_val_2_) ? ((_val_1_) - (_val_2_)) : ((_val_2_) - (_val_1_)))
#define M_DIFF_NOABS(_val_1_, _val_2_) ((_val_1_) - (_val_2_))

#define CROSS_MAX_SEARCH_DEPTH (13)

#define MAX_SOLUTIONS_THREAD (1)
#define MAX_ALLOWED_SOLUTIONS ((MAX_SOLUTIONS_THREAD) * (ARRAY_LENGTH))

#define M_MIN(_val1_, _val2_) (((_val1_ )< (_val2_)) ? (_val1_)  : ( _val2_))
#define M_MAX(_val1_, _val2_) (((_val1_ )> (_val2_)) ? (_val1_)  : ( _val2_))

void cube_assert(int condition);

typedef struct ranctx
{
   int a;
   int b;
   int c;
   int d;
} ranctx;

extern ranctx local_var;
extern int init_seed;

#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
__inline int ranval(ranctx *x)
{
   int e = x->a - rot(x->b, 27);
   x->a = x->b ^ rot(x->c, 17);
   x->b = x->c + x->d;
   x->c = x->d + e;
   x->d = e + x->a;
   return x->d;
}

int random_generate(int ceiling);

__inline void raninit(void)
{
   int i;
   ranctx *x = &local_var;
   int seed = init_seed;

   x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
   for (i = 0; i < 20; ++i)
   {
      (void)ranval(x);
   }
}
