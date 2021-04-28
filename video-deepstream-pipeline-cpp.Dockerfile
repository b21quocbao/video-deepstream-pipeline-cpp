FROM nvcr.io/nvidia/deepstream:5.1-21.02-devel

RUN apt update && apt install --no-install-recommends -y wget unzip software-properties-common pkg-config gpg-agent git

# CMAKE
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | apt-key add - && \
    apt-add-repository 'deb https://apt.kitware.com/ubuntu/ bionic main' && \
    apt-get update && \
    apt-get install -y cmake

# TORCH
RUN wget https://download.pytorch.org/libtorch/cu111/libtorch-shared-with-deps-1.8.1%2Bcu111.zip && unzip libtorch-shared-with-deps-1.8.1+cu111.zip && rm libtorch-shared-with-deps-1.8.1+cu111.zip
RUN git clone https://github.com/b21quocbao/video-deepstream-pipeline-cpp 
RUN cd video-deepstream-pipeline-cpp && mkdir build && cd build && cmake .. && make 



