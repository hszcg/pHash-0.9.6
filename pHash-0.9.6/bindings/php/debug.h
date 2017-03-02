#ifndef PHASH_PHP__DEBUG_H__
#define PHASH_PHP__DEBUG_H__

//#define DEBUG_NEW_VIDEO_PHASH
#ifdef DEBUG_NEW_VIDEO_PHASH
#define debug_printf(x) printf x
#else
#define debug_printf(x) {}
#endif

#endif
