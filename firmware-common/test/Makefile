# Host unit tests for firmware-common/detect.
#
#   make        build and run
#   make clean
#
# Needs gcc and python3. The parsers have no Arduino/ESP-IDF dependency, so
# the exact code that runs on the XIAO ESP32 nodes is exercised here with
# synthetic Remote ID, DJI DroneID and MAVLink frames.

CC      ?= gcc
CFLAGS  ?= -std=gnu11 -Wall -Wextra -O1 -g -I../detect/src
SRCDIR   = ../detect/src
SRC      = $(SRCDIR)/opendroneid.c $(SRCDIR)/wifi.c \
           $(SRCDIR)/detect_record.c $(SRCDIR)/detect_odid.c $(SRCDIR)/detect_wifi.c \
           $(SRCDIR)/detect_dji.c $(SRCDIR)/detect_mavlink.c $(SRCDIR)/detect_fingerprint.c

all: test sanitize

test: test_detect
	./test_detect > json_samples.txt
	python3 check_json.py json_samples.txt

# The same tests under AddressSanitizer / UBSan: the parsers run on whatever
# the radio delivers, so an out-of-bounds read is a bug even when the plain
# build happens to return the right answer.
sanitize: test_detect_asan
	./test_detect_asan > json_samples_asan.txt
	python3 check_json.py json_samples_asan.txt

test_detect: test_detect.c $(SRC) $(SRCDIR)/detect.h
	$(CC) $(CFLAGS) -o $@ test_detect.c $(SRC) -lm

test_detect_asan: test_detect.c $(SRC) $(SRCDIR)/detect.h
	$(CC) $(CFLAGS) -fsanitize=address,undefined -fno-omit-frame-pointer -o $@ test_detect.c $(SRC) -lm

clean:
	rm -f test_detect test_detect_asan json_samples.txt json_samples_asan.txt

.PHONY: all test sanitize clean
