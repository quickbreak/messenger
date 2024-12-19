# Сборка ---------------------------------------

# В качестве базового образа для сборки используем gcc:latest
FROM gcc:latest AS build

# Установим рабочую директорию для сборки auth_service
WORKDIR /msg_build

# Скачаем все необходимые пакеты
RUN apt update && \
    apt install -y \
      cmake \
      libpqxx-dev \
      # на сайте boost сказано, что зависимости такие:
      build-essential python3 libbz2-dev libz-dev libicu-dev 
      # нельзя просто написать libboost-all-dev, там 1.74.0
    # && \
    # wget https://boostorg.jfrog.io/artifactory/main/release/1.85.0/source/boost_1_85_0.tar.bz2 && \
    # tar xf boost_1_85_0.tar.bz2 && \
    # cd boost_1_85_0 && \
    # ./bootstrap.sh && \
    # ./b2 && \
    # ./b2 install --prefix=/usr/local && \
    # export BOOST_ROOT=/usr/local && \
    # export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH && \
    # export CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH 

ENV BOOST_ROOT=/usr/local
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
ENV CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH

# Скопируем исходники в контейнер
COPY ./src /app/src
COPY ./CMakeLists.txt /app

# Установим рабочую директорию для сборки проекта
WORKDIR /app/build

# Выполним сборку нашего проекта
RUN cmake .. -D BOOST_ROOT=/usr/local/include/boost && \
    make


# Запуск ---------------------------------------

# В качестве базового образа используем ubuntu:latest
# FROM ubuntu:latest

# Установим рабочую директорию нашего приложения
# WORKDIR /app

# Скопируем приложение со сборочного контейнера в рабочую директорию
# COPY --from=build /app/build/http_server .

# Установим точку входа
CMD ["./chat_server"]
