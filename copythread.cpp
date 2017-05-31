#include "copythread.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QApplication>

CopyThread::CopyThread(QObject *parent) : QObject(parent)
{
    setOverwrite(false);
    _abortOperation = false;
}

void CopyThread::doSetup(QThread *thread)
{
    connect(thread,SIGNAL(started()),this,SLOT(doWork()));
}

QString CopyThread::sourceElement() const
{
    return _sourceElement;
}

void CopyThread::setSourceElement(const QString &sourceElement)
{
    _sourceElement = sourceElement;
}

QString CopyThread::destinationPath() const
{
    return _destinationPath;
}

void CopyThread::setDestinationPath(const QString &destinationPath)
{
    _destinationPath = destinationPath;
}

bool CopyThread::overwrite() const
{
    return _overwrite;
}

void CopyThread::setOverwrite(bool overwrite)
{
    _overwrite = overwrite;
}

void CopyThread::copyFile(QString input, QString output, bool overwrite = false)
{
    if(shouldAbortOperation())
    {
        return;
    }

    QFile inputFile(input);
    QFile outputFile(output);

    if(!inputFile.exists())
    {
        qWarning() << "Error:" << "InputFile" << input << "does not exists!" << ". Process aborted";
        return;
    }

    if(outputFile.exists())
    {
        qDebug() << "NOTE:" << "outputFile" << output << "does already exist!";
        qDebug() << "Overwrite set to:" << overwrite;

        if(overwrite)
        {
            if(!outputFile.remove())
            {
                qWarning() << outputFile.fileName() << "could not be removed!";
            }
        }
    }

    if(inputFile.copy(output))
    {
        qDebug() << "Successfully copied" << output;
        emit copyingSuccess(output);
    }
    else
    {
        qDebug() << "Unsuccessfully copied" << output << "Error (" << inputFile.error() << ")" << inputFile.errorString();
        emit copyingError(output,QString::number(inputFile.error()),inputFile.errorString());
    }

    QApplication::processEvents();
}

bool CopyThread::shouldAbortOperation()
{
    qDebug() << "Should abourt: " << _abortOperation;
    return _abortOperation;
}




void CopyThread::doWork()
{
    if(shouldAbortOperation())
    {
        return;
    }

    QFileInfo sourceElementInfo(sourceElement());
    if(sourceElementInfo.isDir())
    {
        QDir srcDir = destinationPath() + "/" + sourceElementInfo.fileName();
        if(!srcDir.exists())
        {
            srcDir.mkdir(srcDir.absolutePath());
        }
        copyFolder(sourceElement(),srcDir.absolutePath(),overwrite());
    }
    else if(sourceElementInfo.isFile())
    {
        QString outputFile = QString("%1/%2").arg(destinationPath()).arg(QFileInfo(sourceElement()).fileName());
        copyFile(sourceElement(),outputFile,overwrite());
    }
    else
    {

    }

    emit copyingCompleted();

}

void CopyThread::abortOperation()
{
    qDebug() << "AbortOperation set to true";
    _abortOperation = true;
}
