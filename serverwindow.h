#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

class ChatServer;

class ServerWindow : public QWidget {
    Q_OBJECT
public:
    explicit ServerWindow(QWidget* parent = nullptr);
    ~ServerWindow();

private slots:
    void startStop();
    void refresh();
    void kick();
    void ban();
    void unban();

private:
    void loadUsers();
    void loadMessages();
    void setupUI();

    ChatServer* m_server;
    QListWidget* m_userList;
    QTextEdit* m_msgView;
    QPushButton* m_startBtn;
    QSpinBox* m_portEdit;
    QTimer* m_timer;
    bool m_running;
};

#endif
