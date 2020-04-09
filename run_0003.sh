#!/bin/bash

set -e

test_run_0003()
{
	modprobe -r loop
	for i in $(seq 1 2); do
		./break-blktrace -c 3 -d -s
	done
}

for i in $(seq 1 10); do
	test_run_0003
done
