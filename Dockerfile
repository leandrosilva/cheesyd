FROM debian-wkhtmltopdf AS base

FROM base AS tools
RUN apt-get update -y
RUN apt-get install -y g++
RUN apt-get install -y libhiredis0.13

FROM tools AS build
WORKDIR /app
COPY bin/*.html ./
WORKDIR /deps
COPY /deps .
WORKDIR /src
COPY src .
RUN g++ -Wall -lwkhtmltox -o /app/cheesyd cheesyd.c

FROM build AS clean
RUN apt-get purge -y --auto-remove g++ \
 	&& rm -rf /var/lib/apt/lists/* \
RUN rm -rf /deps \
    && rm -rf /src

FROM clean AS finish
WORKDIR /app

# ENTRYPOINT [ "cheesyd" ]