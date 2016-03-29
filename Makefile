TARGET = parallel_histogram #sequential_histogram
SOURCES = parallel_histogram.cc #sequential_histogram.c

THREADS ?= 64
APP_FLAGS ?= $(CURDIR)/input_keys 
CORES = $(THREADS)

include ../../Makefile.tests
