/*
 * MIT License
 * 
 * Copyright (c) 2025 Alaric de Ruiter
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

/**
 * \brief                 initializes the hashmap
 * \note                  crashes if allocation failed and HM_DISABLE_ALLOC_PANIC is not defined
 * \param self:           hashmap handle
 * \param element_size:   size of the element type the hashmap will store
 * \param capacity:       initial capacity of the hashmap, note that the hashmap will 
 *                        always keep half it's capacity empty to limit collisions
 * \returns               true if initialization was succesful, false if allocation failed **and** 
 *                        HM_DISABLE_ALLOC_PANIC is defined
 */
bool HM_init(HM* self, size_t element_size, size_t capacity);

/**
 * \brief         frees internal buffers
 * \param self:   hashmap handle
 */
void HM_deinit(HM* self);

/**
 * \brief         doubles the hashmaps capacity or allocates its buffers if not previously 
 *                allocated
 * \note          crashes if allocation failed and HM_DISABLE_ALLOC_PANIC is not defined
 * \param self:   hashmap handle
 * \returns       true if allocation was succesful, false if allocation failed **and** 
 *                HM_DISABLE_ALLOC_PANIC is defined
 */
bool HM_grow(HM* self);

/**
 * \brief         returns pointer to element associated with key if available
 * \param self:   hashmap handle 
 * \param key:    key to lookup in hashmap 
 * \return        NULL if key was not found in hashmap otherwise pointer to element in hashmap
 */
void* HM_get(HM* self, const char* key);

/**
 * \brief         inserts a new key value pair into the hashmap
 * \note          calls HM_grow() if element count > capacity, thus will crash on allocation 
 *                failure if HM_DISABLE_ALLOC_PANIC is not defined
 * \param self:   hashmap handle 
 * \param key:    key to lookup in hashmap 
 * \returns       true if insertion was succesful, false if an allocation failed **and** 
 *                HM_DISABLE_ALLOC_PANIC is defined
 */
bool HM_set(HM* self, const char* key, void* value);

/**
 * \brief         removes a key value pair from the hashmap
 * \param self:   hashmap handle 
 * \param key:    key to remove from hashmap
 */
void HM_remove(HM* self, const char* key);

/**
 * \brief           returns HM_Iterator based on give HM_Iterator passed as argument
 * \param self:     hashmap handle 
 * \param current:  NULL to get a HM_Iterator for the first element or previously returned 
 *                  HM_Iterator for the next element
 * \return          HM_Iterator for the nest element or NULL if the given HM_Iterator is for 
 *                  the last element
 */
HM_Iterator HM_iterate(HM* self, HM_Iterator current);

/**
 * \brief         returns key for corresponding HM_Iterator
 * \param self:   hashmap handle 
 * \param it:     HM_Iterator by which to get the key
 * \return        pointer to the key string
 */
const char* HM_key_at(HM* self, HM_Iterator it);

/**
 * \brief         returns pointer to value for corresponding HM_Iterator
 * \param self:   hashmap handle 
 * \param it:     HM_Iterator by which to get the value
 * \return        pointer to the value inside the hashmap
 */
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
    while (*str){
      hash = (hash ^ *str++) * HM_FNV_PRIME;
    }
    return hash;
}

#endif //HM_IMPLEMENTATION
#endif // HM_HASH


#ifdef HM_IMPLEMENTATION

HM_Iterator HM_iterate(HM* self, HM_Iterator current){
  if(self->count == 0) return NULL;
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
  if(self->count == 0) return NULL;

  size_t hash = HM_HASH(key) % self->capacity;
  size_t i = hash;
  while(self->keys[i] == NULL || strcmp(self->keys[i], key) != 0){
    i = (i+1) % self->capacity;
    if(i == hash) return NULL;
  }

  return self->values + i * self->element_size;
}

void HM_remove(HM* self, const char* key){
  if(self->count == 0) return;

  size_t hash = HM_HASH(key) % self->capacity;
  size_t i = hash;
  while(self->keys[i] == NULL || strcmp(self->keys[i], key) != 0){
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

  self->count--;
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
