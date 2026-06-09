#include "chatserver.h"
#include "clienthandler.h"
#include "database.h"

ChatServer::ChatServer(QObject* parent) : QTcpServer(parent), m_nextId(1) {}

bool ChatServer::start(quint16 port) { return listen(QHostAddress::Any, port); }

void ChatServer::stop() {
    close();
    for (auto* c : m_clients) {
        c->disconnectClient();
        delete c;
    }
    m_clients.clear();
}

void ChatServer::incomingConnection(qintptr handle) {
    auto* h = new ClientHandler(handle, m_nextId++);
    connect(h, &ClientHandler::authenticated, [this](int uid, const QString& nick) {
        m_clients[uid] = h;
        emit userConnected(uid, nick);
        broadcast(nick + " joined the chat", uid);
    });
    connect(h, &ClientHandler::newMessage, [this](int uid, const QString& nick, const QString& text) {
        emit newPublicMessage(nick, text);
        broadcast(nick + ": " + text, uid);
    });
    connect(h, &ClientHandler::disconnected, [this, h]() {
        int uid = h->userId();
        if (uid > 0) {
            m_clients.remove(uid);
            broadcast(h->nickname() + " left the chat");
        }
        h->deleteLater();
    });
    h->start();
}

void ChatServer::broadcast(const QString& message, int excludeId) {
    for (auto it = m_clients.begin(); it != m_clients.end(); ++it) {
        if (it.key() != excludeId)
            it.value()->sendMessage(message);
    }
}

void ChatServer::kickUser(int userId) {
    if (m_clients.contains(userId)) {
        m_clients[userId]->sendMessage("KICKED");
        m_clients[userId]->disconnectClient();
        emit userKicked(userId);
    }
}

void ChatServer::banUser(int userId, const QString& reason) {
    Database::instance().banUser(userId, reason);
    kickUser(userId);
    emit userBanned(userId, reason);
}
