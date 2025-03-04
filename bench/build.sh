#! /bin/sh

DBG="-D JSONSINK_ENABLE_ASSERTIONS"
CC="cc -g -O2 -flto=full ${DBG}"
CXX="c++ -g -O2 -flto=full ${DBG}"

JSONSINK=..
${CC} \
-o jsonsink \
-I ${JSONSINK} \
bench.c \
rng.c \
jsonsink.c \
${JSONSINK}/jsonsink.c \
${JSONSINK}/jsonsink_serialization.c

LJSON=deps/ljson
${CC} \
-D JSONSINK_BENCH_JNUM \
-o jsonsink-jnum \
-I ${JSONSINK} \
-I ${LJSON} \
bench.c \
rng.c \
jsonsink.c \
${JSONSINK}/jsonsink.c \
${JSONSINK}/jsonsink_serialization_jnum.c \
${LJSON}/jnum.c

${CC} \
-D JSON_SAX_APIS_SUPPORT \
-o ljson \
-I ${LJSON} \
bench.c \
rng.c \
ljson.c \
${LJSON}/jnum.c \
${LJSON}/json.c

RAPIDJSON=deps/rapidjson/include
${CXX} \
-o rapidjson \
-I ${RAPIDJSON} \
bench.c \
rng.c \
rapidjson.cxx

PARSON=deps/parson
${CC} \
-o parson \
-I ${PARSON} \
bench.c \
rng.c \
parson.c \
${PARSON}/parson.c
