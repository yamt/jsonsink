#! /bin/sh

JSONSINK=..
cc -g -O2 -flto=full \
-D JSONSINK_ENABLE_ASSERTIONS \
-o jsonsink \
-I ${JSONSINK} \
bench.c \
rng.c \
jsonsink.c \
${JSONSINK}/jsonsink.c \
${JSONSINK}/jsonsink_serialization.c

LJSON=deps/ljson
cc -g -O2 -flto=full \
-D JSONSINK_ENABLE_ASSERTIONS \
-o jsonsink-jnum \
-I ${JSONSINK} \
-I ${LJSON} \
bench.c \
rng.c \
jsonsink.c \
${JSONSINK}/jsonsink.c \
${JSONSINK}/jsonsink_serialization_jnum.c \
${LJSON}/jnum.c

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
