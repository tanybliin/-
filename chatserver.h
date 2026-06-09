#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QTcpServer>
#include <QMap>

class ClientHandler;

class ChatServer : public QTcpServer {
    Q_OBJECT
public:
    explicit ChatServer(QObject* parent = nullptr);
    bool start(quint16 port);
    void stop();
    void broadcast(const QString& message, int excludeId = -1);
    void kickUser(int userId);
    void banUser(int userId, const QString& reason);
    QMap<int, ClientHandler*> clients() const { return m_clients; }

signals:
    void userConnected(int userId, const QString& nickname);
    void userBanned(int userId, const QString& reason);
    void userKicked(int userId);
    void newPublicMessage(const QString& nickname, const QString& text);

protected:
    void incomingConnection(qintptr handle) override;

private:
    QMap<int, ClientHandler*> m_clients;
    int m_nextId;
};

#endif
