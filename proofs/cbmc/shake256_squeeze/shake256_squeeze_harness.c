// Copyright (c) 2025 The mldsa-native project authors
// SPDX-License-Identifier: Apache-2.0

#include "fips202/fips202.h"

void harness(void)
{
  uint8_t *out;
  size_t outlen;
  keccak_state *state;

  shake256_squeeze(out, outlen, state);
}
