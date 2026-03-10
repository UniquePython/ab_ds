/*
 * ab_ds.h — umbrella header for ab_ds
 * Part of ab_ds — https://github.com/UniquePython/ab_ds
 *
 * USAGE
 *   Include individual headers for selective use, or include this file
 *   to get everything at once. Define the umbrella IMPLEMENTATION
 *   macro in exactly one .c file before including.
 *
 *     #define AB_DS_IMPLEMENTATION
 *     #include "ab_ds.h"
 *
 * LICENSE — MIT
 *   Copyright (c) 2025 ab_ds contributors
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions: The above copyright notice and this
 *   permission notice shall be included in all copies or substantial
 *   portions of the Software. THE SOFTWARE IS PROVIDED "AS IS", WITHOUT
 *   WARRANTY OF ANY KIND.
 *
 * VERSION — 0.1.0
 */

#ifndef AB_DS_H
#define AB_DS_H

#ifdef AB_DS_IMPLEMENTATION
#define AB_AL_IMPLEMENTATION
#define AB_SL_IMPLEMENTATION
#define AB_DL_IMPLEMENTATION
#define AB_DQ_IMPLEMENTATION
#define AB_SS_IMPLEMENTATION
#define AB_HS_IMPLEMENTATION
#define AB_LHS_IMPLEMENTATION
#define AB_TS_IMPLEMENTATION
#endif

#include "ab_ds_common.h"
#include "list/ab_ds_list.h"
#include "queue/ab_ds_queue.h"
#include "set/ab_ds_set.h"

#endif