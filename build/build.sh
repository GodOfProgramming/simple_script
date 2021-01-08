#!/usr/bin/env bash

build=0
run_tests=0

while getopts 'hbt' flag; do
	case "$flag" in
		h)
			echo 'build.sh [flags]'
			exit 0 
			;;
		b)
			build=1
			;;
		t)
			run_tests=1
			;;
		*)
			echo 'invalid argument given'
			exit 0
			;;
	esac
done

if [ $build -eq 1 ]; then
	make
fi

if [ $run_tests -eq 1 ]; then
	SimpleScriptTest
fi

