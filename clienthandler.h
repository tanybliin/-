#ifndef CLIENTHANDLER_H
#define CLIENTHANDLER_H

#include <QTcpSocket>

class ClientHandler : public QObject {
    Q_OBJECT
public:
    ClientHandler(qintptr socketDescriptor, int id, QObject* parent = nullptr);
    void start();
    void sendMessage(const QString& msg);
    void disconnectClient();
    int userId() const { return m_userId; }
    QString nickname() const { return m_nickname; }

signals:
    void authenticated(int userId, const QString& nickname);
    void newMessage(int senderId, const QString& nickname, const QString& text);
    void disconnected();

private slots:
    void onReadyRead();

private:
    QTcpSocket* m_socket;
    int m_id;
    int m_userId;
    QString m_nickname;
    QByteArray m_buffer;
    bool m_authenticated;
};

#endif
