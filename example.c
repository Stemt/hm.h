#include <stdio.h>
#include <stdint.h>

#define HM_IMPLEMENTATION
#include "hm.h"

HM_GEN_WRAPPER_PROTOTYPE(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

int main(void){
  HM hm = {0};
  
  // --- raw API ---
  {
    HM_init(&hm, sizeof(int), 0);

    int val = 2;
    HM_set(&hm, "test", &val);
    
    int* res = (int*)HM_get(&hm, "test");
    if(res != NULL){
      printf("res: %d\n", *res);
    }else{
      printf("res: not found\n");
    }

    HM_deinit(&hm);
  }

  // --- wrapped API ---
  {
    HM_int_init(&hm, 0);

    HM_int_set(&hm, "test", 2);
    
    int* res = HM_int_get(&hm, "test");
    if(res != NULL){
      printf("res: %d\n", *res);
    }else{
      printf("res: not found\n");
    }

    HM_deinit(&hm);
  }

  // --- iterating --- 
  {
    HM_int_init(&hm, 0);

    for(int i = 0; i < 10; ++i){
      char key[1024] = {0};
      snprintf(key, sizeof(key)-1, "key-%d", i);
      HM_int_set(&hm, key, i);
    }

    for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
      printf("key: '%s', value '%d'\n", HM_key_at(&hm, i), *HM_int_value_at(&hm, i));
    }
    printf("\n");

    HM_remove(&hm, "key-0");
    HM_remove(&hm, "key-5");
    HM_remove(&hm, "key-9");
    
    for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
      printf("key: '%s', value '%d'\n", HM_key_at(&hm, i), *HM_int_value_at(&hm, i));
    }

    HM_deinit(&hm);
  }

  return 0;
}
