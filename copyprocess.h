#ifndef COPYPROCESS_H
#define COPYPROCESS_H

#include <QObject>
#include <QByteArray>
#include <QCryptographicHash>

class CopyProcess : public QObject
{
    Q_OBJECT
public:
    explicit CopyProcess(QObject *parent = 0);


    QStringList inputElements() const;
    void setInputElements(const QStringList &inputElements);

    QString outputDirectory() const;
    void setOutputDirectory(const QString &outputDirectory);

    void start();

    bool overwriteExistingFile() const;
    void setOverwriteExistingFile(bool overwriteExistingFile);

    bool validate() const;
    void setValidate(bool validate);

signals:
    void fileSuccessfullyCopied(QString filename,QString output);
    void fileUnsuccessfullyCopied(QString filename, QString output, QString errorText);
    void fileCopyStarted(QString filename,QString output);
    void processCanceled(QString processDescription);
    void processCompleted(QString processDescription);
    void folderSuccessfullyCreated(QString dirName, QString output);
    void fileSuccessfullyValidated(QString filename,QByteArray checksum);
    void fileUnsuccessfullyValidated(QString inFilename, QString outFilename, QByteArray InChecksum, QByteArray OutChecksum);


public slots:
    void onBytesWritten(qint64 bytes);
    void cancelProcess();

private:
    QString _outputDirectory;
    QStringList _inputElements;
    bool _overwriteExistingFile;
    bool _cancelProcess;
    bool _validate;

    void startCopying();
    void copyElements(QStringList inputElements, QString outputPath);
    bool copyFile(QString input, QString output,bool overwrite);
    void copyFolder(QString input, QString output,bool overwrite);
    QString lastError;
    QString lastErrorString;


    QByteArray fileChecksum(const QString &fileName,
                            QCryptographicHash::Algorithm hashAlgorithm);
};

#endif // COPYPROCESS_H
