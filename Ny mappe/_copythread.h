#ifndef COPYTHREAD_H
#define COPYTHREAD_H

#include <QObject>
#include <QThread>
#include <QtCore>
#include <QCryptographicHash>

class CopyThread : public QThread
{
    Q_OBJECT

public:
    explicit CopyThread();

    QString destinationFolder() const;
    void setDestinationFolder(const QString destinationFolder);
    bool overwritefiles() const;
    void setOverwritefiles(bool overwritefiles);
    bool validateData() const;
    void setValidateData(bool validateData);

    QStringList sourceElements() const;
    void setSourceElements(const QStringList &sourceElements);

protected:
    void run();

private:
    void copyFolder(QString srcFolder, QString destFolder,bool overwriteExistingFiles);
    bool copyFile(QString srcFilename, QString destFilename, bool overwrite);
    void copyElement(QString srcElement, QString destElement, bool overwrite);

    QStringList _sourceElements;
    QString _destinationFolder;

    int copiedFilesCount;
    bool Stop;
    bool _overwritefiles;
    bool _validateData;

    QByteArray fileChecksum(const QString &fileName,
                      QCryptographicHash::Algorithm hashAlgorithm);

    int currentElementIndex;

public slots:
    void cancelProgress();
    void copyNextFile();

signals:
    void fileCopied(bool,QString);
    void statusText(QString);
    void filesCopied(int);
    void nextFile();
    void onFileValidated(bool,QString);
    void onFileCopied(bool,QString);
    void completed();
    void aborted();
};

#endif // COPYTHREAD_H
