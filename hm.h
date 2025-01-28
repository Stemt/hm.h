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
#include <assert.h>
#define HM_ASSERT(expr) assert(expr)
#endif

#ifndef HM_LOG_ERROR
#include <stdio.h>
#define HM_LOG_ERROR(...) fprintf(stderr, __VA_ARGS__)
#endif

// by default HM will panic if an allocation (HM_CALLOC) returns NULL.
// by defining HM_DISABLE_ALLOC_PANIC, HM_init() and HM_set() will 
// return false in case of allocation failure
#ifdef HM_DISABLE_ALLOC_PANIC
#define HM_CHECK_ALLOC(ptr, ...) if((ptr) == NULL){ __VA_ARGS__; return false; }
#else
#define HM_CHECK_ALLOC(ptr, ...) if(ptr == NULL){\
  HM_LOG_ERROR("hm.h: allocation failure, define 'HM_DISABLE_ALLOC_PANIC' if crashing is undesired in this case.\n");\
  abort();\
}
#endif

typedef size_t (*HM_HashFunc)(const char* key, size_t key_len);
typedef const size_t* HM_Iterator;

typedef struct{
  char* key;
  size_t key_len;
  size_t next;
  size_t prev;
} HM_Entry;

typedef struct{
  void* values;
  HM_Entry* entries;
  size_t first;
  size_t last;
  size_t element_size;
  size_t count;
  size_t capacity;

  HM_HashFunc hash_func;
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
 * \brief           returns pointer to element associated with key if available
 * \param self:     hashmap handle 
 * \param key:      key to lookup in hashmap 
 * \param key_len:  length of the given key
 * \return          NULL if key was not found in hashmap otherwise pointer to element in hashmap
 */
void* HM_kwl_get(HM* self, const void* key, size_t key_len);

/**
 * \brief   'sized key' convenience macro for HM_kwl_get, equivalent to 
 *          'HM_kwl_get(self, &(key), sizeof(key))'
 * \note    make sure to dereference if you have a pointer to your key!
 */
#define HM_sk_get(self, key)\
HM_kwl_get(self, &(key), sizeof(key))

/**
 * \brief         inserts a new key value pair into the hashmap
 * \note          calls HM_grow() if element count > capacity, thus will crash on allocation 
 *                failure if HM_DISABLE_ALLOC_PANIC is not defined
 * \param self:   hashmap handle 
 * \param key:    key to lookup in hashmap 
 * \param value:  pointer to value to be inserted 
 * \returns       true if insertion was succesful, false if an allocation failed **and** 
 *                HM_DISABLE_ALLOC_PANIC is defined
 */
bool HM_set(HM* self, const char* key, void* value);

/**
 * \brief           inserts a new key value pair into the hashmap using key with provided lenght
 * \note            calls HM_grow() if element count > capacity, thus will crash on allocation 
 *                  failure if HM_DISABLE_ALLOC_PANIC is not defined
 * \param self:     hashmap handle 
 * \param key:      key to lookup in hashmap 
 * \param key_len:  length of key in bytes
 * \param value:    pointer to value to be inserted 
 * \returns         true if insertion was succesful, false if an allocation failed **and** 
 *                  HM_DISABLE_ALLOC_PANIC is defined
 */
bool HM_kwl_set(HM* self, const void* key, size_t key_len, void* value);

/**
 * \brief   'sized key' convenience macro for HM_kwl_set, equivalent to 
 *          'HM_kwl_set(self, &(key), sizeof(key), value)'
 * \note    make sure to dereference if you have a pointer to your key!
 */
#define HM_sk_set(self, key, value)\
  HM_kwl_set(self, &(key), sizeof(key), value)

/**
 * \brief         removes a key value pair from the hashmap
 * \param self:   hashmap handle 
 * \param key:    key to remove from hashmap
 */
void HM_remove(HM* self, const char* key);

/**
 * \brief         removes a key value pair from the hashmap
 * \param self:   hashmap handle 
 * \param key:    key to remove from hashmap
 */
void HM_kwl_remove(HM* self, const void* key, size_t key_len);

/**
 * \brief   'sized key' convenience macro for HM_kwl_remove, equivalent to 
 *          'HM_kwl_remove(self, &(key), sizeof(key))'
 * \note    make sure to dereference if you have a pointer to your key!
 */
#define HM_sk_remove(self, key)\
  HM_kwl_remove(self, &(key), sizeof(key))

/**
 * \brief         returns HM_Iterator for given key if available
 * \param self:   hashmap handle
 * \param key:    key to of element to look up
 *
 * \return        HM_Iterator for the found element, if not found: NULL
 */
HM_Iterator HM_find(HM* self, const char* key);

/**
 * \brief           returns HM_Iterator for given key if available
 * \param self:     hashmap handle
 * \param key:      key to of element to look up
 * \param key_len:  length of the given key
 *
 * \return          HM_Iterator for the found element, if not found: NULL
 */
HM_Iterator HM_kwl_find(HM* self, const void* key, size_t key_len);

/**
 * \brief   'sized key' convenience macro for HM_kwl_find, equivalent to 
 *          'HM_kwl_find(self, &(key), sizeof(key))'
 * \note    make sure to dereference if you have a pointer to your key!
 */
#define HM_sk_find(self, key)\
  HM_kwl_find(self, &(key), sizeof(key))

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
 * \brief         returns HM_Iterator to the first element
 * \param self:   hashmap handle
 *
 * \return        HM_Iterator to the first element of the given hashmap
 */
#define HM_begin(self) ((HM_Iterator)&((self)->first))

/**
 * \brief         returns HM_Iterator to the last element
 * \param self:   hashmap handle
 *
 * \return        HM_Iterator to the last element of the given hashmap
 */
#define HM_end(self) ((HM_Iterator)&((self)->end))

/**
 * \brief         returns key for corresponding HM_Iterator
 * \param self:   hashmap handle 
 * \param it:     HM_Iterator by which to get the key
 * \return        pointer to the key string
 */
const char* HM_key_at(HM* self, HM_Iterator it);

/**
 * \brief         returns key length for corresponding HM_Iterator
 * \param self:   hashmap handle 
 * \param it:     HM_Iterator by which to get the key length
 * \return        NULL if invalid iterator, otherwise pointer
 */
const size_t* HM_key_len_at(HM* self, HM_Iterator it);

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
  bool HM_##type##_kwl_set(HM* self, const void* key, size_t key_len, type value);\
  type* HM_##type##_kwl_get(HM* self, const void* key, size_t key_len);\

#define HM_GEN_WRAPPER_IMPLEMENTATION(type)\
  bool HM_##type##_init(HM* self, size_t capacity)\
    { return HM_init(self, sizeof(type), capacity); }\
  bool HM_##type##_set(HM* self, const char* key, type value)\
    { return HM_set(self, key, &value); }\
  type* HM_##type##_value_at(HM* self, HM_Iterator it)\
    { return HM_value_at(self, it); }\
  type* HM_##type##_get(HM* self, const char* key)\
    { return HM_get(self, key); }\
  bool HM_##type##_kwl_set(HM* self, const void* key, size_t key_len, type value)\
    { return HM_kwl_set(self, key, key_len, &value); }\
  type* HM_##type##_kwl_get(HM* self, const void* key, size_t key_len)\
    { return HM_kwl_get(self, key, key_len); }\


#ifndef HM_HASH
#if INTPTR_MAX != INT64_MAX
#error "HM: default hash algo only supports 64-bit, please define custom HM_HASH(str, len)"
#endif

size_t HM_default_hash(const char *str, size_t len);
#define HM_HASH HM_default_hash

#ifdef HM_IMPLEMENTATION

// 64-bit fnv-1a hash function (http://isthe.com/chongo/tech/comp/fnv/)
// !! use other version in case of non 64-bit architecture !!
#define HM_FNV_PRIME 0x00000100000001b3
#define HM_FNV_BASIS 0xcbf29ce484222325
size_t HM_default_hash(const char *str, size_t len){
    size_t hash = HM_FNV_BASIS;
    const char* end = str + len;
    while (str < end){
      hash = (hash ^ (unsigned char)(*str++)) * HM_FNV_PRIME;
    }
    return hash;
}
#endif // HM_IMPLEMENTATION
#endif // HM_HASH


#ifdef HM_IMPLEMENTATION

HM_Iterator HM_iterate(HM* self, HM_Iterator current){
  if(self->count == 0) return NULL;
  if(current == NULL){
    return &self->first;
  }else if(*current == self->last){
    return NULL;
  }else{
    return &self->entries[*current].next;
  }
}

bool HM_kwl_set(HM* self, const void* key, size_t key_len, void* value){
  if(self->count > self->capacity/2){
    if(!HM_grow(self)) return false;
  }

  size_t hash = self->hash_func(key, key_len) % self->capacity;
  size_t i = hash;
  while(self->entries[i].key != NULL && (self->entries[i].key_len != key_len || memcmp(self->entries[i].key, key, key_len) != 0)){
    i = (i+1) % self->capacity;
    HM_ASSERT(i != hash && "map is full!");
  }

  // only update entries when new key is inserted
  if(self->entries[i].key == NULL){
    if(self->count == 0){
      self->first = i;
      self->last = i;
    }else{
      self->entries[i].prev = self->last;
      self->entries[self->last].next = i;
      self->last = i;
    }

    // TODO use internal buffer instead of seperate heap buffer for keys
    self->entries[i].key = (char*)HM_CALLOC(key_len, sizeof(char));
    self->entries[i].key_len = key_len;
    HM_CHECK_ALLOC(self->entries[i].key);
    memcpy(self->entries[i].key, key, key_len);
  }else{
    HM_ASSERT(self->entries[i].key_len == key_len);
    HM_ASSERT(memcmp(self->entries[i].key, key, key_len) == 0);
  }
  
  memcpy(self->values + i * self->element_size, value, self->element_size);

  self->count++;
  return true;
}

bool HM_set(HM* self, const char* key, void* value){
  return HM_kwl_set(self, key, strlen(key), value);
}

const char* HM_key_at(HM* self, HM_Iterator it){
  if(it == NULL) return NULL;
  return self->entries[*it].key;
}

const size_t* HM_key_len_at(HM* self, HM_Iterator it){
  if(it == NULL) return NULL;
  return &self->entries[*it].key_len;
}

void* HM_value_at(HM* self, HM_Iterator it){
  if(it == NULL) return NULL;
  return self->values + (*it) * self->element_size;
}


HM_Iterator HM_kwl_find(HM* self, const void* key, size_t key_len){
  size_t hash = self->hash_func(key, key_len) % self->capacity;
  size_t i = hash;
  while(self->entries[i].key == NULL || memcmp(self->entries[i].key, key, key_len) != 0){
    i = (i+1) % self->capacity;
    if(i == hash){
      return NULL;
    }
  }
  if(i == self->first) return &self->first;
  return &self->entries[self->entries[i].prev].next;
  return NULL;
}

HM_Iterator HM_find(HM* self, const char* key){
  return HM_kwl_find(self, key, strlen(key));
}

void* HM_get(HM* self, const char* key){
  if(self->count == 0) return NULL;
  return HM_value_at(self, HM_kwl_find(self, key, strlen(key)));
}

void* HM_kwl_get(HM* self, const void* key, size_t key_len){
  if(self->count == 0) return NULL;
  return HM_value_at(self, HM_kwl_find(self, key, key_len));
}

void HM_kwl_remove(HM* self, const void* key, size_t key_len){
  if(self->count == 0) return;

  HM_Iterator it = HM_kwl_find(self, key, key_len);
  size_t i = *it;

  HM_FREE(self->entries[i].key);
  self->entries[i].key = NULL;
  self->entries[i].key_len = 0;
  
  size_t prev_index = self->entries[i].prev;
  size_t next_index = self->entries[i].next;
  
  if(i == self->first){
    self->first = next_index;
  }else if (i == self->last){
    self->last = prev_index;
  }else{
    self->entries[next_index].prev = prev_index;
    self->entries[prev_index].next = next_index;
  }

  self->count--;
}

void HM_remove(HM* self, const char* key){
  HM_kwl_remove(self, key, strlen(key));
}

bool HM_allocate(HM* self, size_t element_size, size_t capacity){
  self->capacity = capacity;
  self->element_size = element_size;
  
  self->entries = (HM_Entry*)HM_CALLOC(capacity, sizeof(HM_Entry));
  HM_CHECK_ALLOC(self->entries);

  self->values = HM_CALLOC(capacity, element_size);
  HM_CHECK_ALLOC(self->values, 
    HM_FREE(self->entries);
  );

  return true;
}

bool HM_grow(HM* self){
  HM new_hm = {0};
  
  if(!HM_allocate(&new_hm, self->element_size, self->capacity * 2)){
    return false;
  }

  // rehash and deinit
  for(HM_Iterator i = HM_iterate(self, NULL); i != NULL; i = HM_iterate(self, i)){
    HM_kwl_set(&new_hm, HM_key_at(self, i), *HM_key_len_at(self, i), HM_value_at(self, i));
  }
  HM_deinit(self);

  memcpy(self, &new_hm, sizeof(*self));
  return true;
}

void HM_override_hash_func(HM* self, HM_HashFunc func){
  self->hash_func = func;
}

bool HM_init(HM* self, size_t element_size, size_t capacity){
  memset(self, 0, sizeof(*self));
  self->hash_func = HM_HASH;
  return HM_allocate(self, element_size, capacity > 0 ? capacity : HM_DEFAULT_CAPACITY);
}

void HM_deinit(HM* self){
  for(HM_Iterator i = HM_iterate(self, NULL); i != NULL; i = HM_iterate(self, i)){
    HM_FREE((void*)HM_key_at(self, i));
  }
  HM_FREE(self->entries);
  HM_FREE(self->values);
}

HM* HM_new(size_t element_size, size_t capacity){
  HM* self = (HM*)HM_CALLOC(1, sizeof(HM));
  HM_CHECK_ALLOC(self);
  if(!HM_init(self, element_size, capacity)){
    HM_deinit(self);
    return NULL;
  }
  return self;
}

void HM_delete(HM* self){
  HM_deinit(self);
  HM_FREE(self);
}

#endif // HM_IMPLEMENTATION
#endif // HM_H_
