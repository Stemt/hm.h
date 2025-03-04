# hm.h

[![Makefile CI](https://github.com/Stemt/hm.h/actions/workflows/makefile.yml/badge.svg?branch=main)](https://github.com/Stemt/hm.h/actions/workflows/makefile.yml)

This single header library seeks to provide a very simple, quick and easy to use hashmap implementation.
This library does not aim to be the fastest nor most memory efficient hashmap implementation. 
It rather aims to be as easy as possible to use while ramaining useful to more advanced users.

For a performance comparison to other c libraries see [hashmap_benchmarks](https://github.com/Stemt/hashmap_benchmarks).

## Usage

### Raw API Example

> [!NOTE]
> By default hm.h panics if memory allocation has failed and causes an abort if that is the case.
> See [Disable Panic on Allocation Failure](#Disable-Panic-on-Allocation-Failure) if you want to change that behavior.

```c
#define HM_IMPLEMENTATION
#include "hm.h"

int main(void){
    HM hm = {0};
    
    HM_init(&hm, sizeof(int), 10); 
    
    // inserting a key value pair
    int value = 2;
    HM_set(&hm, "key", &value);

    // retrieving value by key
    int* res = (int*)HM_get(&hm, "test");
    if(res != NULL){
      printf("res: %d\n", *res);
    }else{
      printf("res: not found\n");
    }
    
    // removing a key value pair
    HM_remove(&hm, "test");
    assert(HM_get(&hm, "test") == NULL);

    HM_deinit(&hm);
    return 0;
}
```

### Type Wrapped API

To provide better type information, hm.h comes with 2 macros to generate api functions for specific data types.
`HM_GEN_WRAPPER_PROTOYPE(type)` generates function prototypes/declarations which should ideally be placed inside a header file (.h).
`HM_GEN_WRAPPER_IMPLEMENTATION(type)` generates function implementations (which just call their equivalent in the raw api) and should ideally be placed inside a source file (.c).

> [!NOTE]
> Due to the function names that the wrapper generates, it's not possible to generate a wrapper for a pointer using the normal syntax (e.g. :x: `HM_GEN_WRAPPER_PROTOTYPE(int*)` :x:).
> Instead, you'll have to typedef your pointer type:
> ```c
> typedef int* int_ptr_t;
> HM_GEN_WRAPPER_PROTOTYPE(int_ptr_t);
> HM_GEN_WRAPPER_IMPLEMENTATION(int_ptr_t);
> ```

For example for an `int` the macros will generate the following:
```c
// HM_GEN_WRAPPER_PROTOTYPE(int);
bool HM_int_init(HM* self, size_t capacity);\
bool HM_int_set(HM* self, const char* key, int value);\
int* HM_int_value_at(HM* self, HM_Iterator it);\
int* HM_int_get(HM* self, const char* key);\

// HM_GEN_WRAPPER_IMPLEMENTATION(int);
bool HM_int_init(HM* self, size_t capacity){ 
    return HM_init(self, sizeof(int), capacity); 
}

bool HM_int_set(HM* self, const char* key, int value){
    return HM_set(self, key, &value); 
}

int* HM_int_value_at(HM* self, HM_Iterator it){ 
    return HM_value_at(self, it); 
}

int* HM_int_get(HM* self, const char* key){ 
    return HM_get(self, key); 
}
```

Full example:

```c
#define HM_IMPLEMENTATION
#include "hm.h"

HM_GEN_WRAPPER_PROTOTYPE(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

int main(void){
    HM hm = {0};

    HM_int_init(&hm, 10); 
    
    // inserting a key value pair
    HM_int_set(&hm, "key", 2);

    // retrieving value by key
    int* res = HM_int_get(&hm, "test");
    if(res != NULL){
      printf("res: %d\n", *res);
    }else{
      printf("res: not found\n");
    }

    // removing a key value pair
    HM_remove(&hm, "test");
    assert(HM_get(&hm, "test") == NULL);

    HM_deinit(&hm);
    return 0;
}

```

### Using Keys with Custom Length (kwl)

If you want to use a custom type as a key or simply a non null terminated string there are function variants which allow you to specify a length for the given key value.
These variants are indicated with `kwl`, short for "key with length".

```c
bool HM_kwl_set(HM* self, const void* key, size_t key_len, void* value);
void* HM_kwl_get(HM* self, const void* key, size_t key_len);
void HM_kwl_remove(HM* self, const void* key, size_t key_len);
```

The type wrapped API also provides the same variants, e.g. for `int`:
```c
bool HM_int_kwl_set(HM* self, const void* key, size_t key_len, int value);
int* HM_int_kwl_get(HM* self, const void* key, size_t key_len);
void HM_int_kwl_remove(HM* self, const void* key, size_t key_len);
```

Specifically for non null terminated strings there is also a function for requesting the key length.
```c
const size_t* HM_key_len_at(HM* self, HM_Iterator it);
```

### Iterating over keys and values

> [!NOTE]
> Results from the `_at` family of function (e.g. `HM_key_at()` and `HM_value_at()`) only return valid results with an `HM_Iterator` retrieved using `HM_iterate()` performed on the same hashmap. Otherwise the return value is undefined.

```c
#define HM_IMPLEMENTATION
#include "hm.h"

HM_GEN_WRAPPER_PROTOTYPE(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

int main(void){
    HM_int_init(&hm, 0);
    
    for(int i = 0; i < 10; ++i){
        char key[1024] = {0};
        snprintf(key, sizeof(key)-1, "key-%d", i);
        HM_int_set(&hm, key, i);
    }

    for(HM_Iterator i = HM_iterate(&hm, NULL); i != NULL; i = HM_iterate(&hm, i)){
        printf("key: '%s', value '%d'\n", HM_key_at(&hm, i), *HM_int_value_at(&hm, i));
    }

    HM_deinit(&hm);
    return 0;
}
```

### Disable Panic on Allocation Failure

For convenience hm.h will crash your program so that you don't have to check the results of the `HM_init()` and `HM_set()` functions. 
If however, if you do want to handle these failures you can use the following define:

```c
#define HM_DISABLE_ALLOC_PANIC
```

And most importantly remember to actually handle allocation failures:

```c
#define HM_DISABLE_ALLOC_PANIC
#define HM_IMPLEMENTATION
#include "hm.h"

HM_GEN_WRAPPER_PROTOTYPE(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

int main(void){
    HM hm = {0};
    
    if(!HM_int_init(&hm, 10)){
        // handle alloc failure (hm is not initialized)
    }
    
    if(!HM_int_set(&hm, "key", 2)){
        // handle alloc failure (new value was not added to hm)
    }

    int* res = HM_int_get(&hm, "test");
    if(res != NULL){
      printf("res: %d\n", *res);
    }else{
      printf("res: not found\n");
    }

    HM_deinit(&hm);
    return 0;
}
```

## Tests

Tests make use of [utest.h by sheredom](https://github.com/sheredom/utest.h).
To run them simply run:

```
make test
```
