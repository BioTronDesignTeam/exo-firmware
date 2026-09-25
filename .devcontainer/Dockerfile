FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    git \
    git-lfs \
    unzip \
    libusb-1.0-0-dev \
    udev \
    && rm -rf /var/lib/apt/lists/*

COPY stm32cubeclt_1.22-Lin-Deb-x86_64.sh.zip /tmp/clt.sh.zip

RUN cd /tmp \
    && unzip clt.sh.zip \
    && chmod +x stm32cubeclt_*.sh \
    && bash stm32cubeclt_*.sh --noexec --target /tmp/clt_debs \
    && dpkg --force-all -i /tmp/clt_debs/stm32cubeclt-*.deb \
    && rm -rf /tmp/clt.sh.zip /tmp/stm32cubeclt_*.sh /tmp/clt_debs /var/lib/apt/lists/*

ENV STM32CLT_PATH=/opt/st/stm32cubeclt_1.22.0
ENV PATH="${STM32CLT_PATH}/GNU-tools-for-STM32/bin:${STM32CLT_PATH}/STM32CubeProgrammer/bin:${PATH}"
