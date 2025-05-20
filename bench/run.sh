TESTS="jsonsink jsonsink-jnum jsonsink-fpconv snprintf ljson ljson_dom rapidjson cjson parson"

# note: macOS's system openssl seems to have a bit differnt output format.
# it's preferred for this script.
if type /usr/bin/openssl; then
dosha256() {
	/usr/bin/openssl sha256 $1
}
else
dosha256() {
	openssl sha256 $1
}
fi

# quick test
printf "%15.15s:0b4864bde11d7f8893994605eca96bfc928337d3b04f1a08fcf8a515ca08d4ac\n" expected >&2
for t in $TESTS; do
	printf "%15.15s:" $t >&2
	./$t --test | python -m json.tool | dosha256 >&2
done

# convert the flatbuffers output to json so that it's comparable with others
TMP=$(mktemp -d)
./flatbuffers --test > $TMP/test.bin
flatc --raw-binary --json --strict-json test.fbs -- $TMP/test.bin
printf "%15.15s:" flatbuffers >&2
python -m json.tool test.json | dosha256 >& 2

for t in $TESTS; do
	printf "%15.15s:" $t >&2
	./$t --test | wc -c >&2
done
printf "%15.15s:" flatbuffers >&2
./flatbuffers --test | wc -c >&2

# benchmark
for t in $TESTS; do
	DYLD_INSERT_LIBRARIES=malloc_interposer.dylib ./$t
done
DYLD_INSERT_LIBRARIES=malloc_interposer.dylib ./flatbuffers
