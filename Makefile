SERVICE = service
cc = gcc
CFLAGS := -Wall -g
LDFLAGS := -lpthread -lmysqlclient

OBJ_SERVICE = service.o mysql_save.o

all: $(OBJ_SERVICE) $(OBJ_CLIENT)
	make service

service.o:service.c global.h
	$(cc) $(CFLAGS) -c service.c

mysql_save.o:mysql_save.c mysql_save.h global.h
	$(cc) $(CFLAGS) -c mysql_save.c

$(SERVICE):$(OBJ_SERVICE)
	$(cc) $(LDFLAGS) $^ -o $(SERVICE)

clean :
	rm -f *.o $(SERVICE)
