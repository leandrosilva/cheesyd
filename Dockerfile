FROM jessie-wkhtmltopdf AS base

FROM base AS tools
RUN apt-get update -y
RUN apt-get install -y g++

FROM tools AS build
WORKDIR /src
COPY src .
RUN g++ -Wall -I. cheesyd.c -lwkhtmltox -o cheesyd

FROM build AS clean
RUN apt-get purge -y --auto-remove g++ \
	&& rm -rf /var/lib/apt/lists/*

FROM build AS publish
WORKDIR /app
COPY --from=build /src/cheesyd .
COPY --from=build /src/sample1.html .

# ENTRYPOINT [ "cheesyd" ]