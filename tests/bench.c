#include <stdint.h>
#include <stdio.h>

#define HM_IMPLEMENTATION
#include "hm.h"

HM_GEN_WRAPPER_PROTOTYPE(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

#include "utest.h"

static HM hm = {0};

#define BENCH_INSERTIONS 0xFFFFF
UTEST(HM_Bench, int32_init){
  ASSERT_TRUE(HM_init(&hm, sizeof(int32_t), BENCH_INSERTIONS*2));
}

UTEST(HM_Bench, int32_set){
  for(uint32_t i = 0; i < BENCH_INSERTIONS; ++i){
    ASSERT_TRUE(HM_sk_set(&hm, i, &i));
  }
}

UTEST(HM_Bench, int32_get){
  for(uint32_t i = 0; i < BENCH_INSERTIONS; ++i){
    uint32_t* res = HM_sk_get(&hm, i);
    ASSERT_NE(res, NULL);
    ASSERT_EQ(*res, i);
  }
}

UTEST(HM_Bench, int32_remove){
  for(uint32_t i = 0; i < BENCH_INSERTIONS; ++i){
    HM_sk_remove(&hm, i);
  }
  ASSERT_EQ(hm.count, 0UL);
  HM_deinit(&hm);
}

UTEST_MAIN();
