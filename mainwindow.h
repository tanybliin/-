#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(int userId, const QString& nickname, QWidget* parent = nullptr);
    static MainWindow* createClient();
    ~MainWindow();

private slots:
    void sendMessage();
    void onReadyRead();
    void onConnected();

private:
    void setupUI();
    void sendRaw(const QString& cmd);

    QTcpSocket* m_socket;
    int m_userId;
    QString m_nickname;
    QTextEdit* m_chatView;
    QLineEdit* m_input;
    QListWidget* m_userList;
};

#endif
