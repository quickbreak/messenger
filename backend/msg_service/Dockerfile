# Взять образ со скачанной boost (и установленными переменными окружения)
FROM quickbreak/boost-image:latest

# Скачать pqxx
RUN apt update && apt install -y \
      cmake \
      libpqxx-dev

# Скопировать исходники в контейнер
COPY ./src /app/src
COPY ./CMakeLists.txt /app

# Установить рабочую директорию для сборки проекта
WORKDIR /app/build

# Выполнить сборку проекта
RUN cmake .. -D BOOST_ROOT=/usr/local/include/boost && \
    make


# Установить точку входа
CMD ["./msg_server"]
