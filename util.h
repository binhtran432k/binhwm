/* See LICENSE file for copyright and license details. */
#ifndef _BINHWM_UTIL_H_
#define _BINHWM_UTIL_H_

#include <stdio.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B) ((A) <= (X) && (X) <= (B))

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);

#endif // !_BINHWM_UTIL_H_
