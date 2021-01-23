#!/usr/bin/env bash

build=0
run=0
run_tests=0
gen_coverage=0

proj_root="$(dirname "$0")"
build_dir="${proj_root}/build"

while getopts 'habrtg' flag; do
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
    r)
      run=1
      ;;
		t)
			run_tests=1
			;;
		g)
			gen_coverage=1
			;;
		*)
			echo 'invalid argument given'
			exit 1
			;;
	esac
done

shift $((OPTIND-1))

if [ $build -eq 1 ]; then
	cur_dir=$(pwd)
	cd "${build_dir}"
	make -j$(($(nproc) - 1)) || exit $?
	cd "${cur_dir}"
fi

if [ $run -eq 1 ]; then
	cmd=${build_dir}/SimpleScript
  ${cmd} $@
fi

if [ $run_tests -eq 1 ]; then
	cmd=${build_dir}/SimpleScriptTest
	if [ ! -z "$1" ]; then
		${cmd} --gtest_filter="$1"
	else
		${cmd}
	fi
fi

if [ $gen_coverage -eq 1 ]; then
	cur_dir=$(pwd)
	cd "${build_dir}"
	gcovr -r ../. --html --html-details -o coverage.html || exit $?
	cd "${cur_dir}"
fi
