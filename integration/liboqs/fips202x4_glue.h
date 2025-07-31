/*
 * Copyright (c) The mldsa-native project authors
 * SPDX-License-Identifier: Apache-2.0 OR ISC OR MIT
 */
#ifndef MLD_INTEGRATION_LIBOQS_FIPS202X4_GLUE_H
#define MLD_INTEGRATION_LIBOQS_FIPS202X4_GLUE_H

/* Include OQS's own FIPS202_X4 header */
#include "fips202x4.h"

/* OQS's FIPS202_X4 is as-is compatible with the one expected
 * by mldsa-native, so just remove the mld_xxx prefix. */
#define mld_shake128x4ctx shake128x4ctx
#define mld_shake128x4_absorb_once shake128x4_absorb_once
#define mld_shake128x4_squeezeblocks shake128x4_squeezeblocks
#define mld_shake128x4_init shake128x4_init
#define mld_shake128x4_release shake128x4_release
#define mld_shake256x4 shake256x4

#endif /* !MLD_INTEGRATION_LIBOQS_FIPS202X4_GLUE_H */
