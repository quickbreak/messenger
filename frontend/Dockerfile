FROM nginx:stable

# Копируем статические файлы в контейнер
COPY ./ /usr/share/nginx/html/

EXPOSE 80

# эта команда и так дефолтная для nginx
# CMD ["nginx", "-g", "daemon off;"]