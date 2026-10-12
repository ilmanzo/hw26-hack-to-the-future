FROM registry.suse.com/bci/bci-base:latest

ARG LLVM_MOS_VERSION=v23.3.0

RUN zypper --non-interactive install --no-recommends make python3 tar xz curl libstdc++6 \
 && zypper clean -a \
 && mkdir /opt/llvm-mos \
 && curl -fsSL https://github.com/llvm-mos/llvm-mos-sdk/releases/download/${LLVM_MOS_VERSION}/llvm-mos-linux.tar.xz \
    | tar -xJ -C /opt/llvm-mos --strip-components=1

ENV PATH=/opt/llvm-mos/bin:$PATH
WORKDIR /src
