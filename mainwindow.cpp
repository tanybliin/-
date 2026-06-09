#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QMessageBox>

MainWindow::MainWindow(int userId, const QString& nickname, QWidget* parent)
    : QMainWindow(parent), m_userId(userId), m_nickname(nickname) {
    setupUI();
    setWindowTitle(nickname + " - Chat");

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &MainWindow::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    m_socket->connectToHost("localhost", 5555);
}

MainWindow::~MainWindow() { if (m_socket) m_socket->disconnectFromHost(); }

MainWindow* MainWindow::createClient() {
    bool ok;
    QString login = QInputDialog::getText(nullptr, "Login", "Login:", QLineEdit::Normal, "", &ok);
    if (!ok || login.isEmpty()) return nullptr;
    QString pass = QInputDialog::getText(nullptr, "Password", "Password:", QLineEdit::Password, "", &ok);
    if (!ok || pass.isEmpty()) return nullptr;

    QTcpSocket socket;
    socket.connectToHost("localhost", 5555);
    if (!socket.waitForConnected(3000)) return nullptr;

    socket.write(QString("AUTH " + login + " " + pass + "\n").toUtf8());
    if (!socket.waitForReadyRead(3000)) return nullptr;

    QByteArray resp = socket.readLine().trimmed();
    if (resp == "AUTH_OK") {
        return new MainWindow(1, login);
    }
    QMessageBox::warning(nullptr, "Error", "Auth failed");
    return nullptr;
}

void MainWindow::setupUI() {
    auto* central = new QWidget(this);
    auto* main = new QHBoxLayout(central);
    main->setContentsMargins(5, 5, 5, 5);

    auto* left = new QVBoxLayout();
    left->addWidget(new QLabel("Users"));
    m_userList = new QListWidget();
    left->addWidget(m_userList);
    main->addLayout(left, 1);

    auto* right = new QVBoxLayout();
    m_chatView = new QTextEdit();
    m_chatView->setReadOnly(true);
    right->addWidget(m_chatView);

    auto* bottom = new QHBoxLayout();
    m_input = new QLineEdit();
    auto* sendBtn = new QPushButton("Send");
    bottom->addWidget(m_input);
    bottom->addWidget(sendBtn);
    right->addLayout(bottom);
    main->addLayout(right, 3);

    setCentralWidget(central);
    setStyleSheet(
        "QMainWindow { background-color: #1e1e2e; }"
        "QTextEdit, QListWidget { background-color: #313244; color: #cdd6f4; border: 1px solid #45475a; }"
        "QLineEdit { background-color: #45475a; color: #cdd6f4; border: none; padding: 6px; border-radius: 4px; }"
        "QPushButton { background-color: #89b4fa; color: #1e1e2e; border: none; padding: 6px; border-radius: 4px; }"
        "QLabel { color: #a6adc8; }"
    );
    resize(800, 600);

    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(m_input, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
}

void MainWindow::onConnected() {
    sendRaw("AUTH " + QString::number(m_userId));
}

void MainWindow::onReadyRead() {
    while (m_socket->canReadLine()) {
        QString line = QString::fromUtf8(m_socket->readLine()).trimmed();
        if (line == "AUTH_OK") {
            m_chatView->append("Connected to server");
        } else if (line == "KICKED") {
            m_chatView->append("You were kicked");
            m_socket->disconnectFromHost();
        } else {
            m_chatView->append(line);
        }
    }
}

void MainWindow::sendMessage() {
    if (m_input->text().isEmpty()) return;
    sendRaw("MSG " + m_input->text());
    m_chatView->append("Me: " + m_input->text());
    m_input->clear();
}

void MainWindow::sendRaw(const QString& cmd) {
    m_socket->write(cmd.toUtf8() + "\n");
}
