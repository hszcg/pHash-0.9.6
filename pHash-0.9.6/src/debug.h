#ifndef PHASH__DEBUG_H__
#define PHASH__DEBUG_H__

#define DEBUG_FFMPEG
#ifdef DEBUG_FFMPEG
# define debug_printf(x) \
{ \
	printf("%s(%d): %s: ", __FILE__, __LINE__, __func__); \
	printf x; \
	fflush(stdout); \
}
#else
# define debug_printf(x) do {} while (0)
#endif

#endif
