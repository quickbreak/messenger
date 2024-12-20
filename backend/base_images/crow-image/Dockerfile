FROM gcc:latest AS build

RUN apt update && \
    apt install -y \
      cmake \
    # для Crow нужна asio
      libasio-dev
RUN git clone https://github.com/CrowCpp/Crow.git && \
    cd Crow && \
    mkdir build && cd build && \
    cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF \
    # почему-то это не срабатывает
    make install
    # и приходится копировать include вручную
RUN cp -r Crow/include/* /usr/local/include
