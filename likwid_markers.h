#ifndef LIKWID_MARKERS_H
#define LIKWID_MARKERS_H

/*
 * Se compilado com -DLIKWID_PERFMON (make LIKWID=1, no Linux), usa a API de
 * marcadores real do LIKWID. Caso contrario (ex.: Windows), as macros viram
 * vazias e o programa compila e roda normalmente, sem instrumentacao.
 */
#ifdef LIKWID_PERFMON
#include <likwid.h>
#else
#define LIKWID_MARKER_INIT
#define LIKWID_MARKER_THREADINIT
#define LIKWID_MARKER_REGISTER(regionTag)
#define LIKWID_MARKER_START(regionTag)
#define LIKWID_MARKER_STOP(regionTag)
#define LIKWID_MARKER_CLOSE
#endif

#endif
