#! /bin/sh

set -x
set -e

#DBG="-D JSONSINK_ENABLE_ASSERTIONS"
DBG="-D NDEBUG"
CC="cc -g -O2 -flto=full -undefined dynamic_lookup ${DBG}"
CXX="c++ -g -O2 -flto=full -undefined dynamic_lookup ${DBG}"

${CC} -shared -o malloc_interposer.dylib malloc_interposer.c

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

FPCONV=deps/fpconv/src
${CC} \
-D JSONSINK_BENCH_FPCONV \
-o jsonsink-fpconv \
-I ${JSONSINK} \
-I ${FPCONV} \
bench.c \
rng.c \
jsonsink.c \
${JSONSINK}/jsonsink.c \
${JSONSINK}/jsonsink_serialization_fpconv.c \
${FPCONV}/fpconv.c

${CC} \
-o snprintf \
bench.c \
rng.c \
snprintf.c

# homebrew-installed flatbuffers
flatc --cpp test.fbs
FLATBUFFERS=$(brew --prefix flatbuffers)
${CXX} \
-std=c++17 \
-I ${FLATBUFFERS}/include \
-o flatbuffers \
bench.c \
rng.c \
flatbuffers.cxx

${CC} \
-D JSON_SAX_APIS_SUPPORT \
-o ljson \
-I ${LJSON} \
bench.c \
rng.c \
ljson.c \
${LJSON}/jnum.c \
${LJSON}/json.c

${CC} \
-o ljson_dom \
-I ${LJSON} \
bench.c \
rng.c \
ljson_dom.c \
${LJSON}/jnum.c \
${LJSON}/json.c

RAPIDJSON=deps/rapidjson/include
${CXX} \
-o rapidjson \
-I ${RAPIDJSON} \
bench.c \
rng.c \
rapidjson.cxx

CJSON=deps/cJSON
${CC} \
-o cjson \
-I ${CJSON} \
bench.c \
rng.c \
cjson.c \
${CJSON}/cJSON.c

PARSON=deps/parson
${CC} \
-o parson \
-I ${PARSON} \
bench.c \
rng.c \
parson.c \
${PARSON}/parson.c
