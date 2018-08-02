FROM debian-wkhtmltopdf AS base
RUN apt-get update -y

FROM base AS tools
RUN apt-get install -y g++

FROM tools AS deps
RUN apt-get install -y libhiredis0.13

FROM deps AS build
WORKDIR /app
COPY bin/*.html ./
WORKDIR /deps
COPY /deps .
WORKDIR /src
COPY src .
RUN g++ -std=c++17 -Wall -I/deps/include -lwkhtmltox -l:libhiredis.so.0.13 -o /app/cheesyd *.cpp /deps/lib/json11.a

FROM build AS clean
RUN apt-get purge -y --auto-remove g++ \
 	&& rm -rf /var/lib/apt/lists/* \
RUN rm -rf /deps \
    && rm -rf /src

FROM clean AS final
WORKDIR /app

# ENTRYPOINT [ "cheesyd" ]