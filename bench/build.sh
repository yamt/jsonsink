#! /bin/sh

JSONSINK=..
cc -g -O2 -flto=full \
-o jsonsink \
-I ${JSONSINK} \
bench.c \
rng.c \
jsonsink.c \
${JSONSINK}/jsonsink.c \
${JSONSINK}/jsonsink_serialization.c
