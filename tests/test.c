#include <stdio.h>
#define HM_IMPLEMENTATION
#define HM_DISABLE_ALLOC_PANIC
#include "hm.h"

HM_GEN_WRAPPER_PROTOTYPE(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

#include "utest.h"

UTEST(HM_Basic, insertion){
  HM hm = {0};
  ASSERT_TRUE(HM_init(&hm, sizeof(int), 0));
  
  int value = 2;
  ASSERT_TRUE(HM_set(&hm, "test", &value));
  
  ASSERT_NE(HM_get(&hm, "test"), NULL);
  ASSERT_EQ(*(int*)HM_get(&hm, "test"), 2);
}

UTEST(HM_Basic, removal){
  HM hm = {0};
  ASSERT_TRUE(HM_init(&hm, sizeof(int), 0));
  
  int value = 2;
  ASSERT_TRUE(HM_set(&hm, "test", &value));
  
  HM_remove(&hm, "test");
  ASSERT_EQ(HM_get(&hm, "test"), NULL);
}

UTEST(HM_Basic, resize){
  HM hm = {0};
  ASSERT_TRUE(HM_int_init(&hm, 2));
  ASSERT_EQ(hm.capacity, 2ULL);

  ASSERT_TRUE(HM_int_set(&hm, "key-1", 1));
  ASSERT_TRUE(HM_int_set(&hm, "key-2", 2));
  ASSERT_TRUE(HM_int_set(&hm, "key-3", 3));

  ASSERT_GE(hm.capacity, 3ULL);
}

UTEST(HM_Iteration, iterate){
  HM hm = {0};
  HM_int_init(&hm, 0);

  for(int i = 0; i < 10; ++i){
    char key[1024] = {0};
    snprintf(key, sizeof(key)-1, "key-%d", i);
    HM_int_set(&hm, key, i);
  }

  int count = 0;
  for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
    count++;
  }
  ASSERT_EQ(count, 10);
}

UTEST(HM_Iteration, remove_first){
  HM hm = {0};
  HM_int_init(&hm, 0);

  for(int i = 0; i < 10; ++i){
    char key[1024] = {0};
    snprintf(key, sizeof(key)-1, "key-%d", i);
    HM_int_set(&hm, key, i);
  }

  HM_remove(&hm, "key-0");

  int count = 0;
  for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
    ASSERT_STRNE(HM_key_at(&hm, i), "key-0");
    count++;
  }
  ASSERT_EQ(count, 9);
}

UTEST(HM_Iteration, remove_last){
  HM hm = {0};
  HM_int_init(&hm, 0);

  for(int i = 0; i < 10; ++i){
    char key[1024] = {0};
    snprintf(key, sizeof(key)-1, "key-%d", i);
    HM_int_set(&hm, key, i);
  }

  HM_remove(&hm, "key-9");

  int count = 0;
  for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
    ASSERT_STRNE(HM_key_at(&hm, i), "key-9");
    count++;
  }
  ASSERT_EQ(count, 9);
}

UTEST(HM_Iteration, remove_middle){
  HM hm = {0};
  HM_int_init(&hm, 0);

  for(int i = 0; i < 10; ++i){
    char key[1024] = {0};
    snprintf(key, sizeof(key)-1, "key-%d", i);
    HM_int_set(&hm, key, i);
  }

  HM_remove(&hm, "key-4");

  int count = 0;
  for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
    ASSERT_STRNE(HM_key_at(&hm, i), "key-4");
    count++;
  }
  ASSERT_EQ(count, 9);
}

// found by u/skeeto https://www.reddit.com/r/C_Programming/comments/1ht1xux/comment/m5asl8t
UTEST(HM_Bug, huge_number_should_fail_to_allocate){
  HM hm = {0};
  size_t capacity = ~((size_t)-1>>1);
  bool ok = HM_init(&hm, sizeof(char), capacity);
  ASSERT_FALSE(ok);
}


UTEST_MAIN();
