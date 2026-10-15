FROM registry.suse.com/bci/bci-base:latest

# Version and checksum must change together (sha256 of llvm-mos-linux.tar.xz for that release).
ARG LLVM_MOS_VERSION=v23.3.0
ARG LLVM_MOS_SHA256=9f694497bc93a7c876d8c62325c7d74e7860346d80fbcd0555e19ba6856658c8

RUN zypper --non-interactive install --no-recommends make python3 tar xz curl libstdc++6 \
 && zypper clean -a \
 && curl -fsSL -o /tmp/llvm-mos.tar.xz \
    https://github.com/llvm-mos/llvm-mos-sdk/releases/download/${LLVM_MOS_VERSION}/llvm-mos-linux.tar.xz \
 && echo "${LLVM_MOS_SHA256}  /tmp/llvm-mos.tar.xz" | sha256sum -c - \
 && mkdir /opt/llvm-mos \
 && tar -xJf /tmp/llvm-mos.tar.xz -C /opt/llvm-mos --strip-components=1 \
 && rm /tmp/llvm-mos.tar.xz

ENV PATH=/opt/llvm-mos/bin:$PATH
WORKDIR /src
