# Dockerfile for reproducible System 7.1 build
# Ensures consistent toolchain versions across all builds

FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    gcc-multilib \
    g++-multilib \
    nasm \
    grub-pc-bin \
    grub-common \
    xorriso \
    mtools \
    python3 \
    python3-pip \
    xxd \
    wget \
    git \
    ripgrep \
    universal-ctags \
    && rm -rf /var/lib/apt/lists/*

# Install PMD (for CPD analysis)
RUN cd /opt && \
    wget -q https://github.com/pmd/pmd/releases/download/pmd_releases%2F7.0.0/pmd-dist-7.0.0-bin.zip && \
    unzip -q pmd-dist-7.0.0-bin.zip && \
    rm pmd-dist-7.0.0-bin.zip && \
    ln -s /opt/pmd-bin-7.0.0/bin/pmd /usr/local/bin/pmd

# Set up working directory
WORKDIR /build

# Copy build script
COPY prove/build_and_hash.sh /usr/local/bin/build_and_hash.sh
RUN chmod +x /usr/local/bin/build_and_hash.sh

# Default command: run build
CMD ["/usr/local/bin/build_and_hash.sh"]

# Usage:
#   docker build -t system71-build -f prove/Dockerfile .
#   docker run --rm -v $(pwd):/build system71-build
