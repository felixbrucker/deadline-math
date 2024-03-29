#define USE_MULTI_SHABAL
#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <stddef.h>
#include <string.h>

#include "swap.h"
#include "burstmath.h"
#include "sph_shabal.h"

#define SCOOP_SIZE 64
#define NUM_SCOOPS 4096
#define NONCE_SIZE (NUM_SCOOPS * SCOOP_SIZE)

#define HASH_SIZE 32
#define HASH_CAP 4096

#define SET_NONCE(gendata, nonce, offset)                                      \
  xv = (char *)&nonce;                                                         \
  gendata[NONCE_SIZE + offset] = xv[7];                                        \
  gendata[NONCE_SIZE + offset + 1] = xv[6];                                    \
  gendata[NONCE_SIZE + offset + 2] = xv[5];                                    \
  gendata[NONCE_SIZE + offset + 3] = xv[4];                                    \
  gendata[NONCE_SIZE + offset + 4] = xv[3];                                    \
  gendata[NONCE_SIZE + offset + 5] = xv[2];                                    \
  gendata[NONCE_SIZE + offset + 6] = xv[1];                                    \
  gendata[NONCE_SIZE + offset + 7] = xv[0]

uint32_t calculate_scoop(uint64_t height, uint8_t *gensig) {
  sph_shabal_context sc;
  uint8_t new_gensig[32];

  sph_shabal256_init(&sc);
  sph_shabal256(&sc, gensig, 32);

  uint64_t height_swapped = bswap_64(height);
  sph_shabal256(&sc, (unsigned char *) &height_swapped, sizeof(height_swapped));
  sph_shabal256_close(&sc, new_gensig);

  return ((new_gensig[30] & 0x0F) << 8) | new_gensig[31];
}

void calculate_deadline(CalcDeadlineRequest *req) {
  char final[32];
  char gendata[16 + NONCE_SIZE];
  char *xv;

  SET_NONCE(gendata, req->account_id, 0);
  SET_NONCE(gendata, req->nonce, 8);

  sph_shabal_context x;
  int len;

  for (int i = NONCE_SIZE; i > 0; i -= HASH_SIZE) {
    sph_shabal256_init(&x);

    len = NONCE_SIZE + 16 - i;
    if (len > HASH_CAP)
      len = HASH_CAP;

    sph_shabal256(&x, (unsigned char *) &gendata[i], len);
    sph_shabal256_close(&x, &gendata[i - HASH_SIZE]);
  }

  sph_shabal256_init(&x);
  sph_shabal256(&x, (unsigned char *) gendata, 16 + NONCE_SIZE);
  sph_shabal256_close(&x, final);

  // XOR with final
  for (int i = 0; i < NONCE_SIZE; i++)
      gendata[i] ^= (final[i % 32]);

  sph_shabal_context deadline_sc;
  sph_shabal256_init(&deadline_sc);
  sph_shabal256(&deadline_sc, (unsigned char *) req->gen_sig, HASH_SIZE);

  uint8_t scoop[SCOOP_SIZE];
  memcpy(scoop, gendata + (req->scoop_nr * SCOOP_SIZE), 32);
  memcpy(scoop + 32, gendata + ((4095 - req->scoop_nr) * SCOOP_SIZE) + 32, 32);

  uint8_t finals2[HASH_SIZE];
  sph_shabal256(&deadline_sc, (unsigned char *) scoop, SCOOP_SIZE);
  sph_shabal256_close(&deadline_sc, (uint32_t *)finals2);

  req->deadline = *(uint64_t *)finals2 / req->base_target;
}