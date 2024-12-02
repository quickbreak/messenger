FROM postgres:latest

# Устанавливаем переменные окружения
ENV POSTGRES_USER=myuser
ENV POSTGRES_PASSWORD=mypassword
ENV POSTGRES_DB=mydatabase

# Копируем SQL-скрипт для инициализации
COPY init.sql /docker-entrypoint-initdb.d/
