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
    python3-pip \
    python3-venv \
    stlink-tools \
    sudo \
    ;

RUN apt clean

RUN echo "ubuntu ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Don't use ubuntu's, it's patched
RUN pip3 install --break-system-packages hid

USER ubuntu
