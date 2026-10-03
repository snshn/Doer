#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSettings>

#include "mainwindow.hpp"
#include "ui_mainwindow.h"

// Write state to disk at most once per this interval while editing
static const int SAVE_INTERVAL_MS = 1000;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    singleInstance = new SingleInstance((QWidget*)this, QStringLiteral(PROG_NAME));

    saveTimer.setSingleShot(true);
    saveTimer.setInterval(SAVE_INTERVAL_MS);
    connect(&saveTimer, &QTimer::timeout, this, &MainWindow::saveState);

    loadSettings();

    setMinimumSize(240, 360);
    setWindowIcon(QIcon(":/images/" PROG_NAME ".svg"));

    applyStyle();
    ui->textArea->setFrameStyle(QFrame::NoFrame);

    bindShortcuts();

    ready = true;
}

MainWindow::~MainWindow(void)
{
    delete ui;
}

void MainWindow::applyStyle(void)
{
    QString styleSheet;

    QFile styleFile(":/stylesheets/" PROG_NAME ".qss");
    if (styleFile.open(QFile::ReadOnly)) {
        styleSheet = QLatin1String(styleFile.readAll());
    }

    QFileInfo settingsFileInfo(settings->fileName());
    QFile customStyleFile(settingsFileInfo.absolutePath() + QDir::separator() + PROG_NAME ".qss");
    if (customStyleFile.open(QFile::ReadOnly)) {
        styleSheet += QLatin1String(customStyleFile.readAll());
    }

    ui->textArea->setStyleSheet(styleSheet);
}

void MainWindow::loadSettings(void)
{
    // Parented to this window, so it is cleaned up automatically
    settings = new QSettings(QSettings::IniFormat,
                             QSettings::UserScope,
                             PROG_NAME,
                             PROG_NAME,
                             this);

    if (settings->contains("text")) {
        ui->textArea->setPlainText(settings->value("text").toString());
    }

    if (settings->contains("cursor")) {
        QTextCursor newCursor = ui->textArea->textCursor();
        newCursor.setPosition(settings->value("cursor").toInt());

        if (settings->contains("cursor_end")) {
            newCursor.setPosition(settings->value("cursor_end").toInt(),
                                  QTextCursor::KeepAnchor);
        }

        ui->textArea->setTextCursor(newCursor);
        ui->textArea->ensureCursorVisible();
    }

    if (settings->contains("geometry")) {
        restoreGeometry(QByteArray::fromHex(settings->value("geometry").toByteArray()));
    }
}

void MainWindow::bindShortcuts(void)
{
    struct { QKeySequence key; const char *slot; } bindings[] = {
        { QKeySequence("Ctrl+F"),          SLOT(toggleFullScreen()) },
        { QKeySequence(Qt::Key_F11),       SLOT(toggleFullScreen()) },
        { QKeySequence(Qt::Key_Escape),    SLOT(exitFullScreen())   },
        { QKeySequence("Ctrl+Q"),          SLOT(quitApplication())  },
    };

    for (const auto &b : bindings) {
        QAction *action = new QAction(this);
        action->setShortcut(b.key);
        addAction(action);
        connect(action, SIGNAL(triggered()), this, b.slot);
    }
}

void MainWindow::exitFullScreen(void)
{
    setWindowState(windowState() & ~Qt::WindowFullScreen);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    quitApplication();

    QMainWindow::closeEvent(event);
}

// Both handlers only mark state as changed and arm the timer: no disk I/O here.
// The timer is not restarted if already running, so continuous typing still
// gets saved every SAVE_INTERVAL_MS (throttle, not debounce).

void MainWindow::on_textArea_cursorPositionChanged(void)
{
    if (ready && !saveTimer.isActive()) {
        saveTimer.start();
    }
}

void MainWindow::on_textArea_textChanged(void)
{
    if (ready) {
        textDirty = true;
        if (!saveTimer.isActive()) {
            saveTimer.start();
        }
    }
}

void MainWindow::saveState(void)
{
    if (textDirty) {
        settings->setValue("text", ui->textArea->toPlainText());
        textDirty = false;
    }

    const QTextCursor cursor = ui->textArea->textCursor();
    settings->setValue("cursor", cursor.selectionStart());
    settings->setValue("cursor_end", cursor.selectionEnd());
    settings->sync();
}

void MainWindow::quitApplication(void)
{
    saveTimer.stop();
    saveState();

    settings->setValue("geometry", QString(saveGeometry().toHex()));
    settings->sync();

    QApplication::quit();
}

void MainWindow::toggleFullScreen(void)
{
    setWindowState(windowState() ^ Qt::WindowFullScreen);
}

bool MainWindow::isAlreadyRunning(void)
{
    return singleInstance->isAlreadyRunning(true);
}
