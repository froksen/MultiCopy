#ifndef COPYTHREAD_H
#define COPYTHREAD_H

#include <QObject>
#include <QDebug>
#include <QThread>

class CopyThread : public QObject
{
    Q_OBJECT
public:
    explicit CopyThread(QObject *parent = 0);

    void doSetup(QThread *thread);

    QString sourceElement() const;
    void setSourceElement(const QString &sourceElement);

    QString destinationPath() const;
    void setDestinationPath(const QString &destinationPath);

    bool overwrite() const;
    void setOverwrite(bool overwrite);

private:
    QString _sourceElement;
    QString _destinationPath;
    bool _overwrite;
    bool _abortOperation;

    void copyFolder(QString srcFolder, QString destFolder,bool overwriteExistingFiles);
    void copyFile(QString input, QString output, bool overwrite);
    bool shouldAbortOperation();

signals:
    void copyingError(QString item,QString error, QString errorText);
    void copyingSuccess(QString item);
    void copyingCompleted();

public slots:
    void doWork();
    void abortOperation();

};

#endif // COPYTHREAD_H
