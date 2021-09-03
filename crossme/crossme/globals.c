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