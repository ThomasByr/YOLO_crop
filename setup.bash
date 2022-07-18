#!/usr/bin/env bash
# -*- coding: utf-8 -*-

printf "\n\033[96m> running tests...\033[0m\n\n"

# check if valgrind is installed
if ! which valgrind >/dev/null; then
  printf "\n\033[96m> valgrind is not installed\033[0m\n\n"
  exit 1
fi

# jump to tests directory
cd tests || exit 1

# run tests
make check || exit 1

# clean up
make clean && cd ..

# building the project
printf "\n\033[96m> tests done : building...\033[0m\n\n"
make clean && make generic
