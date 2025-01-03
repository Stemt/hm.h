#ifndef HM_H_
#define HM_H_

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#ifndef HM_CALLOC
#define HM_CALLOC(n, s) calloc(n, s)
#endif

#ifndef HM_FREE
#define HM_FREE(p) free(p)
#endif

#ifndef HM_DEFAULT_CAPACITY
#define HM_DEFAULT_CAPACITY 512
#endif

#ifndef HM_ASSERT
#define HM_ASSERT(expr) assert(expr)
#endif

// by default HM will panic if an allocation (calloc) returns NULL
// by defining HM_DISABLE_ALLOC_PANIC HM_init() and HM_set() will 
// return false in case of allocation failure
#ifdef HM_DISABLE_ALLOC_PANIC
#define HM_CHECK_ALLOC(ptr, ...) if((ptr) == NULL) return (__VA_ARGS__ ,false)
#else
#define HM_CHECK_ALLOC(ptr, ...) HM_ASSERT(ptr != NULL)
#endif

typedef const size_t* HM_Iterator;

typedef struct{
  char** keys;
  void* values;
  size_t* next;
  size_t* prev;
  size_t first;
  size_t last;
  size_t element_size;
  size_t count;
  size_t capacity;
} HM;

bool HM_init(HM* self, size_t element_size, size_t capacity);
void HM_deinit(HM* self);
bool HM_grow(HM* self);
void* HM_get(HM* self, const char* key);
bool HM_set(HM* self, const char* key, void* value);
void HM_remove(HM* self, const char* key);
HM_Iterator HM_iterate(HM* self, HM_Iterator current);
const char* HM_key_at(HM* self, HM_Iterator it);
void* HM_value_at(HM* self, HM_Iterator it);

#define HM_GEN_WRAPPER_PROTOTYPE(type)\
  bool HM_##type##_init(HM* self, size_t capacity);\
  bool HM_##type##_set(HM* self, const char* key, type value);\
  type* HM_##type##_value_at(HM* self, HM_Iterator it);\
  type* HM_##type##_get(HM* self, const char* key);\

#define HM_GEN_WRAPPER_IMPLEMENTATION(type)\
  bool HM_##type##_init(HM* self, size_t capacity){ return HM_init(self, sizeof(type), capacity); }\
  bool HM_##type##_set(HM* self, const char* key, type value){ return HM_set(self, key, &value); }\
  type* HM_##type##_value_at(HM* self, HM_Iterator it){ return HM_value_at(self, it); }\
  type* HM_##type##_get(HM* self, const char* key){ return HM_get(self, key); }\

// 64-bit fnv-1a hash function (http://isthe.com/chongo/tech/comp/fnv/)
// !! use other version in case of non 64-bit architecture !!
#ifndef HM_HASH

size_t HM_default_hash(const char *str);
#define HM_HASH(str) HM_default_hash(str)

#ifdef HM_IMPLEMENTATION
#define HM_FNV_PRIME 0x00000100000001b3
#define HM_FNV_BASIS 0xcbf29ce484222325
size_t HM_default_hash(const char *str) {
    size_t hash = HM_FNV_BASIS;
    while (*str)
        hash = (hash ^ *str++) * HM_FNV_PRIME;
    return hash;
}

#endif //HM_IMPLEMENTATION
#endif // HM_HASH


#ifdef HM_IMPLEMENTATION

HM_Iterator HM_iterate(HM* self, HM_Iterator current){
  if(current == NULL){
    return &self->first;
  }else if(*current == self->last){
    return NULL;
  }else{
    return &self->next[*current];
  }
}

