#!/usr/bin/env bash
# -*- coding: utf-8 -*-

echo "> running tests..."

# check if valgrind is installed
if ! which valgrind >/dev/null; then
  echo "> valgrind is not installed"
  exit 1
fi

# jump to tests directory
cd tests || echo "> tests directory not found" && exit 1

# run tests
make check || echo "> tests failed" && exit 1

# clean up
make clean && cd ..

# building the project
echo "> tests done : building..."
make clean && make release