[//]: # (SPDX-License-Identifier: CC-BY-4.0)

# Replacing FIPS-202

If your library has a FIPS-202[^FIPS202] implementation, you can use it instead of the one shipped with mldsa-native.

1. Replace `mldsa/src/fips202/*` by your own FIPS-202 implementation.
2. Provide replacements for the headers [`mldsa/fips202/fips202.h`](mldsa/fips202/fips202.h) and [`mldsa/fips202/fips202x4.h`](mldsa/fips202/fips202x4.h) and the
functionalities specified therein:
  * Structure definitions for `mld_shake128ctx` and `mld_shake128x4ctx`
  * `mld_shake128_absorb_once()`: Initialize a SHAKE-128 context and perform a single> absorb step.
  * `mld_shake128_squeezeblocks()`: Squeeze SHAKE-128 context
  * `mld_shake128_release()`: Release a SHAKE-128 context after use
  * `mld_shake256()`, `mld_sha3_256()`, `mld_sha3_512()`: One-shot SHAKE-256 / SHA3-256 / SHA3-512 operations
  * `mld_shake256x4()`: One-shot 4x-batched SHAKE-256 operation
  * `mld_shake128x4_absorb_once()`: Initialize a 4x-batched SHAKE-128 context and perform a single absorb step.
  * `mld_shake128x4_squeezeblocks()`: Squeeze 4x-batched SHAKE-128 context
  * `mld_shake128x4_release()`: Release a 4x-batched SHAKE-128 context after use

See [`mldsa/fips202/fips202.h`](mldsa/fips202/fips202.h) and [`mldsa/fips202/fips202x4.h`](mldsa/fips202/fips202x4.h) for more details. Note that the structure
definitions may differ from those shipped with mldsa-native: In particular, you may fall back to an incremental hashing
implementation which tracks the current offset in its state.

<!--- bibliography --->
[^FIPS202]: National Institute of Standards and Technology: FIPS202 SHA-3 Standard: Permutation-Based Hash and Extendable-Output Functions, [https://csrc.nist.gov/pubs/fips/202/final](https://csrc.nist.gov/pubs/fips/202/final)
