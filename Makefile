
TARGET += sample.ext4
TARGET += test_ext4

all: $(TARGET)

test: .FORCE
	./test_ext4 sample.ext4

OBJS += test.o
OBJS += ext4.o

CFLAGS += -Wall

test_ext4: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<

sample.dir: .FORCE
	rm -Rf $@
	mkdir $@
	for i in $(seq 0 9); do \
		ls -l > $@/sample$i.txt; \
	done
	for d in 0 1; do \
		mkdir $@/dir$d; \
		for i in $(seq 0 7); do \
			ls -l > $@/dir$d/sample$i.txt; \
		done; \
	done

sample.ext4: sample.dir
	rm -f $@
	dd if=/dev/zero of=$@ bs=1024 seek=$$((4*1024)) count=0
	mkfs.ext4 -d $< $@

.FORCE:
