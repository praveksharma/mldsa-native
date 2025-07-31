/*
 * Copyright (c) The mldsa-native project authors
 * SPDX-License-Identifier: Apache-2.0 OR ISC OR MIT
 */
#ifndef MLD_INTEGRATION_LIBOQS_FIPS202_GLUE_H
#define MLD_INTEGRATION_LIBOQS_FIPS202_GLUE_H

/* Include OQS's own FIPS202 header */
#include "fips202.h"

/* OQS's FIPS202 is as-is compatible with the one expected
 * by mldsa-native, so just remove the mld_xxx prefix. */
#define mld_shake128ctx shake128ctx
#define mld_shake128_absorb_once shake128_absorb_once
#define mld_shake128_squeezeblocks shake128_squeezeblocks
#define mld_shake128_init shake128_init
#define mld_shake128_release shake128_release
#define mld_shake256 shake256
#define mld_sha3_256 sha3_256
#define mld_sha3_512 sha3_512

#endif /* !MLD_INTEGRATION_LIBOQS_FIPS202_GLUE_H */
