#! /bin/sh

set -e
set -x

TMP=$(mktemp)
./test | python -m json.tool > ${TMP}
diff -up expected.txt ${TMP}
