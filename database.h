#ifndef DATABASE_H
#define DATABASE_H

#include <QtSql>
#include <vector>

struct User {
    int id;
    QString login;
    QString nickname;
    bool banned;
    QString banReason;
};

struct Message {
    int id;
    int senderId;
    QString senderNickname;
    int receiverId;
    QString content;
    QDateTime timestamp;
};

class Database {
public:
    static Database& instance();
    bool connect(const QString& host, int port, const QString& dbName,
                 const QString& user, const QString& password);
    bool registerUser(const QString& login, const QString& password, const QString& nickname);
    bool authenticateUser(const QString& login, const QString& password, int& userId, QString& nickname);
    std::vector<User> getAllUsers();
    bool banUser(int userId, const QString& reason);
    bool unbanUser(int userId);
    bool isBanned(int userId);
    bool saveMessage(int senderId, int receiverId, const QString& content);
    std::vector<Message> getAllMessages();

private:
    Database() = default;
    QSqlDatabase db;
    bool createTables();
};

#endif
