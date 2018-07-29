build:
	g++ -Wall -I./include src/cheesyd.c -L./lib -lwkhtmltox -o bin/cheesyd
	cp src/sample1.html bin/

run:
	cd bin/ && ./cheesyd
