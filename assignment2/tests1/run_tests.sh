#!/usr/bin/bash

for i in {1..20}; do
	cat tests/example$i\_command | xargs ./cacheSim tests/example$i\_trace > our_output$i
	diff -s ref_results/example$i\_output our_output$i
	rm our_output$i
done
echo "All done.. "





