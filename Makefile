
CC=/home/fangyang/files/ndk/bin/arm-linux-androideabi-gcc
SYSROOT=--sysroot /home/fangyang/files/ndk/sysroot/
#CC=gcc
CFLAGS=-c -g -W -Wall -MMD -MP -O -DNOLOG -DDBG_LOG -I./mpp_inc -I./mpp_osal
LDFLAGS=-fPIC -pthread -L./libmpp -lmpp -pie -fPIE
#LDFLAGS=-fPIC -pthread 

all: mpp_multi
mpp_multi: mpp_multi.o mpp_multi_dec.o mpp_multi_enc.o
	$(CC) $(SYSROOT) -o mpp_multi mpp_multi.o mpp_multi_dec.o mpp_multi_enc.o $(LDFLAGS)
mpp_multi.o: mpp_multi.c
	$(CC) $(SYSROOT) $(CFLAGS) -o mpp_multi.o mpp_multi.c
mpp_multi_dec.o: mpp_multi_dec.c
	$(CC) $(SYSROOT) $(CFLAGS) -o mpp_multi_dec.o mpp_multi_dec.c
mpp_multi_enc.o: mpp_multi_enc.c
	$(CC) $(SYSROOT) $(CFLAGS) -o mpp_multi_enc.o mpp_multi_enc.c
screencap.o: screencap.c
	$(CC) $(SYSROOT) $(CFLAGS) -o screencap.o screencap.c

clean_obj:
	rm -f *.o *.d 

clean:
	rm -f *.o *.d mpp_multi

