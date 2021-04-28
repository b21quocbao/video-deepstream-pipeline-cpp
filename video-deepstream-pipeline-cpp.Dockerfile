FROM nvcr.io/nvidia/deepstream:5.1-21.02-devel

RUN apt update && apt install --no-install-recommends -y wget unzip software-properties-common pkg-config gpg-agent &&

# CMAKE
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add - && \
    apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' && \
    apt-get update && \
    apt-get install -y cmake

# TORCH
RUN wget https://download.pytorch.org/libtorch/cu102/libtorch-shared-with-deps-1.8.1%2Bcu102.zip && unzip libtorch-shared-with-deps-1.8.1+cu111.zip && rm libtorch-shared-with-deps-1.8.1+cu111.zip



