TESTS="jsonsink jsonsink-jnum ljson rapidjson parson"

# quick test
printf "%13.13s:0b4864bde11d7f8893994605eca96bfc928337d3b04f1a08fcf8a515ca08d4ac\n" expected >&2
for t in $TESTS; do
	printf "%13.13s:" $t >&2
	./$t --test | python -m json.tool | openssl sha256 >&2
done

# benchmark
for t in $TESTS; do
	./$t
done
