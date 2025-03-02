#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "jsonsink.h"

void
jsonsink_add_value_uint32(struct jsonsink *s, uint32_t v)
{
        char buf[sizeof("4294967295")];
        int ret = snprintf(buf, sizeof(buf), "%" PRIu32, v);
        if (ret < 0) {
                jsonsink_set_error(s, JSONSINK_ERROR_SERIALIZATION);
                return;
        }
        assert(ret < sizeof(buf));
        jsonsink_add_serialized_value(s, buf, ret);
}
