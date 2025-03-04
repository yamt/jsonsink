TESTS="jsonsink jsonsink-jnum snprintf ljson rapidjson parson"

# quick test
printf "%13.13s:0b4864bde11d7f8893994605eca96bfc928337d3b04f1a08fcf8a515ca08d4ac\n" expected >&2
for t in $TESTS; do
	printf "%13.13s:" $t >&2
	./$t --test | python -m json.tool | openssl sha256 >&2
done

# convert the flatbuffers output to json so that it's comparable with others
TMP=$(mktemp -d)
./flatbuffers --test > $TMP/test.bin
flatc --raw-binary --json --strict-json test.fbs -- $TMP/test.bin
printf "%13.13s:" flatbuffers >&2
python -m json.tool test.json | openssl sha256 >& 2

# benchmark
for t in $TESTS; do
	./$t
done
./flatbuffers
