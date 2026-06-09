#include "serverwindow.h"
#include "chatserver.h"
#include "database.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QInputDialog>

ServerWindow::ServerWindow(QWidget* parent)
    : QWidget(parent), m_server(nullptr), m_running(false) {
    setupUI();
    loadUsers();
    loadMessages();
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &ServerWindow::refresh);
    m_timer->start(3000);
}

ServerWindow::~ServerWindow() { if (m_server) m_server->stop(); }

void ServerWindow::setupUI() {
    setWindowTitle("Server Admin Panel");
    resize(1000, 700);
    setStyleSheet(
        "QWidget { background-color: #2b2b2b; color: #f0f0f0; font-size: 13px; }"
        "QGroupBox { border: 1px solid #555; border-radius: 4px; margin-top: 10px; }"
        "QListWidget, QTextEdit { background-color: #3c3c3c; border: 1px solid #555; }"
        "QPushButton { background-color: #0d6efd; border: none; border-radius: 3px; padding: 6px 12px; }"
        "QPushButton:hover { background-color: #0b5ed7; }"
        "QSpinBox { background-color: #3c3c3c; border: 1px solid #555; }"
    );

    auto* main = new QVBoxLayout(this);
    auto* top = new QHBoxLayout();
    m_startBtn = new QPushButton("Start Server");
    m_portEdit = new QSpinBox();
    m_portEdit->setRange(1024, 65535);
    m_portEdit->setValue(5555);
    top->addWidget(m_startBtn);
    top->addWidget(new QLabel("Port:"));
    top->addWidget(m_portEdit);
    top->addStretch();
    main->addLayout(top);

    auto* content = new QHBoxLayout();
    auto* usersGroup = new QGroupBox("Users");
    auto* usersLayout = new QVBoxLayout(usersGroup);
    m_userList = new QListWidget();
    usersLayout->addWidget(m_userList);
    auto* btnRow = new QHBoxLayout();
    auto* kickBtn = new QPushButton("Kick");
    auto* banBtn = new QPushButton("Ban");
    auto* unbanBtn = new QPushButton("Unban");
    btnRow->addWidget(kickBtn);
    btnRow->addWidget(banBtn);
    btnRow->addWidget(unbanBtn);
    usersLayout->addLayout(btnRow);

    auto* msgsGroup = new QGroupBox("All Messages");
    auto* msgsLayout = new QVBoxLayout(msgsGroup);
    m_msgView = new QTextEdit();
    m_msgView->setReadOnly(true);
    msgsLayout->addWidget(m_msgView);

    content->addWidget(usersGroup, 1);
    content->addWidget(msgsGroup, 2);
    main->addLayout(content);

    connect(m_startBtn, &QPushButton::clicked, this, &ServerWindow::startStop);
    connect(kickBtn, &QPushButton::clicked, this, &ServerWindow::kick);
    connect(banBtn, &QPushButton::clicked, this, &ServerWindow::ban);
    connect(unbanBtn, &QPushButton::clicked, this, &ServerWindow::unban);
}

void ServerWindow::startStop() {
    if (!m_running) {
        m_server = new ChatServer(this);
        connect(m_server, &ChatServer::userConnected, [this](int, const QString&) { refresh(); });
        connect(m_server, &ChatServer::userBanned, [this](int, const QString&) { refresh(); });
        connect(m_server, &ChatServer::newPublicMessage, [this](const QString&, const QString&) { refresh(); });
        if (m_server->start(m_portEdit->value())) {
            m_running = true;
            m_startBtn->setText("Stop Server");
            m_portEdit->setEnabled(false);
        } else {
            delete m_server;
            m_server = nullptr;
            QMessageBox::critical(this, "Error", "Cannot start server");
        }
    } else {
        m_server->stop();
        delete m_server;
        m_server = nullptr;
        m_running = false;
        m_startBtn->setText("Start Server");
        m_portEdit->setEnabled(true);
    }
}

void ServerWindow::refresh() { loadUsers(); loadMessages(); }

void ServerWindow::loadUsers() {
    auto users = Database::instance().getAllUsers();
    m_userList->clear();
    for (const auto& u : users) {
        QString txt = u.nickname + " (" + u.login + ")";
        if (u.banned) txt += " [BANNED]";
        m_userList->addItem(txt);
    }
}

void ServerWindow::loadMessages() {
    auto msgs = Database::instance().getAllMessages();
    m_msgView->clear();
    for (const auto& m : msgs) {
        QString type = (m.receiverId == 0) ? "public" : "private";
        m_msgView->append(QString("[%1] [%2] %3: %4")
            .arg(m.timestamp.toString("hh:mm:ss"), type, m.senderNickname, m.content));
    }
}

void ServerWindow::kick() {
    int row = m_userList->currentRow();
    if (row < 0 || !m_server) return;
    auto users = Database::instance().getAllUsers();
    if (row >= (int)users.size()) return;
    if (QMessageBox::question(this, "Kick", "Kick " + users[row].nickname + "?") == QMessageBox::Yes) {
        m_server->kickUser(users[row].id);
        refresh();
    }
}

void ServerWindow::ban() {
    int row = m_userList->currentRow();
    if (row < 0 || !m_server) return;
    auto users = Database::instance().getAllUsers();
    if (row >= (int)users.size()) return;
    QString reason = QInputDialog::getMultiLineText(this, "Ban", "Reason:");
    if (!reason.isEmpty()) {
        m_server->banUser(users[row].id, reason);
        refresh();
    }
}

void ServerWindow::unban() {
    int row = m_userList->currentRow();
    if (row < 0) return;
    auto users = Database::instance().getAllUsers();
    if (row >= (int)users.size() || !users[row].banned) return;
    if (QMessageBox::question(this, "Unban", "Unban " + users[row].nickname + "?") == QMessageBox::Yes) {
        Database::instance().unbanUser(users[row].id);
        refresh();
    }
}
