
TARGET += test_ext4

all: $(TARGET)

test: .FORCE
	./test_ext4 sample.ext4 list /
	./test_ext4 sample.ext4 list /dir1
	./test_ext4 sample.ext4 cat  /dir1/sample7.txt
	./test_ext4 sample.ext4 list /dir1/big
	./test_ext4 sample.ext4 cat  /dir1/big > big
	diff sample.dir/dir1/big big

OBJS += test.o
OBJS += ext4.o

CFLAGS += -Wall
#CFLAGS += -ggdb -O0
CFLAGS += -O2

test_ext4: $(OBJS)
	$(CC) -o $@ $(LDFLAGS) $^

.c.o:
	$(CC) -o $@ -c $(CFLAGS) $<

sample.dir: .FORCE
	rm -Rf $@
	mkdir $@
	for i in $$(seq 0 9); do \
		ls -l > $@/sample$i.txt; \
	done
	for d in 0 1; do \
		mkdir $@/dir$$d; \
		for i in $$(seq 0 7); do \
			ls -lR > $@/dir$$d/sample$$i.txt; \
		done; \
	done
	dd if=/dev/random of=sample.dir/dir1/big bs=1024 count=$$((48*1024))

sample.ext4: sample.dir
	rm -f $@
	dd if=/dev/zero of=$@ bs=1024 seek=$$((64*1024)) count=0
	mkfs.ext4 -d $< $@

.FORCE:
