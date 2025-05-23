/*
 * Copyright (c) 2025 The mldsa-native project authors
 * SPDX-License-Identifier: Apache-2.0
 */
/* Based on the public domain implementation in crypto_hash/keccakc512/simple/
 * from http://bench.cr.yp.to/supercop.html by Ronny Van Keer and the public
 * domain "TweetFips202" implementation from https://twitter.com/tweetfips202 by
 * Gilles Van Assche, Daniel J. Bernstein, and Peter Schwabe */

#include <stddef.h>
#include <stdint.h>

#include "fips202.h"

#define NROUNDS 24
#define ROL(a, offset) ((a << offset) ^ (a >> (64 - offset)))

/*************************************************
 * Name:        load64
 *
 * Description: Load 8 bytes into uint64_t in little-endian order
 *
 * Arguments:   - const uint8_t *x: pointer to input byte array
 *
 * Returns the loaded 64-bit unsigned integer
 **************************************************/
static uint64_t load64(const uint8_t x[8])
__contract__(
  requires(memory_no_alias(x, sizeof(uint8_t) * 8))
)
{
  unsigned int i;
  uint64_t r = 0;

  for (i = 0; i < 8; i++)
  __loop__(
    invariant(i <= 8)
  )
  {
    r |= (uint64_t)x[i] << 8 * i;
  }

  return r;
}

/*************************************************
 * Name:        store64
 *
 * Description: Store a 64-bit integer to array of 8 bytes in little-endian
 *order
 *
 * Arguments:   - uint8_t *x: pointer to the output byte array (allocated)
 *              - uint64_t u: input 64-bit unsigned integer
 **************************************************/
static void store64(uint8_t x[8], uint64_t u)
__contract__(
  requires(memory_no_alias(x, sizeof(uint8_t) * 8))
  assigns(memory_slice(x, sizeof(uint8_t) * 8))
)
{
  unsigned int i;

  for (i = 0; i < 8; i++)
  __loop__(
    invariant(i <= 8)
  )
  {
    /* Explicitly truncate to uint8_t */
    x[i] = (uint8_t)((u >> (8 * i)) & 0xFF);
  }
}

/* Keccak round constants */
const uint64_t KeccakF_RoundConstants[NROUNDS] = {
    (uint64_t)0x0000000000000001ULL, (uint64_t)0x0000000000008082ULL,
    (uint64_t)0x800000000000808aULL, (uint64_t)0x8000000080008000ULL,
    (uint64_t)0x000000000000808bULL, (uint64_t)0x0000000080000001ULL,
    (uint64_t)0x8000000080008081ULL, (uint64_t)0x8000000000008009ULL,
    (uint64_t)0x000000000000008aULL, (uint64_t)0x0000000000000088ULL,
    (uint64_t)0x0000000080008009ULL, (uint64_t)0x000000008000000aULL,
    (uint64_t)0x000000008000808bULL, (uint64_t)0x800000000000008bULL,
    (uint64_t)0x8000000000008089ULL, (uint64_t)0x8000000000008003ULL,
    (uint64_t)0x8000000000008002ULL, (uint64_t)0x8000000000000080ULL,
    (uint64_t)0x000000000000800aULL, (uint64_t)0x800000008000000aULL,
    (uint64_t)0x8000000080008081ULL, (uint64_t)0x8000000000008080ULL,
    (uint64_t)0x0000000080000001ULL, (uint64_t)0x8000000080008008ULL};

/*************************************************
 * Name:        KeccakF1600_StatePermute
 *
 * Description: The Keccak F1600 Permutation
 *
 * Arguments:   - uint64_t *state: pointer to input/output Keccak state
 **************************************************/
static void KeccakF1600_StatePermute(uint64_t state[MLD_KECCAK_LANES])
__contract__(
  requires(memory_no_alias(state, sizeof(uint64_t) * MLD_KECCAK_LANES))
  assigns(memory_slice(state, sizeof(uint64_t) * MLD_KECCAK_LANES)))
{
  unsigned round;

  uint64_t Aba, Abe, Abi, Abo, Abu;
  uint64_t Aga, Age, Agi, Ago, Agu;
  uint64_t Aka, Ake, Aki, Ako, Aku;
  uint64_t Ama, Ame, Ami, Amo, Amu;
  uint64_t Asa, Ase, Asi, Aso, Asu;
  uint64_t BCa, BCe, BCi, BCo, BCu;
  uint64_t Da, De, Di, Do, Du;
  uint64_t Eba, Ebe, Ebi, Ebo, Ebu;
  uint64_t Ega, Ege, Egi, Ego, Egu;
  uint64_t Eka, Eke, Eki, Eko, Eku;
  uint64_t Ema, Eme, Emi, Emo, Emu;
  uint64_t Esa, Ese, Esi, Eso, Esu;

  /* copyFromState(A, state) */
  Aba = state[0];
  Abe = state[1];
  Abi = state[2];
  Abo = state[3];
  Abu = state[4];
  Aga = state[5];
  Age = state[6];
  Agi = state[7];
  Ago = state[8];
  Agu = state[9];
  Aka = state[10];
  Ake = state[11];
  Aki = state[12];
  Ako = state[13];
  Aku = state[14];
  Ama = state[15];
  Ame = state[16];
  Ami = state[17];
  Amo = state[18];
  Amu = state[19];
  Asa = state[20];
  Ase = state[21];
  Asi = state[22];
  Aso = state[23];
  Asu = state[24];

  for (round = 0; round < NROUNDS; round += 2)
  __loop__(invariant(round <= NROUNDS && round % 2 == 0))
  {
    /* prepareTheta */
    BCa = Aba ^ Aga ^ Aka ^ Ama ^ Asa;
    BCe = Abe ^ Age ^ Ake ^ Ame ^ Ase;
    BCi = Abi ^ Agi ^ Aki ^ Ami ^ Asi;
    BCo = Abo ^ Ago ^ Ako ^ Amo ^ Aso;
    BCu = Abu ^ Agu ^ Aku ^ Amu ^ Asu;

    /* thetaRhoPiChiIotaPrepareTheta(round, A, E) */
    Da = BCu ^ ROL(BCe, 1);
    De = BCa ^ ROL(BCi, 1);
    Di = BCe ^ ROL(BCo, 1);
    Do = BCi ^ ROL(BCu, 1);
    Du = BCo ^ ROL(BCa, 1);

    Aba ^= Da;
    BCa = Aba;
    Age ^= De;
    BCe = ROL(Age, 44);
    Aki ^= Di;
    BCi = ROL(Aki, 43);
    Amo ^= Do;
    BCo = ROL(Amo, 21);
    Asu ^= Du;
    BCu = ROL(Asu, 14);
    Eba = BCa ^ ((~BCe) & BCi);
    Eba ^= (uint64_t)KeccakF_RoundConstants[round];
    Ebe = BCe ^ ((~BCi) & BCo);
    Ebi = BCi ^ ((~BCo) & BCu);
    Ebo = BCo ^ ((~BCu) & BCa);
    Ebu = BCu ^ ((~BCa) & BCe);

    Abo ^= Do;
    BCa = ROL(Abo, 28);
    Agu ^= Du;
    BCe = ROL(Agu, 20);
    Aka ^= Da;
    BCi = ROL(Aka, 3);
    Ame ^= De;
    BCo = ROL(Ame, 45);
    Asi ^= Di;
    BCu = ROL(Asi, 61);
    Ega = BCa ^ ((~BCe) & BCi);
    Ege = BCe ^ ((~BCi) & BCo);
    Egi = BCi ^ ((~BCo) & BCu);
    Ego = BCo ^ ((~BCu) & BCa);
    Egu = BCu ^ ((~BCa) & BCe);

    Abe ^= De;
    BCa = ROL(Abe, 1);
    Agi ^= Di;
    BCe = ROL(Agi, 6);
    Ako ^= Do;
    BCi = ROL(Ako, 25);
    Amu ^= Du;
    BCo = ROL(Amu, 8);
    Asa ^= Da;
    BCu = ROL(Asa, 18);
    Eka = BCa ^ ((~BCe) & BCi);
    Eke = BCe ^ ((~BCi) & BCo);
    Eki = BCi ^ ((~BCo) & BCu);
    Eko = BCo ^ ((~BCu) & BCa);
    Eku = BCu ^ ((~BCa) & BCe);

    Abu ^= Du;
    BCa = ROL(Abu, 27);
    Aga ^= Da;
    BCe = ROL(Aga, 36);
    Ake ^= De;
    BCi = ROL(Ake, 10);
    Ami ^= Di;
    BCo = ROL(Ami, 15);
    Aso ^= Do;
    BCu = ROL(Aso, 56);
    Ema = BCa ^ ((~BCe) & BCi);
    Eme = BCe ^ ((~BCi) & BCo);
    Emi = BCi ^ ((~BCo) & BCu);
    Emo = BCo ^ ((~BCu) & BCa);
    Emu = BCu ^ ((~BCa) & BCe);

    Abi ^= Di;
    BCa = ROL(Abi, 62);
    Ago ^= Do;
    BCe = ROL(Ago, 55);
    Aku ^= Du;
    BCi = ROL(Aku, 39);
    Ama ^= Da;
    BCo = ROL(Ama, 41);
    Ase ^= De;
    BCu = ROL(Ase, 2);
    Esa = BCa ^ ((~BCe) & BCi);
    Ese = BCe ^ ((~BCi) & BCo);
    Esi = BCi ^ ((~BCo) & BCu);
    Eso = BCo ^ ((~BCu) & BCa);
    Esu = BCu ^ ((~BCa) & BCe);

    /* prepareTheta */
    BCa = Eba ^ Ega ^ Eka ^ Ema ^ Esa;
    BCe = Ebe ^ Ege ^ Eke ^ Eme ^ Ese;
    BCi = Ebi ^ Egi ^ Eki ^ Emi ^ Esi;
    BCo = Ebo ^ Ego ^ Eko ^ Emo ^ Eso;
    BCu = Ebu ^ Egu ^ Eku ^ Emu ^ Esu;

    /* thetaRhoPiChiIotaPrepareTheta(round+1, E, A) */
    Da = BCu ^ ROL(BCe, 1);
    De = BCa ^ ROL(BCi, 1);
    Di = BCe ^ ROL(BCo, 1);
    Do = BCi ^ ROL(BCu, 1);
    Du = BCo ^ ROL(BCa, 1);

    Eba ^= Da;
    BCa = Eba;
    Ege ^= De;
    BCe = ROL(Ege, 44);
    Eki ^= Di;
    BCi = ROL(Eki, 43);
    Emo ^= Do;
    BCo = ROL(Emo, 21);
    Esu ^= Du;
    BCu = ROL(Esu, 14);
    Aba = BCa ^ ((~BCe) & BCi);
    Aba ^= (uint64_t)KeccakF_RoundConstants[round + 1];
    Abe = BCe ^ ((~BCi) & BCo);
    Abi = BCi ^ ((~BCo) & BCu);
    Abo = BCo ^ ((~BCu) & BCa);
    Abu = BCu ^ ((~BCa) & BCe);

    Ebo ^= Do;
    BCa = ROL(Ebo, 28);
    Egu ^= Du;
    BCe = ROL(Egu, 20);
    Eka ^= Da;
    BCi = ROL(Eka, 3);
    Eme ^= De;
    BCo = ROL(Eme, 45);
    Esi ^= Di;
    BCu = ROL(Esi, 61);
    Aga = BCa ^ ((~BCe) & BCi);
    Age = BCe ^ ((~BCi) & BCo);
    Agi = BCi ^ ((~BCo) & BCu);
    Ago = BCo ^ ((~BCu) & BCa);
    Agu = BCu ^ ((~BCa) & BCe);

    Ebe ^= De;
    BCa = ROL(Ebe, 1);
    Egi ^= Di;
    BCe = ROL(Egi, 6);
    Eko ^= Do;
    BCi = ROL(Eko, 25);
    Emu ^= Du;
    BCo = ROL(Emu, 8);
    Esa ^= Da;
    BCu = ROL(Esa, 18);
    Aka = BCa ^ ((~BCe) & BCi);
    Ake = BCe ^ ((~BCi) & BCo);
    Aki = BCi ^ ((~BCo) & BCu);
    Ako = BCo ^ ((~BCu) & BCa);
    Aku = BCu ^ ((~BCa) & BCe);

    Ebu ^= Du;
    BCa = ROL(Ebu, 27);
    Ega ^= Da;
    BCe = ROL(Ega, 36);
    Eke ^= De;
    BCi = ROL(Eke, 10);
    Emi ^= Di;
    BCo = ROL(Emi, 15);
    Eso ^= Do;
    BCu = ROL(Eso, 56);
    Ama = BCa ^ ((~BCe) & BCi);
    Ame = BCe ^ ((~BCi) & BCo);
    Ami = BCi ^ ((~BCo) & BCu);
    Amo = BCo ^ ((~BCu) & BCa);
    Amu = BCu ^ ((~BCa) & BCe);

    Ebi ^= Di;
    BCa = ROL(Ebi, 62);
    Ego ^= Do;
    BCe = ROL(Ego, 55);
    Eku ^= Du;
    BCi = ROL(Eku, 39);
    Ema ^= Da;
    BCo = ROL(Ema, 41);
    Ese ^= De;
    BCu = ROL(Ese, 2);
    Asa = BCa ^ ((~BCe) & BCi);
    Ase = BCe ^ ((~BCi) & BCo);
    Asi = BCi ^ ((~BCo) & BCu);
    Aso = BCo ^ ((~BCu) & BCa);
    Asu = BCu ^ ((~BCa) & BCe);
  }

  /* copyToState(state, A) */
  state[0] = Aba;
  state[1] = Abe;
  state[2] = Abi;
  state[3] = Abo;
  state[4] = Abu;
  state[5] = Aga;
  state[6] = Age;
  state[7] = Agi;
  state[8] = Ago;
  state[9] = Agu;
  state[10] = Aka;
  state[11] = Ake;
  state[12] = Aki;
  state[13] = Ako;
  state[14] = Aku;
  state[15] = Ama;
  state[16] = Ame;
  state[17] = Ami;
  state[18] = Amo;
  state[19] = Amu;
  state[20] = Asa;
  state[21] = Ase;
  state[22] = Asi;
  state[23] = Aso;
  state[24] = Asu;
}

