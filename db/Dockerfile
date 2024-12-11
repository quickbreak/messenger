FROM postgres:latest

# это нужно для инициализации бд при первом запуске контейнера
ENV POSTGRES_DB: $DB_NAME
ENV POSTGRES_USER: $DB_USER
ENV POSTGRES_PASSWORD: $DB_PASSWORD

# Копируем SQL-скрипт для инициализации
COPY init.sql /docker-entrypoint-initdb.d/
