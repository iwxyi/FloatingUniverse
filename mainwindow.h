#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include "defines.h"
#if defined(ENABLE_SHORTCUT)
#include "qxtglobalshortcut.h"
#endif
#include "watercirclebutton.h"
#include "panel/universepanel.h"
#include "panelsettingswidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QListWidgetItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void trayAction(QSystemTrayIcon::ActivationReason reason);

    void on_searchEdit_textEdited(const QString &arg1);

    void on_pushButton_clicked();

private:
    void initView();
    void initTray();
    void initKey();
    void initPanel();

    QRect screenGeometry() const;

public slots:

protected:
    void showEvent(QShowEvent* e) override;
    void closeEvent(QCloseEvent* e) override;
    void resizeEvent(QResizeEvent* e) override;

private:
    void returnToPrevWindow();

private:
    Ui::MainWindow *ui;
#if defined(ENABLE_SHORTCUT)
    QxtGlobalShortcut* editShortcut;
#endif
#ifdef Q_OS_WIN32
    HWND prevWindow = nullptr;
#endif
    UniversePanel* panel = nullptr;
    WaterCircleButton *confirmButton = nullptr;
};
#endif // MAINWINDOW_H
