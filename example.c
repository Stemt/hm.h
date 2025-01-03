#include <stdio.h>
#include <stdint.h>

#define HM_IMPLEMENTATION
#include "hm.h"

HM_GEN_WRAPPER_DEF(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

int main(void){
  HM hm = {0};
  
  // --- raw API ---
  HM_init(&hm, sizeof(int), 0);

  int val = 2;
  HM_set(&hm, "test", &val);
  printf("res: %d\n", *(int*)HM_get(&hm, "test"));

  HM_deinit(&hm);

  // --- wrapped API ---
  HM_int_init(&hm, 0);

  HM_int_set(&hm, "test", 2);
  printf("res: %d\n", *HM_int_get(&hm, "test"));

  HM_deinit(&hm);

  // --- iterating --- 
  HM_int_init(&hm, 0);

  for(int i = 0; i < 10; ++i){
    char key[1024] = {0};
    snprintf(key, sizeof(key)-1, "key-%d", i);
    HM_int_set(&hm, key, i);
  }

  for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
    printf("key: '%s', value '%d'\n", HM_int_key_at(&hm, i), *HM_int_value_at(&hm, i));
  }

  HM_deinit(&hm);

  return 0;
}