bool HM_set(HM* self, const char* key, void* value){
  if(self->count > self->capacity/2){
    if(!HM_grow(self)) return false;
  }
  size_t hash = HM_HASH(key) % self->capacity;
  size_t i = hash;
  while(self->keys[i] != NULL && strcmp(self->keys[i], key) != 0){
    i = (i+1) % self->capacity;
    HM_ASSERT(i != hash && "map is full!");
  }

  if(self->count == 0){
    self->first = i;
    self->last = i;
  }else{
    self->prev[i] = self->last;
    self->next[self->last] = i;
    self->last = i;
  }
  
  // TODO use internal buffer instead of seperate heap buffer for keys
  self->keys[i] = HM_CALLOC(strlen(key)+1, sizeof(char));
  HM_CHECK_ALLOC(self->keys[i]);
  strcpy(self->keys[i], key);
  memcpy(self->values + i * self->element_size, value, self->element_size);
  self->count++;
  return true;
}

const char* HM_key_at(HM* self, HM_Iterator it){
  return self->keys[*it];
}

void* HM_value_at(HM* self, HM_Iterator it){
  return self->values + (*it) * self->element_size;
}

void* HM_get(HM* self, const char* key){
  size_t hash = HM_HASH(key) % self->capacity;
  size_t i = hash;
  while(strcmp(self->keys[i], key) != 0){
    i = (i+1) % self->capacity;
    if(i == hash) return NULL;
  }
  return self->values + i * self->element_size;
}

void HM_remove(HM* self, const char* key){
  size_t hash = HM_HASH(key) % self->capacity;
  size_t i = hash;
  while(strcmp(self->keys[i], key) != 0){
    i = (i+1) % self->capacity;
    if(i == hash) return;
  }
  HM_FREE(self->keys[i]);
  self->keys[i] = NULL;
  
  size_t prev_index = self->prev[i];
  size_t next_index = self->next[i];
  
  if(i == self->first){
    self->first = next_index;
  }else if (i == self->last){
    self->last = prev_index;
  }else{
    self->prev[next_index] = prev_index;
    self->next[prev_index] = next_index;
  }
}

bool HM_grow(HM* self){
  HM new_hm = {0};
  new_hm.capacity = self->capacity > 0 ? self->capacity * 2 : HM_DEFAULT_CAPACITY;
  new_hm.element_size = self->element_size;
  
  new_hm.keys = HM_CALLOC(new_hm.capacity, sizeof(self->keys[0]));
  HM_CHECK_ALLOC(new_hm.keys);

  new_hm.values = HM_CALLOC(new_hm.capacity, self->element_size);
  HM_CHECK_ALLOC(new_hm.values, 
    HM_FREE(new_hm.keys)
  );

  new_hm.next = HM_CALLOC(new_hm.capacity, sizeof(self->next[0]));
  HM_CHECK_ALLOC(new_hm.next, 
    HM_FREE(new_hm.keys), 
    HM_FREE(new_hm.values)
  );

  new_hm.prev = HM_CALLOC(new_hm.capacity, sizeof(self->prev[0]));
  HM_CHECK_ALLOC(new_hm.prev, 
    HM_FREE(new_hm.keys), 
    HM_FREE(new_hm.values), 
    HM_FREE(new_hm.next)
  );

  // rehash and deinit if hm was previously initialized
  if(self->keys != NULL){
    for(HM_Iterator i = HM_iterate(self, NULL); i != NULL; i = HM_iterate(self, i)){
      HM_set(&new_hm, HM_key_at(self, i), HM_value_at(self, i));
    }

    HM_deinit(self);
  }

  memcpy(self, &new_hm, sizeof(*self));
  return true;
}

bool HM_init(HM* self, size_t element_size, size_t capacity){
  memset(self, 0, sizeof(*self));
  self->capacity = capacity;
  self->element_size = element_size;
  self->count = 0;
  return HM_grow(self);
}

void HM_deinit(HM* self){
  for(HM_Iterator i = HM_iterate(self, NULL); i != NULL; i = HM_iterate(self, i)){
    HM_FREE((void*)HM_key_at(self, i));
  }
  HM_FREE(self->keys);
  HM_FREE(self->values);
  HM_FREE(self->next);
  HM_FREE(self->prev);
}

#endif // HM_IMPLEMENTATION
#endif // HM_H_
