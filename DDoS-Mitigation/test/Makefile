CC = aarch64-linux-gnu-gcc

all: puzzle run  

puzzle: puzzle.c
	$(CC) -o puzzle puzzle.c


run:
	mkdir mntdir
	sudo mount ./../tizen-image/rootfs.img ./mntdir
	sudo cp puzzle ./mntdir/root/
	sudo umount ./mntdir
	rmdir mntdir

clean:
	rm -rf puzzle


