#define NAPI_EXPERIMENTAL
#include <node_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "burstmath.c"

CalcDeadlineRequest buildDeadlineRequest(napi_env env, napi_value accountIdNapi, napi_value nonceNapi, napi_value scoopNrNapi, napi_value baseTargetNapi, napi_value genSigNapi);

napi_value calculate_scoop_wrapper(napi_env env, napi_callback_info info) {
  napi_status status;
  size_t argc = 2;
  napi_value argv[2];
  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  if (status != napi_ok) {
      napi_throw_error(env, NULL, "Failed to parse arguments");
  }

  uint32_t arg0;
  status = napi_get_value_uint32(env, argv[0], &arg0);

  if (status != napi_ok) {
      napi_throw_error(env, NULL, "No valid height provided");
  }

  uint8_t *arg1;
  size_t len = 32;
  status = napi_get_buffer_info(env, argv[1], (void**) &arg1, &len);

  if (status != napi_ok) {
      napi_throw_error(env, NULL, "No valid buffer gensig provided");
  }

  napi_value scoop;
  status = napi_create_uint32(env, calculate_scoop((uint64_t) arg0, arg1), &scoop);

  return scoop;
}

napi_value calculate_deadline_wrapper(napi_env env, napi_callback_info info) {
  napi_status status;
  size_t argc = 5;
  napi_value argv[5];
  status = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);

  CalcDeadlineRequest req = buildDeadlineRequest(env, argv[0], argv[1], argv[2], argv[3], argv[4]);

  calculate_deadline(&req);

  napi_value dl;
  status = napi_create_bigint_uint64(env, req.deadline, &dl);

  return dl;
}

CalcDeadlineRequest buildDeadlineRequest(napi_env env, napi_value accountIdNapi, napi_value nonceNapi, napi_value scoopNrNapi, napi_value baseTargetNapi, napi_value genSigNapi) {
      napi_status status;
      char *accountIdString;
      size_t result;
      status = napi_get_value_string_utf8(env, accountIdNapi, NULL, 0, &result);
      accountIdString = (char*)malloc(result + 1);
      status = napi_get_value_string_utf8(env, accountIdNapi, accountIdString, result + 1, &result);

      if (status != napi_ok) {
          free(accountIdString);
          napi_throw_error(env, NULL, "No valid accountId provided");
      }

      uint64_t accountId;
      sscanf(accountIdString, "%" SCNu64, &accountId);
      free(accountIdString);

      char *nonceString;
      status = napi_get_value_string_utf8(env, nonceNapi, NULL, 0, &result);
      nonceString = (char*)malloc(result + 1);
      status = napi_get_value_string_utf8(env, nonceNapi, nonceString, result + 1, &result);

      if (status != napi_ok) {
         free(nonceString);
         napi_throw_error(env, NULL, "No valid nonce provided");
      }

      uint64_t nonce;
      sscanf(nonceString, "%" SCNu64, &nonce);
      free(nonceString);

      uint32_t scoopNr;
      status = napi_get_value_uint32(env, scoopNrNapi, &scoopNr);

      if (status != napi_ok) {
          napi_throw_error(env, NULL, "No valid scoopNr provided");
      }

      char *baseTargetString;
      status = napi_get_value_string_utf8(env, baseTargetNapi, NULL, 0, &result);
      baseTargetString = (char*)malloc(result + 1);
      status = napi_get_value_string_utf8(env, baseTargetNapi, baseTargetString, result + 1, &result);

      if (status != napi_ok) {
         free(baseTargetString);
         napi_throw_error(env, NULL, "No valid baseTarget provided");
      }

      uint64_t baseTarget;
      sscanf(baseTargetString, "%" SCNu64, &baseTarget);
      free(baseTargetString);

      uint8_t *genSig;
      size_t len = 32;
      status = napi_get_buffer_info(env, genSigNapi, (void**) &genSig, &len);

      if (status != napi_ok) {
          napi_throw_error(env, NULL, "No valid buffer gensig provided");
      }

      CalcDeadlineRequest req;
      req.account_id = accountId;
      req.nonce = nonce;
      req.scoop_nr = scoopNr;
      req.base_target = baseTarget;
      req.gen_sig = genSig;
      req.deadline = 0;

      return req;
}

napi_value Init(napi_env env, napi_value exports) {
    napi_status status;
    napi_value fn;

    status = napi_create_function(env, NULL, 0, calculate_scoop_wrapper, NULL, &fn);
    if (status != napi_ok) {
        napi_throw_error(env, NULL, "Unable to wrap native function");
    }

    status = napi_set_named_property(env, exports, "calculate_scoop", fn);
    if (status != napi_ok) {
        napi_throw_error(env, NULL, "Unable to populate exports");
    }

    status = napi_create_function(env, NULL, 0, calculate_deadline_wrapper, NULL, &fn);
    if (status != napi_ok) {
        napi_throw_error(env, NULL, "Unable to wrap native function");
    }

    status = napi_set_named_property(env, exports, "calculate_deadline", fn);
    if (status != napi_ok) {
        napi_throw_error(env, NULL, "Unable to populate exports");
    }

    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
