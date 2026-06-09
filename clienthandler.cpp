#include "clienthandler.h"
#include "database.h"

ClientHandler::ClientHandler(qintptr socketDescriptor, int id, QObject* parent)
    : QObject(parent), m_id(id), m_userId(-1), m_authenticated(false) {
    m_socket = new QTcpSocket(this);
    m_socket->setSocketDescriptor(socketDescriptor);
}

void ClientHandler::start() {
    connect(m_socket, &QTcpSocket::readyRead, this, &ClientHandler::onReadyRead);
    connect(m_socket, &QTcpSocket::disconnected, this, &ClientHandler::disconnected);
}

void ClientHandler::onReadyRead() {
    m_buffer.append(m_socket->readAll());
    while (m_buffer.contains('\n')) {
        int idx = m_buffer.indexOf('\n');
        QByteArray line = m_buffer.left(idx).trimmed();
        m_buffer.remove(0, idx + 1);
        if (line.isEmpty()) continue;

        QList<QByteArray> parts = line.split(' ');
        if (parts.isEmpty()) continue;

        if (parts[0] == "AUTH" && parts.size() >= 3 && !m_authenticated) {
            QString login = QString::fromUtf8(parts[1]);
            QString pass = QString::fromUtf8(parts[2]);
            if (Database::instance().authenticateUser(login, pass, m_userId, m_nickname)) {
                m_authenticated = true;
                sendMessage("AUTH_OK");
                emit authenticated(m_userId, m_nickname);
            } else {
                sendMessage("AUTH_FAIL");
                m_socket->disconnectFromHost();
            }
        }
        else if (parts[0] == "MSG" && m_authenticated && parts.size() >= 2) {
            QString text = QString::fromUtf8(parts.mid(1).join(' '));
            Database::instance().saveMessage(m_userId, 0, text);
            emit newMessage(m_userId, m_nickname, text);
        }
    }
}

void ClientHandler::sendMessage(const QString& msg) {
    m_socket->write(msg.toUtf8() + "\n");
}

void ClientHandler::disconnectClient() {
    m_socket->disconnectFromHost();
}
