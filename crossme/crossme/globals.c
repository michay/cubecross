#include "globals.h"
#include <stdio.h>
#include <assert.h>

void cube_assert(int condition)
{
#ifndef RELEASE
   if (!condition)
   {
      printf("Error!!!");
   }

   assert(condition);
#endif
}

ranctx local_var;
int init_seed = 123456789;


int random_generate(int ceiling)
{
   unsigned int result;

   result = ranval(&local_var);

   return (int)(result % ceiling);
}