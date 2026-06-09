
CREATE DATABASE chatdb;
\c chatdb;

CREATE TABLE users (
    id SERIAL PRIMARY KEY,
    login VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    nickname VARCHAR(50) NOT NULL
);

CREATE TABLE banned_users (
    user_id INTEGER PRIMARY KEY REFERENCES users(id),
    reason TEXT
);

CREATE TABLE messages (
    id SERIAL PRIMARY KEY,
    sender_id INTEGER REFERENCES users(id),
    receiver_id INTEGER DEFAULT 0,
    content TEXT NOT NULL,
    timestamp TIMESTAMP DEFAULT NOW()
);

INSERT INTO users (login, password, nickname) VALUES
('alice', 'a665a45920422f9d417e4867efdc4fb8a04a1f3fff1fa07e998e86f7f7a27ae3', 'Alice');
