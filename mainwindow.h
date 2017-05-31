#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <copyingdialog.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_beginCopyingButton_released();
    void on_refreshListButton_clicked();
    void on_drivesList_clicked(const QModelIndex &index);
    void on_formatDrive_clicked();
    void on_formatDrive_toggled(bool checked);

    void on_sourceTreeview_clicked(const QModelIndex &index);

    void on_beginCopyingButton_clicked();

    void on_actionFormat_drives_triggered();

    void on_previousTabButton_clicked();

    void on_nextTabButton_clicked();

    void on_refreshListButton_2_clicked();

    void on_actionAbout_triggered();

private:
    Ui::MainWindow *ui;

    void refreshTreeViews();
    void refreshSourceTreeview();
    void refreshDestinationTreeview();
    bool hasSelections();
    void updateUserEnabled();


    void updateTabNavigationButtons();
    void summeryGenerator();

protected:
    void resizeEvent(QResizeEvent* event);

};

#endif // MAINWINDOW_H
