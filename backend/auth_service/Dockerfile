# Взять образ со скачанной crow
FROM quickbreak/crow-image:latest

# Скачать pqxx
RUN apt update && apt install -y \
      cmake \
      libpqxx-dev

# Скопируем исходники в контейнер
COPY ./src /app/src
COPY ./CMakeLists.txt /app

# Установить рабочую директорию для сборки проекта
WORKDIR /app/build

# Выполнить сборку проекта
RUN cmake .. && \
    make


# Установить точку входа
CMD ["./auth_server"]
