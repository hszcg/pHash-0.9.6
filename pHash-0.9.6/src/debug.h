#ifndef PHASH__DEBUG_H__
#define PHASH__DEBUG_H__

#define DEBUG_PHASH_LIB
#ifdef DEBUG_PHASH_LIB
#define debug_printf(x) \
{ \
	printf("%s(%d): %s: ", __FILE__, __LINE__, __func__); \
	printf x; \
	fflush(stdout); \
}
#else
#define debug_printf(x) {}
#endif

#endif
