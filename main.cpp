#include <QApplication>
#include "database.h"
#include "serverwindow.h"
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    if (!Database::instance().connect("localhost", 5432, "chatdb", "postgres", "password")) {
        qCritical() << "Cannot connect to database";
        return 1;
    }

    ServerWindow sw;
    sw.show();

    MainWindow* client = MainWindow::createClient();
    if (client) client->show();

    return app.exec();
}
