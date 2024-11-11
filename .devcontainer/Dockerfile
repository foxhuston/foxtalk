FROM ubuntu:24.04

# do this first to get timezones or opencv will be very mad
# well, nevermind, i don't need opencv in this container
# but having timezones is probably good anyway
RUN apt-get update && \
    apt-get install -yq tzdata && \
    ln -fs /usr/share/zoneinfo/America/New_York /etc/localtime && \
    dpkg-reconfigure -f noninteractive tzdata

RUN apt-get update && apt-get install -y \
    wget \
    unzip \
    gnupg \
    lsb-release \
    curl \
    libssl-dev \
    apt-transport-https \
    build-essential \
    gcc \
    g++ \
    ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# and grab Rust
# RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y

# aaaaand Bazel
ARG BAZEL_VERSION=7.4.0
RUN ARCH="$(dpkg --print-architecture)" && \
    if [ "$ARCH" = "amd64" ]; then \
        wget -q https://github.com/bazelbuild/bazel/releases/download/${BAZEL_VERSION}/bazel-${BAZEL_VERSION}-linux-x86_64 -O /usr/local/bin/bazel ; \
    elif [ "$ARCH" = "arm64" ]; then \
        wget -q https://github.com/bazelbuild/bazel/releases/download/${BAZEL_VERSION}/bazel-${BAZEL_VERSION}-linux-arm64 -O /usr/local/bin/bazel ; \
    else \
        echo "Unsupported architecture: $ARCH" && exit 1 ; \
    fi && \
    chmod +x /usr/local/bin/bazel

