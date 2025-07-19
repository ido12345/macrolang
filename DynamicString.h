#ifndef _DYNAMIC_STRING_H
#define _DYNAMIC_STRING_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *data;
    size_t count; // count == strlen(data)
    size_t capacity;
} DynamicString;

void DynamicStringRemoveAll(DynamicString *ds, char x);
bool DynamicStringReadFile(DynamicString *ds, const char *filePath);
bool DynamicStringWriteFile(const char *filePath, DynamicString *ds);

#ifndef DYNAMIC_STRING_DEFAULT_SIZE
#    define DYNAMIC_STRING_DEFAULT_SIZE (256)
#endif // DYNAMIC_STRING_DEFAULT_SIZE

#define DS_Fmt "%.*s"
#define DS_Arg(ds) (ds)->count, (ds)->data

#ifdef DYNAMIC_STRING_IMPLEMENTATION

#    define DynamicStringReserve(ds, size)                        \
        do {                                                      \
            size_t __dsr_reqSize = (size);                        \
            if ((ds)->capacity < __dsr_reqSize) {                 \
                if ((ds)->capacity == 0) {                        \
                    (ds)->capacity = DYNAMIC_STRING_DEFAULT_SIZE; \
                }                                                 \
                while ((ds)->capacity < __dsr_reqSize) {          \
                    (ds)->capacity *= 2;                          \
                }                                                 \
                (ds)->data = realloc((ds)->data, (ds)->capacity); \
                assert(((ds)->data != NULL) && "REALLOC FAIL");   \
            }                                                     \
        } while (0)

#    define DynamicStringAppendf(ds, fstr, ...)                                                    \
        do {                                                                                       \
            size_t __dsa_amount = snprintf(NULL, 0, fstr, ##__VA_ARGS__);                          \
            DynamicStringReserve((ds), (ds)->count + __dsa_amount + 1);                            \
            snprintf((ds)->data + (ds)->count, (ds)->capacity - (ds)->count, fstr, ##__VA_ARGS__); \
            (ds)->count += __dsa_amount;                                                           \
            (ds)->data[(ds)->count] = '\0';                                                        \
        } while (0)

#    define DynamicStringAppendStr(ds, str, len)                 \
        do {                                                     \
            DynamicStringReserve((ds), (ds)->count + (len) + 1); \
            memcpy((ds)->data + (ds)->count, (str), (len));      \
            (ds)->count += (len);                                \
            (ds)->data[(ds)->count] = '\0';                      \
        } while (0)

#    define DynamicStringAppendChar(ds, char)            \
        do {                                             \
            DynamicStringReserve((ds), (ds)->count + 2); \
            (ds)->data[(ds)->count] = (char);            \
            (ds)->count++;                               \
            (ds)->data[(ds)->count] = '\0';              \
        } while (0)

#    define DynamicStringClear(ds)    \
        do {                          \
            if ((ds)->data) {         \
                (ds)->count = 0;      \
                (ds)->data[0] = '\0'; \
            }                         \
        } while (0)

#    define DynamicStringPrint(ds)                             \
        do {                                                   \
            if ((ds)->data) {                                  \
                printf("%.*s", (int) (ds)->count, (ds)->data); \
            }                                                  \
        } while (0)

// print + newline
#    define DynamicStringPrintN(ds)                              \
        do {                                                     \
            if ((ds)->data) {                                    \
                printf("%.*s\n", (int) (ds)->count, (ds)->data); \
            }                                                    \
        } while (0)

#    define DynamicStringDestroy(ds)            \
        do {                                    \
            if ((ds)->data) {                   \
                free((ds)->data);               \
                memset((ds), 0, sizeof(*(ds))); \
            }                                   \
        } while (0)

bool DynamicStringReadFile(DynamicString *ds, const char *filePath) {
    bool ret = true;
    FILE *inputFile = NULL;

    inputFile = fopen(filePath, "rb");
    if (!inputFile) {
        fprintf(stderr, "Could not fopen \"%s\": %s", filePath, strerror(errno));
        ret = false;
        goto finish;
    }
    if (fseek(inputFile, 0, SEEK_END) != 0) {
        fprintf(stderr, "Could not fseek \"%s\" to end: %s", filePath, strerror(errno));
        ret = false;
        goto finish;
    }
    long fileLen = ftell(inputFile);
    if (fileLen < 0) {
        fprintf(stderr, "Could not ftell \"%s\": %s", filePath, strerror(errno));
        ret = false;
        goto finish;
    }
    if (fseek(inputFile, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Could not fseek \"%s\" to start: %s", filePath, strerror(errno));
        ret = false;
        goto finish;
    }
    DynamicStringReserve(ds, fileLen);
    if (fread(ds->data, 1, fileLen, inputFile) != (size_t) fileLen) {
        fprintf(stderr, "Could not fread \"%s\" to dynamic string: %s", filePath, strerror(errno));
        ret = false;
        goto finish;
    }
    ds->count += fileLen;

finish:
    if (inputFile) {
        fclose(inputFile);
    }
    return ret;
}

bool DynamicStringWriteFile(const char *filePath, DynamicString *ds) {
    bool ret = true;
    FILE *outputFile = NULL;

    outputFile = fopen(filePath, "wb");
    if (!outputFile) {
        fprintf(stderr, "Could not fopen \"%s\": %s", filePath, strerror(errno));
        ret = false;
        goto finish;
    }
    if (fwrite(ds->data, 1, ds->count, outputFile) != ds->count) {
        fprintf(stderr, "Could not fwrite dynamic string to \"%s\": %s", filePath, strerror(errno));
        ret = false;
        goto finish;
    }

finish:
    if (outputFile) {
        fclose(outputFile);
    }
    return ret;
}

void DynamicStringRemoveAll(DynamicString *ds, char x) {
    char *dest = ds->data;
    char *src = ds->data;
    size_t count = ds->count;
    while ((size_t)(src - ds->data) < count) {
        if (*src == x) {
            src++;
            ds->count--;
        } else {
            *dest++ = *src++;
        }
    }
    if (dest < src) {
        *dest = '\0';
    }
}

#endif // DYNAMIC_STRING_IMPLEMENTATION

#endif // _DYNAMIC_STRING_H