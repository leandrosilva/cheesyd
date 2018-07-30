build:
	g++ -Wall -I./deps/hiredis/include -I./deps/wkhtmltopdf/include -L./deps/hiredis/lib -L./deps/wkhtmltopdf/lib -lwkhtmltox -o ./bin/cheesyd ./src/cheesyd.c

run:
	cd ./bin/ && ./cheesyd