/*************************************************
 * Name:        keccak_init
 *
 * Description: Initializes the Keccak state.
 *
 * Arguments:   - uint64_t *s: pointer to Keccak state
 **************************************************/
static void keccak_init(uint64_t s[MLD_KECCAK_LANES])
__contract__(
  requires(memory_no_alias(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  assigns(memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
)
{
  unsigned int i;
  for (i = 0; i < MLD_KECCAK_LANES; i++)
  __loop__(
    invariant(i <= MLD_KECCAK_LANES)
    invariant(forall(k, 0, i, s[k] == 0))
  )
  {
    s[i] = 0;
  }

  cassert(forall(k, 0, MLD_KECCAK_LANES, s[k] == 0));
}

/*************************************************
 * Name:        keccak_absorb
 *
 * Description: Absorb step of Keccak; incremental.
 *
 * Arguments:   - uint64_t *s: pointer to Keccak state
 *              - unsigned int pos: position in current block to be absorbed
 *              - unsigned int r: rate in bytes (e.g., 168 for SHAKE128)
 *              - const uint8_t *in: pointer to input to be absorbed into s
 *              - size_t inlen: length of input in bytes
 *
 * Returns new position pos in current block
 **************************************************/
static unsigned int keccak_absorb(uint64_t s[MLD_KECCAK_LANES],
                                  unsigned int pos, unsigned int r,
                                  const uint8_t *in, size_t inlen)
__contract__(
  requires(r < sizeof(uint64_t) * MLD_KECCAK_LANES)
  requires(pos <= r)
  requires(memory_no_alias(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  requires(memory_no_alias(in, inlen))
  assigns(memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  ensures(return_value < r))
{
  unsigned int i;

  while (pos + inlen >= r)
  __loop__(
    assigns(pos, i, in, inlen,
      memory_slice(s, sizeof(uint64_t) *  MLD_KECCAK_LANES))
    invariant(inlen <= loop_entry(inlen))
    invariant(pos <= r)
    invariant(in == loop_entry(in) + (loop_entry(inlen) - inlen)))
  {
    for (i = pos; i < r; i++)
    __loop__(
      assigns(i, in, memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
      invariant(i >= pos && i <= r)
      invariant(in == loop_entry(in) + (i - pos)))
    {
      s[i / 8] ^= (uint64_t)*in++ << 8 * (i % 8);
    }
    inlen -= r - pos;
    KeccakF1600_StatePermute(s);
    pos = 0;
  }

  for (i = pos; i < pos + inlen; i++)
  __loop__(
    invariant(i >= pos && i - pos <= inlen)
    invariant(in == loop_entry(in) + (i - pos)))
  {
    s[i / 8] ^= (uint64_t)*in++ << 8 * (i % 8);
  }

  return i;
}

/*************************************************
 * Name:        keccak_finalize
 *
 * Description: Finalize absorb step.
 *
 * Arguments:   - uint64_t *s: pointer to Keccak state
 *              - unsigned int pos: position in current block to be absorbed
 *              - unsigned int r: rate in bytes (e.g., 168 for SHAKE128)
 *              - uint8_t p: domain separation byte
 **************************************************/
static void keccak_finalize(uint64_t s[MLD_KECCAK_LANES], unsigned int pos,
                            unsigned int r, uint8_t p)
__contract__(
  requires(pos <= r && r < sizeof(uint64_t) * MLD_KECCAK_LANES)
  requires((r / 8) >= 1)
  requires(memory_no_alias(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  assigns(memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
)
{
  s[pos / 8] ^= (uint64_t)p << 8 * (pos % 8);
  s[r / 8 - 1] ^= 1ULL << 63;
}

/*************************************************
 * Name:        keccak_squeeze
 *
 * Description: Squeeze step of Keccak. Squeezes arbitratrily many bytes.
 *              Modifies the state. Can be called multiple times to keep
 *              squeezing, i.e., is incremental.
 *
 * Arguments:   - uint8_t *out: pointer to output
 *              - size_t outlen: number of bytes to be squeezed (written to out)
 *              - uint64_t *s: pointer to input/output Keccak state
 *              - unsigned int pos: number of bytes in current block already
 *squeezed
 *              - unsigned int r: rate in bytes (e.g., 168 for SHAKE128)
 *
 * Returns new position pos in current block
 **************************************************/
static unsigned int keccak_squeeze(uint8_t *out, size_t outlen,
                                   uint64_t s[MLD_KECCAK_LANES],
                                   unsigned int pos, unsigned int r)
__contract__(
  requires((r == SHAKE128_RATE && pos <= SHAKE128_RATE) ||
    (r == SHAKE256_RATE && pos <= SHAKE256_RATE))
  requires(outlen <= 8 * r /* somewhat arbitrary bound */)
  requires(memory_no_alias(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  requires(memory_no_alias(out, outlen))
  assigns(memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  assigns(memory_slice(out, outlen))
  ensures(return_value <= r))
{
  unsigned int i;
  uint64_t lane;
  uint8_t byte;

  while (outlen > 0)
  __loop__(
    assigns(i, outlen, pos, out, memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES), memory_slice(out, outlen))
    invariant(outlen <= loop_entry(outlen))
    invariant(out == loop_entry(out) + (loop_entry(outlen) - outlen))
    invariant(pos <= r)
  )
  {
    if (pos == r)
    {
      KeccakF1600_StatePermute(s);
      pos = 0;
    }
    for (i = pos; i < r && i < pos + outlen; i++)
    __loop__(
      assigns(i, out, memory_slice(out, outlen))
      invariant(i >= pos && i <= r && i <= pos + outlen)
      invariant(out == loop_entry(out) + (i - pos))
    )
    {
      lane = s[i / 8];
      byte = (uint8_t)((lane >> (8 * (i % 8))) & 0xFF);
      *out++ = byte;
    }
    outlen -= i - pos;
    pos = i;
  }

  return pos;
}

/*************************************************
 * Name:        keccak_absorb_once
 *
 * Description: Absorb step of Keccak;
 *              non-incremental, starts by zeroeing the state.
 *
 * Arguments:   - uint64_t *s: pointer to (uninitialized) output Keccak state
 *              - const unsigned int r: rate in bytes (e.g., 168 for SHAKE128)
 *              - const uint8_t *in: pointer to input to be absorbed into s
 *              - size_t inlen: length of input in bytes
 *              - uint8_t p: domain-separation byte for different Keccak-derived
 *functions
 **************************************************/
static void keccak_absorb_once(uint64_t s[MLD_KECCAK_LANES],
                               const unsigned int r, const uint8_t *in,
                               size_t inlen, uint8_t p)
__contract__(
  requires(r <= sizeof(uint64_t) * MLD_KECCAK_LANES)
  requires(memory_no_alias(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  requires(memory_no_alias(in, inlen))
  assigns(memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES)))
{
  unsigned int i;

  for (i = 0; i < MLD_KECCAK_LANES; i++)
  __loop__(invariant(i <= MLD_KECCAK_LANES))
  {
    s[i] = 0;
  }

  while (inlen >= r)
  __loop__(
    invariant(inlen <= loop_entry(inlen))
    invariant(in == loop_entry(in) + (loop_entry(inlen) - inlen)))
  {
    for (i = 0; i < r / 8; i++)
    __loop__(invariant(i <= r / 8))
    {
      s[i] ^= load64(in + 8 * i);
    }
    in += r;
    inlen -= r;
    KeccakF1600_StatePermute(s);
  }

  for (i = 0; i < inlen; i++)
  __loop__(invariant(i <= inlen))
  {
    s[i / 8] ^= (uint64_t)in[i] << 8 * (i % 8);
  }

  s[i / 8] ^= (uint64_t)p << 8 * (i % 8);
  s[(r - 1) / 8] ^= 1ULL << 63;
}

/*************************************************
 * Name:        keccakf1600_extract_bytes
 *
 * Description: Extracts a specified number of bytes from the state
 *              buffer starting at offset and stores them in the provided
 *              data buffer
 *
 * Arguments:   - uint64_t *state: pointer to the state array
 *              - unsigned char *data: pointer to the output byte array
 *(allocated)
 *              - unsigned offset: starting byte position in the state
 *              - unsigned length: number of bytes to extract
 **************************************************/
static void keccakf1600_extract_bytes(uint64_t *state, unsigned char *data,
                                      unsigned offset, unsigned length)
__contract__(
  requires(0 <= offset && offset <= MLD_KECCAK_LANES * sizeof(uint64_t) &&
      0 <= length && length <= MLD_KECCAK_LANES * sizeof(uint64_t) - offset)
  requires(memory_no_alias(state, sizeof(uint64_t) * MLD_KECCAK_LANES))
  requires(memory_no_alias(data, length))
  assigns(memory_slice(data, length)))
{
  unsigned i;
#if defined(MLD_SYS_LITTLE_ENDIAN)
  uint8_t *state_ptr = (uint8_t *)state + offset;
  for (i = 0; i < length; i++)
  __loop__(invariant(i <= length))
  {
    data[i] = state_ptr[i];
  }
#else  /* MLD_SYS_LITTLE_ENDIAN */
  /* Portable version */
  for (i = 0; i < length; i++)
  __loop__(invariant(i <= length))
  {
    data[i] = (state[(offset + i) >> 3] >> (8 * ((offset + i) & 0x07))) & 0xFF;
  }
#endif /* !MLD_SYS_LITTLE_ENDIAN */
}

/*************************************************
 * Name:        keccak_squeezeblocks
 *
 * Description: Squeeze step of Keccak. Squeezes full blocks of r bytes each.
 *              Modifies the state. Can be called multiple times to keep
 *              squeezing, i.e., is incremental. Assumes zero bytes of current
 *              block have already been squeezed.
 *
 * Arguments:   - uint8_t *out: pointer to output blocks
 *              - size_t nblocks: number of blocks to be squeezed (written to
 *out)
 *              - uint64_t *s: pointer to input/output Keccak state
 *              - unsigned int r: rate in bytes (e.g., 168 for SHAKE128)
 **************************************************/
static void keccak_squeezeblocks(uint8_t *out, size_t nblocks,
                                 uint64_t s[MLD_KECCAK_LANES], unsigned int r)
__contract__(
  requires(r == SHAKE128_RATE || r == SHAKE256_RATE)
  requires(nblocks <= 8 /* somewhat arbitrary bound */)
  requires(memory_no_alias(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  requires(memory_no_alias(out, nblocks * r))
  assigns(memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES))
  assigns(memory_slice(out, nblocks * r)))
{
  while (nblocks > 0)
  __loop__(
    assigns(out, nblocks, memory_slice(s, sizeof(uint64_t) * MLD_KECCAK_LANES),
      memory_slice(out, nblocks * r))
    invariant(nblocks <= loop_entry(nblocks))
    invariant(out == loop_entry(out) + r * (loop_entry(nblocks) - nblocks))
  )
  {
    KeccakF1600_StatePermute(s);
    keccakf1600_extract_bytes(s, out, 0, r);
    out += r;
    nblocks -= 1;
  }
}

/*************************************************
 * Name:        shake128_init
 *
 * Description: Initilizes Keccak state for use as SHAKE128 XOF
 *
 * Arguments:   - keccak_state *state: pointer to (uninitialized) Keccak state
 **************************************************/
void shake128_init(keccak_state *state)
{
  keccak_init(state->s);
  state->pos = 0;
}

/*************************************************
 * Name:        shake128_absorb
 *
 * Description: Absorb step of the SHAKE128 XOF; incremental.
 *
 * Arguments:   - keccak_state *state: pointer to (initialized) output Keccak
 *state
 *              - const uint8_t *in: pointer to input to be absorbed into s
 *              - size_t inlen: length of input in bytes
 **************************************************/
void shake128_absorb(keccak_state *state, const uint8_t *in, size_t inlen)
{
  state->pos = keccak_absorb(state->s, state->pos, SHAKE128_RATE, in, inlen);
}

/*************************************************
 * Name:        shake128_finalize
 *
 * Description: Finalize absorb step of the SHAKE128 XOF.
 *
 * Arguments:   - keccak_state *state: pointer to Keccak state
 **************************************************/
void shake128_finalize(keccak_state *state)
{
  keccak_finalize(state->s, state->pos, SHAKE128_RATE, 0x1F);
  state->pos = SHAKE128_RATE;
}

/*************************************************
 * Name:        shake128_squeeze
 *
 * Description: Squeeze step of SHAKE128 XOF. Squeezes arbitraily many
 *              bytes. Can be called multiple times to keep squeezing.
 *
 * Arguments:   - uint8_t *out: pointer to output blocks
 *              - size_t outlen : number of bytes to be squeezed (written to
 *output)
 *              - keccak_state *s: pointer to input/output Keccak state
 **************************************************/
void shake128_squeeze(uint8_t *out, size_t outlen, keccak_state *state)
{
  state->pos = keccak_squeeze(out, outlen, state->s, state->pos, SHAKE128_RATE);
}

/*************************************************
 * Name:        shake128_absorb_once
 *
 * Description: Initialize, absorb into and finalize SHAKE128 XOF;
 *non-incremental.
 *
 * Arguments:   - keccak_state *state: pointer to (uninitialized) output Keccak
 *state
 *              - const uint8_t *in: pointer to input to be absorbed into s
 *              - size_t inlen: length of input in bytes
 **************************************************/
void shake128_absorb_once(keccak_state *state, const uint8_t *in, size_t inlen)
{
  keccak_absorb_once(state->s, SHAKE128_RATE, in, inlen, 0x1F);
  state->pos = SHAKE128_RATE;
}

/*************************************************
 * Name:        shake128_squeezeblocks
 *
 * Description: Squeeze step of SHAKE128 XOF. Squeezes full blocks of
 *              SHAKE128_RATE bytes each. Can be called multiple times
 *              to keep squeezing. Assumes new block has not yet been
 *              started (state->pos = SHAKE128_RATE).
 *
 * Arguments:   - uint8_t *out: pointer to output blocks
 *              - size_t nblocks: number of blocks to be squeezed (written to
 *output)
 *              - keccak_state *s: pointer to input/output Keccak state
 **************************************************/
void shake128_squeezeblocks(uint8_t *out, size_t nblocks, keccak_state *state)
{
  keccak_squeezeblocks(out, nblocks, state->s, SHAKE128_RATE);
}

/*************************************************
 * Name:        shake256_init
 *
 * Description: Initilizes Keccak state for use as SHAKE256 XOF
 *
 * Arguments:   - keccak_state *state: pointer to (uninitialized) Keccak state
 **************************************************/
void shake256_init(keccak_state *state)
{
  keccak_init(state->s);
  state->pos = 0;
}

/*************************************************
 * Name:        shake256_absorb
 *
 * Description: Absorb step of the SHAKE256 XOF; incremental.
 *
 * Arguments:   - keccak_state *state: pointer to (initialized) output Keccak
 *state
 *              - const uint8_t *in: pointer to input to be absorbed into s
 *              - size_t inlen: length of input in bytes
 **************************************************/
void shake256_absorb(keccak_state *state, const uint8_t *in, size_t inlen)
{
  state->pos = keccak_absorb(state->s, state->pos, SHAKE256_RATE, in, inlen);
}

/*************************************************
 * Name:        shake256_finalize
 *
 * Description: Finalize absorb step of the SHAKE256 XOF.
 *
 * Arguments:   - keccak_state *state: pointer to Keccak state
 **************************************************/
void shake256_finalize(keccak_state *state)
{
  keccak_finalize(state->s, state->pos, SHAKE256_RATE, 0x1F);
  state->pos = SHAKE256_RATE;
}

/*************************************************
 * Name:        shake256_squeeze
 *
 * Description: Squeeze step of SHAKE256 XOF. Squeezes arbitraily many
 *              bytes. Can be called multiple times to keep squeezing.
 *
 * Arguments:   - uint8_t *out: pointer to output blocks
 *              - size_t outlen : number of bytes to be squeezed (written to
 *output)
 *              - keccak_state *s: pointer to input/output Keccak state
 **************************************************/
void shake256_squeeze(uint8_t *out, size_t outlen, keccak_state *state)
{
  state->pos = keccak_squeeze(out, outlen, state->s, state->pos, SHAKE256_RATE);
}

/*************************************************
 * Name:        shake256_absorb_once
 *
 * Description: Initialize, absorb into and finalize SHAKE256 XOF;
 *non-incremental.
 *
 * Arguments:   - keccak_state *state: pointer to (uninitialized) output Keccak
 *state
 *              - const uint8_t *in: pointer to input to be absorbed into s
 *              - size_t inlen: length of input in bytes
 **************************************************/
void shake256_absorb_once(keccak_state *state, const uint8_t *in, size_t inlen)
{
  keccak_absorb_once(state->s, SHAKE256_RATE, in, inlen, 0x1F);
  state->pos = SHAKE256_RATE;
}

/*************************************************
 * Name:        shake256_squeezeblocks
 *
 * Description: Squeeze step of SHAKE256 XOF. Squeezes full blocks of
 *              SHAKE256_RATE bytes each. Can be called multiple times
 *              to keep squeezing. Assumes next block has not yet been
 *              started (state->pos = SHAKE256_RATE).
 *
 * Arguments:   - uint8_t *out: pointer to output blocks
 *              - size_t nblocks: number of blocks to be squeezed (written to
 *output)
 *              - keccak_state *s: pointer to input/output Keccak state
 **************************************************/
void shake256_squeezeblocks(uint8_t *out, size_t nblocks, keccak_state *state)
{
  keccak_squeezeblocks(out, nblocks, state->s, SHAKE256_RATE);
}

/*************************************************
 * Name:        shake128
 *
 * Description: SHAKE128 XOF with non-incremental API
 *
 * Arguments:   - uint8_t *out: pointer to output
 *              - size_t outlen: requested output length in bytes
 *              - const uint8_t *in: pointer to input
 *              - size_t inlen: length of input in bytes
 **************************************************/
void shake128(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
  size_t nblocks;
  keccak_state state;

  shake128_absorb_once(&state, in, inlen);
  nblocks = outlen / SHAKE128_RATE;
  shake128_squeezeblocks(out, nblocks, &state);
  outlen -= nblocks * SHAKE128_RATE;
  out += nblocks * SHAKE128_RATE;
  shake128_squeeze(out, outlen, &state);
}

/*************************************************
 * Name:        shake256
 *
 * Description: SHAKE256 XOF with non-incremental API
 *
 * Arguments:   - uint8_t *out: pointer to output
 *              - size_t outlen: requested output length in bytes
 *              - const uint8_t *in: pointer to input
 *              - size_t inlen: length of input in bytes
 **************************************************/
void shake256(uint8_t *out, size_t outlen, const uint8_t *in, size_t inlen)
{
  size_t nblocks;
  keccak_state state;

  shake256_absorb_once(&state, in, inlen);
  nblocks = outlen / SHAKE256_RATE;
  shake256_squeezeblocks(out, nblocks, &state);
  outlen -= nblocks * SHAKE256_RATE;
  out += nblocks * SHAKE256_RATE;
  shake256_squeeze(out, outlen, &state);
}

/*************************************************
 * Name:        sha3_256
 *
 * Description: SHA3-256 with non-incremental API
 *
 * Arguments:   - uint8_t *h: pointer to output (32 bytes)
 *              - const uint8_t *in: pointer to input
 *              - size_t inlen: length of input in bytes
 **************************************************/
void sha3_256(uint8_t h[SHA3_256_HASHBYTES], const uint8_t *in, size_t inlen)
{
  unsigned int i;
  uint64_t s[MLD_KECCAK_LANES];

  keccak_absorb_once(s, SHA3_256_RATE, in, inlen, 0x06);
  KeccakF1600_StatePermute(s);
  for (i = 0; i < 4; i++)
  __loop__(invariant(i <= 4))
  {
    store64(h + 8 * i, s[i]);
  }
}

/*************************************************
 * Name:        sha3_512
 *
 * Description: SHA3-512 with non-incremental API
 *
 * Arguments:   - uint8_t *h: pointer to output (64 bytes)
 *              - const uint8_t *in: pointer to input
 *              - size_t inlen: length of input in bytes
 **************************************************/
void sha3_512(uint8_t h[SHA3_512_HASHBYTES], const uint8_t *in, size_t inlen)
{
  unsigned int i;
  uint64_t s[MLD_KECCAK_LANES];

  keccak_absorb_once(s, SHA3_512_RATE, in, inlen, 0x06);
  KeccakF1600_StatePermute(s);
  for (i = 0; i < 8; i++)
  __loop__(invariant(i <= 8))
  {
    store64(h + 8 * i, s[i]);
  }
}
