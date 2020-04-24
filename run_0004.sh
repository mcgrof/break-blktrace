#!/bin/bash

set -e

test_run_0004()
{
	for i in $(seq 1 2); do
		./break-blktrace -c 3 -d -s &
		./break-blktrace -c 3 -d -s &
		wait
	done
	echo done
}

test_run_0004
