#include <stdint.h>
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

UTEST(HM_Basic, reinsertion){
  HM hm = {0};
  ASSERT_TRUE(HM_int_init(&hm, 2));
  ASSERT_EQ(hm.capacity, 2ULL);

  ASSERT_TRUE(HM_int_set(&hm, "key-1", 1));
  ASSERT_TRUE(HM_int_set(&hm, "key-2", 2));
  ASSERT_TRUE(HM_int_set(&hm, "key-3", 3));
  ASSERT_EQ(hm.count, 3ULL);
  
  ASSERT_TRUE(HM_int_set(&hm, "key-1", 1));
  ASSERT_TRUE(HM_int_set(&hm, "key-2", 2));
  ASSERT_TRUE(HM_int_set(&hm, "key-3", 3));
  ASSERT_EQ(hm.count, 3ULL);
}

UTEST(HM_Basic_key_with_length, insertion){
  HM hm = {0};
  ASSERT_TRUE(HM_init(&hm, sizeof(int), 0));
  
  int value = 2;
  ASSERT_TRUE(HM_kwl_set(&hm, &value, sizeof(value), &value));
  
  ASSERT_NE(HM_kwl_get(&hm, &value, sizeof(value)), NULL);
  ASSERT_EQ(*(int*)HM_kwl_get(&hm, &value, sizeof(value)), 2);
}

UTEST(HM_Basic_key_with_length, removal){
  HM hm = {0};
  ASSERT_TRUE(HM_init(&hm, sizeof(int), 0));
  
  int value = 2;
  ASSERT_TRUE(HM_kwl_set(&hm, &value, sizeof(value), &value));
  
  ASSERT_NE(HM_kwl_get(&hm, &value, sizeof(value)), NULL);
  ASSERT_EQ(*(int*)HM_kwl_get(&hm, &value, sizeof(value)), 2);
  
  HM_kwl_remove(&hm, &value, sizeof(int));
  ASSERT_EQ(HM_kwl_get(&hm, &value, sizeof(value)), NULL);
}

UTEST(HM_Basic_key_with_length, resize){
  HM hm = {0};
  ASSERT_TRUE(HM_int_init(&hm, 2));
  ASSERT_EQ(hm.capacity, 2ULL);

  ASSERT_TRUE(HM_int_kwl_set(&hm, &(int){1}, sizeof(int), 1));
  ASSERT_TRUE(HM_int_kwl_set(&hm, &(int){2}, sizeof(int), 2));
  ASSERT_TRUE(HM_int_kwl_set(&hm, &(int){3}, sizeof(int), 3));

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
    ASSERT_NE(i, NULL);
    count++;
  }
  ASSERT_EQ(count, 10);
}

UTEST(HM_Iteration, iterate_key_length){
  HM hm = {0};
  HM_int_init(&hm, 0);

  for(int i = 0; i < 10; ++i){
    HM_int_kwl_set(&hm, &i, sizeof(int), i);
  }

  int count = 0;
  for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
    ASSERT_NE(i, NULL);
    ASSERT_NE(HM_key_len_at(&hm, i), NULL);
    ASSERT_EQ(*HM_key_len_at(&hm, i), sizeof(int));
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
