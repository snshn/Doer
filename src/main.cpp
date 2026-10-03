#include <csignal>

#include <QApplication>
#include <QDebug>

#include "mainwindow.hpp"

MainWindow *mainWindow;

#if defined(Q_OS_UNIX)
static void onSignalHandler(int signum)
{
    if (mainWindow) {
        mainWindow->close();
        delete mainWindow;
        mainWindow = nullptr;
    }

    qDebug() << "Terminated with signal" << signum;

    exit(128 + signum);
}
#endif

int main(int argc, char *argv[])
{
#if defined(Q_OS_UNIX)
    signal(SIGHUP,  onSignalHandler);
    signal(SIGINT,  onSignalHandler);
    signal(SIGQUIT, onSignalHandler);
    signal(SIGILL,  onSignalHandler);
    signal(SIGABRT, onSignalHandler);
    signal(SIGFPE,  onSignalHandler);
    signal(SIGBUS,  onSignalHandler);
    signal(SIGSEGV, onSignalHandler);
    signal(SIGSYS,  onSignalHandler);
    signal(SIGPIPE, onSignalHandler);
    signal(SIGALRM, onSignalHandler);
    signal(SIGTERM, onSignalHandler);
    signal(SIGXCPU, onSignalHandler);
    signal(SIGXFSZ, onSignalHandler);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    // Account for running on high-DPI displays
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);

    QApplication::setApplicationName(PROG_NAME);
    QApplication::setApplicationDisplayName("Doer");
    QApplication::setApplicationVersion(VERSION);
    QApplication::setOrganizationDomain("y2z.github.io");
    QApplication::setOrganizationName("Y2Z");

    mainWindow = new MainWindow;

    if (mainWindow->isAlreadyRunning()) {
        exit(EXIT_FAILURE);
    }

    mainWindow->show();

    return app.exec();
}
