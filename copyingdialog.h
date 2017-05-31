#ifndef COPYINGDIALOG_H
#define COPYINGDIALOG_H

#include <QDialog>
#include <QProgressDialog>
#include "copythread.h"
#include <QAbstractButton>
#include <QMessageBox>
#include <QCloseEvent>

namespace Ui {
class CopyingDialog;
}

class CopyingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CopyingDialog(QWidget *parent = 0);
    ~CopyingDialog();

    QStringList outputDirectories() const;
    void setOutputDirectories(const QStringList &outputDirectories);

    void beginCopying();

    bool overwritefiles() const;
    void setOverwritefiles(bool overwritefiles);

    bool validateData() const;
    void setValidateData(bool validateData);

    QStringList failedFilesSummary() const;
    void appendFailedFilesSummary(QString summary);
    void setFailedFilesSummary(const QStringList &failedFilesSummary);

    bool formatDrives() const;
    void setFormatDrives(bool formatDrives);

    QString formatDiskLabel() const;
    void setFormatDiskLabel(const QString &formatDiskLabel);

    QStringList sourceElements() const;
    void setSourceElements(const QStringList &sourceElements);

signals:
    void countingFiles(int filesCount);

private slots:
    void onFileValidated(bool, QString);
    void onFileSuccessfulliValidated(QString filename, QByteArray checksum);
    void onFileUnsuccessfullyValidated(QString inFilename, QString outFilename, QByteArray InChecksum, QByteArray OutChecksum);
    void copyingCompleted();
    void on_buttonBox_rejected();

    void onFilesCountChanged(int filesCount);
    void onCopyingSuccessfull(QString item,QString output);
    void onDirCreated(QString dirName, QString output);
    void onFileCopied();
    void onFileCopyStarted(QString item, QString output);
    void onCopyingError(QString item, QString error, QString errorText);
    void onThreadFinished();
    void onProcessCancelled(QString desc);

    void on_abortAllOperations_clicked();

    void on_buttonBox_clicked(QAbstractButton *button);

    void on_abortAllOperationsButton_clicked();
    void on_processCompleted(QString processDesc);

private:
    Ui::CopyingDialog *ui;
    bool formatDrive(QString driveLetter,QString newLabel = "DISK",bool autoYes = false);

    int countFilesInSources(QStringList sourceElements);
    void copyFolder2(QString sourceFolder, QString destFolder);
    int countEntries(QString srcFolder);
    void copyFolder(QString srcFolder, QString destFolder);

    QMessageBox filecountmsg;

    QStringList _outputDirectories;
    QStringList _sourceElements;
    bool _overwritefiles;
    bool _validateData;
    bool _formatDrives;
    QString _formatDiskLabel;
    QStringList _failedFilesSummary;

    QList<CopyThread*> CopyThreads;
    QList<CopyThread*> copyThreads;

    bool abortOpertions;

protected:
    void closeEvent(QCloseEvent *event);
    void done(int r);


};

#endif // COPYINGDIALOG_H
