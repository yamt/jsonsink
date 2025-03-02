#include <stdbool.h>
#include <stddef.h>

#if !defined(JSONSINK_MAX_NEST)
#define JSONSINK_MAX_NEST 8
#endif

#define JSONSINK_OK 0
#define JSONSINK_ERROR_BUFFEROVERFLOW 1
#define JSONSINK_ERROR_SERIALIZATION 2

struct jsonsink {
        void *buf;
        size_t buflen;
        size_t bufpos;
        unsigned int level; /* object/array nest level */
        int error;
        bool need_comma[JSONSINK_MAX_NEST];
};

void jsonsink_init(struct jsonsink *s);
void jsonsink_set_buffer(struct jsonsink *s, void *buf, size_t buflen);
void jsonsink_clear(struct jsonsink *s);
int jsonsink_error(struct jsonsink *s);
void jsonsink_set_error(struct jsonsink *s, int error);
void *jsonsink_pointer(struct jsonsink *s);
size_t jsonsink_size(struct jsonsink *s);

void jsonsink_object_start(struct jsonsink *s);
void jsonsink_object_end(struct jsonsink *s);

void jsonsink_add_serialized_key(struct jsonsink *s, const char *key,
                                 size_t keylen);
void jsonsink_add_serialized_value(struct jsonsink *s, const char *value,
                                   size_t valuelen);

void jsonsink_array_start(struct jsonsink *s);
void jsonsink_array_end(struct jsonsink *s);

void jsonsink_add_value_uint32(struct jsonsink *s, uint32_t v);
