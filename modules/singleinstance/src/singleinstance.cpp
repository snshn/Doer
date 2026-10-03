#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>

#include "singleinstance.hpp"

// X11/Windows headers define macros (None, Bool, ...) that clash with Qt,
// so they must come after all Qt includes.
#if defined(Q_OS_WIN)
#include <windows.h>
#elif !defined(Q_OS_MACOS)
#include <X11/Xlib.h>
#endif

static const int CONNECT_TIMEOUT_MS = 500;
static const QByteArray RAISE_MSG = "raise";

SingleInstance::SingleInstance(QWidget *parent, const QString &progName)
    : QObject(parent)
{
    // Per-user name, so different users on one machine don't collide
    serverName = progName + "-" + QDir::home().dirName();
}

bool SingleInstance::isAlreadyRunning(const bool raiseExisting)
{
    // Is another instance listening?
    {
        QLocalSocket socket;
        socket.connectToServer(serverName);

        if (socket.waitForConnected(CONNECT_TIMEOUT_MS)) {
            if (raiseExisting) {
                socket.write(RAISE_MSG);
                socket.waitForBytesWritten(CONNECT_TIMEOUT_MS);
            }
            socket.disconnectFromServer();
            return true;
        }
    }

    // We're the first instance. Clear a stale socket left behind by a crash,
    // then listen. Nothing runs until a second instance actually connects.
    QLocalServer::removeServer(serverName);

    server = new QLocalServer(this);
    connect(server, &QLocalServer::newConnection, this, [this]() {
        while (QLocalSocket *client = server->nextPendingConnection()) {
            connect(client, &QLocalSocket::readyRead, this, [this, client]() {
                if (client->readAll().contains(RAISE_MSG)) {
                    raiseWindow(qobject_cast<QWidget*>(parent()));
                }
            });
            connect(client, &QLocalSocket::disconnected,
                    client, &QObject::deleteLater);
        }
    });
    server->listen(serverName);

    return false;
}

void SingleInstance::raiseWindow(QWidget *window)
{
    if (!window) {
        return;
    }

#if defined(Q_OS_WIN) // Windows
    HWND winId = reinterpret_cast<HWND>(window->effectiveWinId());

    SetWindowPos(winId, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    SetWindowPos(winId, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
#elif defined(Q_OS_MACOS) // macOS
    window->show();
    window->raise();
    window->activateWindow();
#else // GNU/Linux, FreeBSD, etc
    Window winId = window->effectiveWinId();

    if (winId > 0) {
        Display *disp = XOpenDisplay(nullptr);

        if (disp) {
            XWindowAttributes attributes;

            if (XGetWindowAttributes(disp, winId, &attributes)) {
                XSetInputFocus(disp, winId, RevertToPointerRoot, CurrentTime);
                XRaiseWindow(disp, winId);

                // Show window if minimized, instead of just highlighting its icon
                const Atom atom = XInternAtom(disp, "_NET_ACTIVE_WINDOW", True);

                if (atom != None) {
                    XEvent xev = {};

                    xev.xclient.type = ClientMessage;
                    xev.xclient.send_event = True;
                    xev.xclient.message_type = atom;
                    xev.xclient.display = disp;
                    xev.xclient.window = winId;
                    xev.xclient.format = 32;
                    xev.xclient.data.l[0] = 1;

                    XSendEvent(disp, DefaultRootWindow(disp), False,
                               SubstructureRedirectMask | SubstructureNotifyMask, &xev);
                }
            }

            XFlush(disp);
            XCloseDisplay(disp);
        }
    }
#endif
}
