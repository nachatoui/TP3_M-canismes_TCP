all: ServeurUDP ClientUDP

ServeurUDP: ServeurUDP.c
	gcc ServeurUDP.c -o ServeurUDP

ClientUDP: ClientUDP.c
	gcc ClientUDP.c -o ClientUDP

clean:
	rm -f ServeurUDP
	rm -f ClientUDP