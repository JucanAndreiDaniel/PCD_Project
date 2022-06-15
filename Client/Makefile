include ../config.mk

all: client adminClient

client: client.c
	$(CC) -o client client.c

adminClient: adminClient.c
	$(CC) -o adminClient adminClient.c

testClient: client
	./client

testAdmin: adminClient
	./adminClient

clean:
	$(RM) adminClient client