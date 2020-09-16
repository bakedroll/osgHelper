#pragma once

#define assert_return(cond, ...) if (!(cond)) { assert(false); return __VA_ARGS__; }
#define assert_continue(cond) if (!(cond)) { assert(false); continue; }