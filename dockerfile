FROM alpine:latest

RUN apk add --update \
    g++ \
    make

COPY . /app

WORKDIR /app/src

RUN make

CMD ["./redis++"]