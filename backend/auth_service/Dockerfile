# Сборка ---------------------------------------

# В качестве базового образа для сборки используем gcc:latest
FROM gcc:latest AS build

# Установим рабочую директорию для сборки auth_service
WORKDIR /auth_build

# Скачаем все необходимые пакеты
RUN apt update && \
    apt install -y \
      cmake \
      libpqxx-dev \
    # для Crow нужна asio
      libasio-dev \ 
    && \
    git clone https://github.com/CrowCpp/Crow.git && \
    cd Crow && \
    mkdir build && cd build && \
    cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCROW_BUILD_TESTS=OFF \
    make install
# RUN cp -r Crow/include /usr/local/include
# RUN ls /usr/local/include && \
#     echo "MIHAAAAA"
# Скопируем исходники в контейнер                                                сейчас копируются исходники и мусор, надо создать src/
ADD . /app

# Установим рабочую директорию для сборки проекта
WORKDIR /app/build

# Выполним сборку нашего проекта
RUN cmake .. && \
    make


# Запуск ---------------------------------------

# В качестве базового образа используем ubuntu:latest
# FROM ubuntu:latest

# Установим рабочую директорию нашего приложения
# WORKDIR /app

# Скопируем приложение со сборочного контейнера в рабочую директорию
# COPY --from=build /app/build/http_server .

# Установим точку входа
CMD ["./http_server"]
