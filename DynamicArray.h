#ifndef _DYNAMIC_ARRAY_H
#define _DYNAMIC_ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DynamicArrayStruct(type)         \
    struct {                             \
        type *data;                      \
        int count;                       \
        int capacity;                    \
        void (*printFunc)(const type *); \
    }
#define DynamicArraySliceStruct(type)    \
    struct {                             \
        const type *data;                \
        const int count;                 \
        void (*printFunc)(const type *); \
    }

#define DynamicArrayDef(name, type)        \
    typedef DynamicArrayStruct(type) name; \
    typedef DynamicArraySliceStruct(type) name##Slice

#ifndef DYNAMIC_ARRAY_DEFAULT_SIZE
#    define DYNAMIC_ARRAY_DEFAULT_SIZE (256)
#endif // DYNAMIC_ARRAY_DEFAULT_SIZE

#ifndef DYNAMIC_ARRAY_ASSERT
#    include <assert.h>
#    define DYNAMIC_ARRAY_ASSERT assert
#endif // DYNAMIC_ARRAY_ASSERT

#ifdef DYNAMIC_ARRAY_IMPLEMENTATION

#    define DynamicArrayReserve(da, size)                                               \
        do {                                                                            \
            if ((da)->capacity < (size)) {                                              \
                if ((da)->capacity == 0) {                                              \
                    (da)->capacity = DYNAMIC_ARRAY_DEFAULT_SIZE;                        \
                }                                                                       \
                while ((da)->capacity < (size)) {                                       \
                    (da)->capacity *= 2;                                                \
                }                                                                       \
                (da)->data = realloc((da)->data, (da)->capacity * sizeof(*(da)->data)); \
                DYNAMIC_ARRAY_ASSERT((da)->data != NULL && "Buy more RAM!!!");          \
            }                                                                           \
        } while (0)

#    define DynamicArrayAppend(da, x)                   \
        do {                                            \
            DynamicArrayReserve((da), (da)->count + 1); \
            (da)->data[(da)->count++] = (x);            \
        } while (0)

#    define DynamicArrayRemoveAt(da, i)                                                                   \
        do {                                                                                              \
            if ((da)->data) {                                                                             \
                if ((i) >= 0 && (i) < (da)->count) {                                                      \
                    for (int __dara_shiftIdx = i; __dara_shiftIdx < (da)->count - 1; __dara_shiftIdx++) { \
                        (da)->data[__dara_shiftIdx] = (da)->data[__dara_shiftIdx + 1];                    \
                    }                                                                                     \
                    (da)->count--;                                                                        \
                }                                                                                         \
            }                                                                                             \
        } while (0)

#    define DynamicArrayIsEmpty(da) ((da)->count == 0)

#    define DynamicArrayPrint(da)                                                                                                                                   \
        do {                                                                                                                                                        \
            printf("[");                                                                                                                                            \
            if ((da)->printFunc) {                                                                                                                                  \
                for (const void *__dap_var = (da)->data; __dap_var < (void *) ((da)->data + (da)->count); __dap_var = (char *) (__dap_var) + sizeof(*(da)->data)) { \
                    (da)->printFunc(__dap_var);                                                                                                                     \
                    if (__dap_var < (void *) ((da)->data + (da)->count - 1)) {                                                                                      \
                        printf(", ");                                                                                                                               \
                    }                                                                                                                                               \
                }                                                                                                                                                   \
            } else {                                                                                                                                                \
                fprintf(stderr, "WARNING: line %d:DynamicArrayPrintN(%s): No print function provided", __LINE__, #da);                                              \
            }                                                                                                                                                       \
            printf("]");                                                                                                                                            \
        } while (0)

#    define DynamicArrayPrintN(da)   \
        do {                         \
            DynamicArrayPrint((da)); \
            printf("\n");            \
        } while (0)

#    define DynamicArrayClear(da) \
        do {                      \
            (da)->count = 0;      \
        } while (0)

#    define DynamicArrayDestroy(da)         \
        do {                                \
            free((da)->data);               \
            memset((da), 0, sizeof(*(da))); \
        } while (0)

#    define DynamicArrayForeach(type, var, da) for (type *var = (da)->data; var < (da)->data + (da)->count; var++)

// TODO: im not too sure if i want to crash in case of out of bounds,
//       it makes sense but im not sure

// inclusive indices
#    define DynamicArraySlice(da, start_idx, end_idx) \
        {                                             \
            .data = (da)->data + (start_idx),         \
            .count = (end_idx) - (start_idx) + 1,     \
            .printFunc = (da)->printFunc,             \
        };                                            \
        DYNAMIC_ARRAY_ASSERT("Slice out of bounds" && 0 <= (start_idx) && (start_idx) <= (end_idx) && (end_idx) < (da)->count)

#endif // DYNAMIC_ARRAY_IMPLEMENTATION

#endif // _DYNAMIC_ARRAY_H