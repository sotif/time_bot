FROM gcc:latest AS builder
WORKDIR /app
COPY . .
RUN make

FROM ubuntu:latest 
RUN apt-get update; apt-get install libcurl4-openssl-dev -y
COPY --from=builder /app/bot /app/bot
COPY config.json /app/config.json
COPY concord /app/concord/
WORKDIR /app

CMD ["/app/bot"]
