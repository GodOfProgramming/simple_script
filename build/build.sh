#!/usr/bin/env bash

build=0
run_tests=0
gen_coverage=0

while getopts 'habtg' flag; do
	case "$flag" in
		h)
			echo 'build.sh [flags]'
			exit 0
			;;
		a)
			build=1
			run_tests=1
			gen_coverage=1
			;;
		b)
			build=1
			;;
		t)
			run_tests=1
			;;
		g)
			gen_coverage=1
			;;
		*)
			echo 'invalid argument given'
			exit 0
			;;
	esac
done

if [ $build -eq 1 ]; then
	make -j$(($(nproc) - 1)) || exit $?
fi

if [ $run_tests -eq 1 ]; then
	SimpleScriptTest
fi

if [ $gen_coverage -eq 1 ]; then
	gcovr -r ../. --html --html-details -o coverage.html || exit $?
fi
