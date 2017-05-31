#ifndef FORMATDRIVESDIALOG_H
#define FORMATDRIVESDIALOG_H

#include <QDialog>
#include <QProcess>

namespace Ui {
class FormatDrivesDialog;
}

class FormatDrivesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FormatDrivesDialog(QWidget *parent = 0);
    ~FormatDrivesDialog();

private slots:
    void on_showAllDrives_clicked();
    void on_refreshListButton_clicked();
    void on_formatButton_clicked();
    void onReadyReadStandardOutput();
    void onProgressFinished(int,QProcess::ExitStatus);

    void on_showAllDrives_clicked(bool checked);

    void on_closeButton_clicked();

    void on_drivesList_itemSelectionChanged();

private:
    Ui::FormatDrivesDialog *ui;

    void updateDrivesList();
    bool formatDrive(QString driveLetter, QString newLabel,bool autoYes);
    void doFormating();

    QProcess *shell;
};

#endif // FORMATDRIVESDIALOG_H
