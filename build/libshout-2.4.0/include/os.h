#ifdef _MSC_VER
#include <stdint.h>
#if defined(_WIN64)
typedef __int64 ssize_t;
#else
typedef long ssize_t;
#endif
#endif
