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

RAPIDJSON=deps/rapidjson/include
c++ -g -O2 -flto=full \
-o rapidjson \
-I ${RAPIDJSON} \
bench.c \
rng.c \
rapidjson.cxx

PARSON=deps/parson
cc -g -O2 -flto=full \
-o parson \
-I ${PARSON} \
bench.c \
rng.c \
parson.c \
${PARSON}/parson.c
