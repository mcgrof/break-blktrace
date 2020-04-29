#!/bin/bash

set -e

test_run_0005()
{
	modprobe loop
	blktrace /dev/loop1 &
	sleep 1
	blktrace /dev/loop1 &
	sleep 1
	pkill -P $$
	wait
	echo done
}

test_run_0005
