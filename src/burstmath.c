#define USE_MULTI_SHABAL
#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <stddef.h>
#include <string.h>

#include "swap.h"
#include "burstmath.h"
#include "mshabal.h"
#include "mshabal256.h"
#include "sph_shabal.h"

#define SCOOP_SIZE 64
#define NUM_SCOOPS 4096
#define NONCE_SIZE (NUM_SCOOPS * SCOOP_SIZE)

#define HASH_SIZE 32
#define HASH_CAP 4096

#define SSE4_PARALLEL 4
#define AVX2_PARALLEL 8

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

void calculate_deadlines_sse4(CalcDeadlineRequest **reqs){
  char finals[SSE4_PARALLEL][32];
  char(*gendata)[16 + NONCE_SIZE] = (char(*)[16 + NONCE_SIZE])malloc(SSE4_PARALLEL * (16 + NONCE_SIZE) * sizeof(char));
  char *xv;

  for (int i = 0; i < SSE4_PARALLEL; i++) {
    SET_NONCE(gendata[i], reqs[i]->account_id, 0);
    SET_NONCE(gendata[i], reqs[i]->nonce, 8);
  }

  mshabal_context x;
  int len;

  for (int i = NONCE_SIZE; i > 0; i -= HASH_SIZE) {
    sse4_mshabal_init(&x, 256);

    len = NONCE_SIZE + 16 - i;
    if (len > HASH_CAP)
      len = HASH_CAP;

    sse4_mshabal(&x, &gendata[0][i], &gendata[1][i], &gendata[2][i], &gendata[3][i], len);
    sse4_mshabal_close(&x, 0, 0, 0, 0, 0, &gendata[0][i - HASH_SIZE], &gendata[1][i - HASH_SIZE],
                       &gendata[2][i - HASH_SIZE], &gendata[3][i - HASH_SIZE]);
  }

  sse4_mshabal_init(&x, 256);
  sse4_mshabal(&x, gendata[0], gendata[1], gendata[2], gendata[3], 16 + NONCE_SIZE);
  sse4_mshabal_close(&x, 0, 0, 0, 0, 0, finals[0], finals[1], finals[2], finals[3]);

  // XOR with final
  for (int i = 0; i < NONCE_SIZE; i++)
    for (int j = 0; j < SSE4_PARALLEL; j++)
      gendata[j][i] ^= (finals[j][i % 32]);


  mshabal_context deadline_sc;
  sse4_mshabal_init(&deadline_sc, 256);
  sse4_mshabal(&deadline_sc, reqs[0]->gen_sig, reqs[1]->gen_sig, reqs[2]->gen_sig, reqs[3]->gen_sig, HASH_SIZE);

  uint8_t scoops[SSE4_PARALLEL][SCOOP_SIZE];
  for (int i = 0; i < SSE4_PARALLEL; i++) {
    memcpy(scoops[i], gendata[i] + (reqs[i]->scoop_nr * SCOOP_SIZE), 32);
    memcpy(scoops[i] + 32, gendata[i] + ((4095 - reqs[i]->scoop_nr) * SCOOP_SIZE) + 32, 32);
  }

  free(gendata);

  uint8_t finals2[SSE4_PARALLEL][HASH_SIZE];
  sse4_mshabal(&deadline_sc, scoops[0], scoops[1], scoops[2], scoops[3], SCOOP_SIZE);
  sse4_mshabal_close(&deadline_sc, 0, 0, 0, 0, 0, (uint32_t *)finals2[0], (uint32_t *)finals2[1],
                     (uint32_t *)finals2[2], (uint32_t *)finals2[3]);

  for (int i = 0; i < SSE4_PARALLEL; i++)
    reqs[i]->deadline = *(uint64_t *)finals2[i] / reqs[i]->base_target;
}


