FROM ubuntu:23.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libboost-all-dev \
    libcurl4-openssl-dev \
    libssl-dev \
    libxml2-dev \
    libomp-dev \
    pkg-config \
    wget \
    zlib1g-dev

WORKDIR /cadmium-src

COPY . /cadmium-src/

RUN git submodule update --init --recursive

VOLUME ["/cadmium/build"]

WORKDIR /cadmium-src/build

RUN cmake ..

RUN cmake --build .

RUN ctest --output-on-failure . 

RUN cmake --install .

RUN rm -rf /cadmium-src

CMD [ "bash" ]