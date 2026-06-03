# syntax=docker/dockerfile:1

#Budowanie 
FROM alpine:3.20 AS builder

WORKDIR /app
RUN apk add --no-cache g++

COPY main.cpp .

RUN g++ -O2 -static -s -std=c++17 -pthread main.cpp -o app

#Etap wykonywania
FROM alpine:3.20

WORKDIR /app

RUN apk add --no-cache curl

COPY --from=builder /app/app .

LABEL org.opencontainers.image.authors="Łukasz Pisula"
LABEL org.opencontainers.image.title="Weather App"

EXPOSE 8080

HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
  CMD curl -f http://localhost:8080/ || exit 1

CMD ["./app"]