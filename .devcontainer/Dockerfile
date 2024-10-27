FROM ubuntu:22.04

# do this first to get timezones or opencv will be very mad
# well, nevermind, i don't need opencv in this container
# but having timezones is probably good anyway
RUN apt-get update && \
    apt-get install -yq tzdata && \
    ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime && \
    dpkg-reconfigure -f noninteractive tzdata

RUN apt-get update && apt-get install -y \
    software-properties-common \
    wget \
    gnupg \
    lsb-release \
    curl \
    libssl-dev \
    apt-transport-https \
    ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# and grab Rust
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

# Add LLVM's APT repository and install clang-16
RUN wget https://apt.llvm.org/llvm.sh && \
    chmod +x llvm.sh && \
    ./llvm.sh 16 && \
    rm llvm.sh

# Add the Kitware APT repository to install CMake 3.30
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | apt-key add - && \
    apt-add-repository 'deb https://apt.kitware.com/ubuntu/ jammy main' && \
    apt-get update && \
    apt-get install -y cmake=3.30.* && \
    rm -rf /var/lib/apt/lists/*
