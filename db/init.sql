CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,         
    username VARCHAR(255) NOT NULL UNIQUE, 
    password VARCHAR(255) NOT NULL
);

CREATE TABLE IF NOT EXISTS messages (
    id SERIAL PRIMARY KEY,         
    from_user VARCHAR(255) NOT NULL, 
    to_user VARCHAR(255) NOT NULL,   
    content TEXT NOT NULL,           
    timestamp TIMESTAMP NOT NULL DEFAULT NOW() 
);
