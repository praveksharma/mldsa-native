/*
 * Copyright (c) The mlkem-native project authors
 * Copyright (c) 2025 The mldsa-native project authors
 * SPDX-License-Identifier: Apache-2.0
 */

/* NOTE: You can remove this file unless you compile with MLDSA_DEBUG. */

#include "common.h"

#if defined(MLDSA_DEBUG)

#include <stdio.h>
#include <stdlib.h>
#include "debug.h"

#define MLD_DEBUG_ERROR_HEADER "[ERROR:%s:%04d] "

void mld_debug_check_assert(const char *file, int line, const int val)
{
  if (val == 0)
  {
    fprintf(stderr, MLD_DEBUG_ERROR_HEADER "Assertion failed (value %d)\n",
            file, line, val);
    exit(1);
  }
}

void mld_debug_check_bounds(const char *file, int line, const int32_t *ptr,
                            unsigned len, int lower_bound_exclusive,
                            int upper_bound_exclusive)
{
  int err = 0;
  unsigned i;
  for (i = 0; i < len; i++)
  {
    int32_t val = ptr[i];
    if (!(val > lower_bound_exclusive && val < upper_bound_exclusive))
    {
      fprintf(
          stderr,
          MLD_DEBUG_ERROR_HEADER
          "Bounds assertion failed: Index %u, value %d out of bounds (%d,%d)\n",
          file, line, i, (int)val, lower_bound_exclusive,
          upper_bound_exclusive);
      err = 1;
    }
  }

  if (err == 1)
  {
    exit(1);
  }
}

#else /* MLDSA_DEBUG */

MLD_EMPTY_CU(debug)

#endif /* !MLDSA_DEBUG */

/* To facilitate single-compilation-unit (SCU) builds, undefine all macros.
 * Don't modify by hand -- this is auto-generated by scripts/autogen. */
#undef MLD_DEBUG_ERROR_HEADER
