CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,         
    username VARCHAR(255) NOT NULL UNIQUE,  -- ID пользователя
    password VARCHAR(255) NOT NULL          -- пароль пользователя
);

CREATE TABLE IF NOT EXISTS messages (
    id SERIAL PRIMARY KEY,             -- Уникальный идентификатор сообщения
    from_user_id INTEGER NOT NULL,     -- ID отправителя
    to_user_id INTEGER NOT NULL,       -- ID получателя
    content TEXT NOT NULL,             -- Текст сообщения
    timestamp TIMESTAMP NOT NULL DEFAULT NOW(), -- Время отправки
    FOREIGN KEY (from_user_id) REFERENCES users (id) ON DELETE CASCADE, -- Внешний ключ
    FOREIGN KEY (to_user_id) REFERENCES users (id) ON DELETE CASCADE  -- Внешний ключ
);
