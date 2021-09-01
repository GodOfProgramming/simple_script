#!/usr/bin/env ruby

require 'optparse'

module Opts
  SETUP = :setup
  CLEAN = :clean
  BUILD = :build
  BUILD_SLOW = :build_slow
  TEST = :test
  COVERAGE = :coverage
  RUN = :run
  ALL = :all
end

PROJECT_ROOT = "#{__dir__}".freeze
PROJECT_NAME = File.basename(PROJECT_ROOT).freeze
PROJECT_NAME_TEST = "#{PROJECT_NAME}Test".freeze
BUILD_DIR = "#{PROJECT_ROOT}/build".freeze

CORES = `nproc`.strip.to_i.freeze

options = {}

OptionParser.new do |opts|
  opts.on('--setup', 'do all initialization tasks') do |_|
    options[Opts::SETUP] = true
  end

  opts.on('-c', '--clean', 'clean build files') do |_|
    options[Opts::CLEAN] = true
  end

  opts.on('-b', '--build', 'build') do |_|
    options[Opts::BUILD] = true
  end

  opts.on('--slow', 'build slowly') do |_|
    options[Opts::BUILD_SLOW] = true
  end

  opts.on('-t', '--test', 'run tests') do |_|
    options[Opts::TEST] = true
  end

  opts.on('--coverage', 'generate code coverage') do |_|
    options[Opts::COVERAGE] = true
  end

  opts.on('-r', '--run', 'run the repl interpreter') do |_|
    options[Opts::RUN] = true
  end

  opts.on('-a', '--all', 'build, test, generate code coverage in that order') do |_|
    options[Opts::ALL] = true
  end
end.parse!

def exit_if_fail(cmd)
  if !system(cmd)
    puts("#{cmd}: failed")
    exit(1)
  end
end

def do_in_dir(path, &blk)
  cur_dir = Dir.pwd
  Dir.chdir(path)
  yield
  Dir.chdir(cur_dir)
end

if options[Opts::SETUP]
  do_in_dir(PROJECT_ROOT) do
    system('git submodule update --init --recursive')
  end
end

if options[Opts::CLEAN]
  do_in_dir(BUILD_DIR) do
    exit_if_fail("make -j #{CORES - 1} clean")
  end
end

if options[Opts::BUILD]
  do_in_dir(BUILD_DIR) do
    if options[Opts::BUILD_SLOW]
      exit_if_fail('make')
    else
      exit_if_fail("make -j #{CORES - 1}")
    end
  end
end

if options[Opts::TEST]
  do_in_dir(BUILD_DIR) do
    cmd = "#{PROJECT_NAME_TEST}"
    if ARGV.length == 1
      cmd = "#{cmd} --gtest_filter=#{ARGV[1]}"
    end
    exit_if_fail(cmd)
  end
end

if options[Opts::COVERAGE]
  do_in_dir(BUILD_DIR) do
    exit_if_fail('gcovr -r ../. --html --html-details -o coverage.html')
  end
end

if options[Opts::RUN]
  do_in_dir(BUILD_DIR) do
    exit_if_fail("#{PROJECT_NAME} #{ARGV.join(' ')}")
  end
end
