#! /bin/sh

set -e
set -x

JSONSINK=..
cc -g -O2 -flto=full -DJSONSINK_ENABLE_ASSERTIONS \
-I ${JSONSINK} \
-o test \
test.c \
${JSONSINK}/jsonsink.c \
${JSONSINK}/jsonsink_serialization.c \
${JSONSINK}/jsonsink_escape.c \
${JSONSINK}/jsonsink_base64.c
