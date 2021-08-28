#pragma once

#undef TRUE
#undef FALSE
#define TRUE 1
#define FALSE 0

#define CROSS_MAX_SEARCH_DEPTH (13)

#define MAX_SOLUTIONS_THREAD (1)
#define MAX_ALLOWED_SOLUTIONS ((MAX_SOLUTIONS_THREAD) * (ARRAY_LENGTH))

#define M_MIN(_val1_, _val2_) (((_val1_ )< (_val2_)) ? (_val1_)  : ( _val2_))
#define M_MAX(_val1_, _val2_) (((_val1_ )> (_val2_)) ? (_val1_)  : ( _val2_))

void cube_assert(int condition);