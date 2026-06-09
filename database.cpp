#include "database.h"
#include <QCryptographicHash>

Database& Database::instance() {
    static Database inst;
    return inst;
}

bool Database::connect(const QString& host, int port, const QString& dbName,
                       const QString& user, const QString& password) {
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);
    if (!db.open()) return false;
    return createTables();
}

bool Database::createTables() {
    QSqlQuery q;
    q.exec("CREATE TABLE IF NOT EXISTS users ("
           "id SERIAL PRIMARY KEY, login VARCHAR(50) UNIQUE NOT NULL, "
           "password VARCHAR(255) NOT NULL, nickname VARCHAR(50) NOT NULL)");
    q.exec("CREATE TABLE IF NOT EXISTS banned_users ("
           "user_id INTEGER PRIMARY KEY REFERENCES users(id), reason TEXT)");
    q.exec("CREATE TABLE IF NOT EXISTS messages ("
           "id SERIAL PRIMARY KEY, sender_id INTEGER REFERENCES users(id), "
           "receiver_id INTEGER DEFAULT 0, content TEXT NOT NULL, timestamp TIMESTAMP DEFAULT NOW())");
    return true;
}

bool Database::registerUser(const QString& login, const QString& password, const QString& nickname) {
    QSqlQuery q;
    q.prepare("INSERT INTO users (login, password, nickname) VALUES (?, ?, ?)");
    q.addBindValue(login);
    q.addBindValue(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    q.addBindValue(nickname);
    return q.exec();
}

bool Database::authenticateUser(const QString& login, const QString& password, int& userId, QString& nickname) {
    QSqlQuery q;
    q.prepare("SELECT id, nickname FROM users WHERE login = ? AND password = ?");
    q.addBindValue(login);
    q.addBindValue(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
    if (q.exec() && q.next()) {
        userId = q.value(0).toInt();
        nickname = q.value(1).toString();
        return !isBanned(userId);
    }
    return false;
}

std::vector<User> Database::getAllUsers() {
    std::vector<User> users;
    QSqlQuery q("SELECT u.id, u.login, u.nickname, "
                "CASE WHEN b.user_id IS NOT NULL THEN true ELSE false END, "
                "COALESCE(b.reason, '') FROM users u LEFT JOIN banned_users b ON u.id = b.user_id");
    while (q.next()) {
        users.push_back({q.value(0).toInt(), q.value(1).toString(), q.value(2).toString(),
                         q.value(3).toBool(), q.value(4).toString()});
    }
    return users;
}

bool Database::banUser(int userId, const QString& reason) {
    QSqlQuery q;
    q.prepare("INSERT INTO banned_users (user_id, reason) VALUES (?, ?)");
    q.addBindValue(userId);
    q.addBindValue(reason);
    return q.exec();
}

bool Database::unbanUser(int userId) {
    QSqlQuery q;
    q.prepare("DELETE FROM banned_users WHERE user_id = ?");
    q.addBindValue(userId);
    return q.exec();
}

bool Database::isBanned(int userId) {
    QSqlQuery q;
    q.prepare("SELECT 1 FROM banned_users WHERE user_id = ?");
    q.addBindValue(userId);
    return q.exec() && q.next();
}

bool Database::saveMessage(int senderId, int receiverId, const QString& content) {
    QSqlQuery q;
    q.prepare("INSERT INTO messages (sender_id, receiver_id, content) VALUES (?, ?, ?)");
    q.addBindValue(senderId);
    q.addBindValue(receiverId);
    q.addBindValue(content);
    return q.exec();
}

std::vector<Message> Database::getAllMessages() {
    std::vector<Message> msgs;
    QSqlQuery q("SELECT m.id, m.sender_id, u.nickname, m.receiver_id, m.content, m.timestamp "
                "FROM messages m JOIN users u ON m.sender_id = u.id ORDER BY m.timestamp DESC LIMIT 500");
    while (q.next()) {
        msgs.push_back({q.value(0).toInt(), q.value(1).toInt(), q.value(2).toString(),
                        q.value(3).toInt(), q.value(4).toString(), q.value(5).toDateTime()});
    }
    return msgs;
}
