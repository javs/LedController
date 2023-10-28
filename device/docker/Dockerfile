# syntax = docker/dockerfile:1.2

FROM archlinux:base-devel

# install packages
RUN --mount=type=cache,sharing=locked,target=/var/cache/pacman \
    pacman -Syu --noconfirm --needed \
        git base-devel sudo

RUN useradd --create-home build
RUN echo "build ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER build
RUN --mount=type=cache,sharing=locked,target=/var/cache/pacman \
    cd \
    && git clone https://aur.archlinux.org/yay.git \
    && cd yay \
    && chmod a+rw -R . \
    && makepkg --noconfirm -si \
    && cd \
    && rm -rf yay

RUN --mount=type=cache,sharing=locked,target=/var/cache/pacman \
    yay -Syu --noconfirm --needed \
        cmake \
        gcc-arm-none-eabi-bin \
        python-pip \
        python-setuptools \
        python-prettytable \
        python-future \
        python-intelhex

USER root

RUN python -m pip install --break-system-packages \
    jinja2 \
    mbed-tools

