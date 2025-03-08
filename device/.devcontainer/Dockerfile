# syntax = docker/dockerfile:1.2

FROM ubuntu:24.10

RUN apt-get update && \
    apt-get upgrade -y

RUN apt-get install -y \
    ccache \
    cmake \
    g++ \
    gcc-arm-none-eabi \
    git \
    ninja-build \
    python3 \
    python3-venv \
    stlink-tools \
    ;

RUN apt clean

USER ubuntu
WORKDIR /workspace
