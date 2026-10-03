#pragma once

#include <QAction>
#include <QMainWindow>
#include <QSettings>
#include <QTimer>

#include "singleinstance.hpp"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = Q_NULLPTR);
    ~MainWindow(void);

    bool isAlreadyRunning(void);

protected:
    void closeEvent(QCloseEvent *event) override;
    // void moveEvent(QMoveEvent *event) override;
    // void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_textArea_cursorPositionChanged(void);
    void on_textArea_textChanged(void);

    void saveState(void);
    void exitFullScreen(void);
    void quitApplication(void);
    void toggleFullScreen(void);

private:
    void applyStyle(void);
    void bindShortcuts(void);
    void loadSettings(void);

    Ui::MainWindow *ui;
    SingleInstance *singleInstance = Q_NULLPTR;
    QSettings *settings = Q_NULLPTR;
    QTimer saveTimer;
    // QByteArray windowGeometry;

    bool ready = false;
    bool textDirty = false;
};
