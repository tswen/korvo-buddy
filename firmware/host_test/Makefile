CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -I../main

SRCS = \
	../main/buddy_proto.c \
	../main/buddy_state.c \
	../main/pet_ui.c \
	../main/input_handler.c \
	../main/ble_transport_stub.c \
	../main/status_led.c \
	../main/audio_chime.c \
	test_buddy.c

.PHONY: test clean

test:
	$(CC) $(CFLAGS) -o test_buddy $(SRCS)
	./test_buddy

clean:
	rm -f test_buddy
