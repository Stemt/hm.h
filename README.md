# hm.h

This single header library seeks to provide a very simple, quick and easy to use hashmap implementation.

## Usage

### Type Wrapped API Quick Example

> [!NOTE]
> By default hm.h uses HM_ASSERT to check if memory allocation has failed, causing an abort if that is the case.
> See [](#Disable-Panic-on-Allocation-Failure)

> [!NOTE]
> Due to the function names that the wrapper generates, it's not possible to generate a wrapper for a pointer using the normal syntax (e.g. :x: `HM_GEN_WRAPPER_DEF(int*)` :x:).
> Instead, typedef your pointer type and use that instead (e.g. :white_check_mark: `typedef int* int_ptr_t;` and `HM_GEN_WRAPPER_DEF(int_ptr_t)` :white_check_mark:) 

```c
#define HM_IMPLEMENTATION
#include "hm.h"

HM_GEN_WRAPPER_DEF(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

int main(void){
    HM hm = {0};

    HM_int_init(&hm, 10); 
    
    HM_set(&hm, "key", 2);

    printf("%d\n", *HM_int_get(&hm, "key"));

    HM_deinit(&hm);
}

```

### Raw API Quick Example

```c
#define HM_IMPLEMENTATION
#include "hm.h"

int main(void){
    HM hm = {0};

    HM_init(&hm, sizeof(int), 10); 
    
    int value = 2;
    HM_set(&hm, "key", &value);

    printf("%d\n", *(int*)HM_get(&hm, "key"));

    HM_deinit(&hm);
}
```

### Iterating over keys and values

```c
#define HM_IMPLEMENTATION
#include "hm.h"

HM_GEN_WRAPPER_DEF(int);
HM_GEN_WRAPPER_IMPLEMENTATION(int);

int main(void){
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

int main(void){
    HM hm = {0};
    
    if(!HM_int_init(&hm, 10)){
        // handle alloc failure (hm is not initialized)
    }
    
    
    if(!HM_set(&hm, "key", 2)){
        // handle alloc failure (new value was not added to hm)
    }

    printf("%d\n", *HM_int_get(&hm, "key"));

    HM_deinit(&hm);

    return 0;
}
```

