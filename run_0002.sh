#!/bin/bash

set -e
for i in $(seq 1 150); do
	./break-blktrace -c 3 -d -s
done
