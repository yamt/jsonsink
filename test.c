#include <stdio.h>
#include <stdlib.h>

#include "jsonsink.h"

int
main(int argc, char **argv)
{
        char buf[1000];
        struct jsonsink s;
        jsonsink_init(&s);
        jsonsink_set_buffer(&s, buf, sizeof(buf));
        jsonsink_object_start(&s);
        jsonsink_add_serialized_key(&s, "\"key1\"", 6);
        jsonsink_object_start(&s);
        jsonsink_add_serialized_key(&s, "\"key1\"", 6);
        jsonsink_add_serialized_value(&s, "100", 3);
        jsonsink_add_serialized_key(&s, "\"key2\"", 6);
        jsonsink_add_serialized_value(&s, "200", 3);
        jsonsink_add_serialized_key(&s, "\"array1\"", 8);
        jsonsink_array_start(&s);
        jsonsink_array_start(&s);
        jsonsink_add_serialized_value(&s, "1", 1);
        jsonsink_add_serialized_value(&s, "1", 1);
        jsonsink_add_serialized_value(&s, "1", 1);
        jsonsink_array_end(&s);
        jsonsink_array_start(&s);
        jsonsink_add_serialized_value(&s, "2", 1);
        jsonsink_add_serialized_value(&s, "2", 1);
        jsonsink_add_serialized_value(&s, "2", 1);
        jsonsink_array_end(&s);
        jsonsink_array_end(&s);
        jsonsink_object_end(&s);
        jsonsink_object_end(&s);
        int error = jsonsink_error(&s);
        void *p = jsonsink_pointer(&s);
        size_t sz = jsonsink_size(&s);
        jsonsink_clear(&s);
        if (error != 0) {
                printf("jsonsink error: %d\n", error);
                return 1;
        }
        fwrite(p, sz, 1, stdout);
}
