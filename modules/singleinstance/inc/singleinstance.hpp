#pragma once

#include <QObject>
#include <QString>
#include <QWidget>

class QLocalServer;

class SingleInstance : public QObject
{
    Q_OBJECT

public:
    SingleInstance(QWidget *parent, const QString &progName);

    bool isAlreadyRunning(bool raiseExisting);
    static void raiseWindow(QWidget *window);

private:
    QString serverName;
    QLocalServer *server = nullptr;
};
