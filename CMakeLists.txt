cmake_minimum_required(VERSION 3.9)
project(WordSimilarity)

set(CMAKE_CXX_STANDARD 11)

add_executable(WordSimilarity
        hownet/glossary.dat
        hownet/WHOLE.DAT
        test/result.txt
        test/sim_test_1.sh
        test/sim_test_2.sh
        Makefile
        README.md
        similarity.cpp
        similarity.h
        utility.h)
