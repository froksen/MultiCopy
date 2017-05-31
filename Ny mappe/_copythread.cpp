#include "_copythread.h"
#include <QtCore>
#include <QDebug>
#include <QDir>

CopyThread::CopyThread()
{
    copiedFilesCount = 0;
    this->Stop = false;
}

void CopyThread::run()
{
    qDebug() << "Copying start";
//    connect(this,SIGNAL(nextFile()),this,SLOT(copyNextFile()));

//    currentElementIndex = 0;
//    copyNextFile();


    foreach(QString sourceElement, sourceElements())
    {
        QMutex mutex;
        mutex.lock();
            copyElement(sourceElement,destinationFolder(),overwritefiles());
        mutex.unlock();
        qDebug() << "Copying completed";
    }
    emit completed();

}

void CopyThread::copyFolder(QString srcFolder, QString destFolder,bool overwriteExistingFiles = false)
{
    //Checks if source exists
    QDir sourceDir(srcFolder);
    if(!sourceDir.exists())
    {
        qDebug() << "Source directory does not exist!";
        return;
    }

    QDir newDestDir(destFolder + "/" + sourceDir.dirName());
    if(!newDestDir.exists())
    {
        qDebug() << "Creating" << destFolder << "/" << sourceDir.dirName();
        sourceDir.mkdir(destFolder + "/" + sourceDir.dirName());
        destFolder = destFolder + "/" + sourceDir.dirName();
    }


    //Checks if destanation folder exists. If not then it will be created.
    QDir destDir(destFolder);
    if(!destDir.exists())
    {
        destDir.mkdir(destFolder);
        qDebug() << "Destanation directory does not exists. Creating it now";
    }

    //Finds all files in sourcedir
    QStringList files = sourceDir.entryList(QDir::Files);
    if(files.count()<=0)
    {
        qDebug() << "No files found in" << srcFolder;
    }
    else
    {
        qDebug() << "Files" << files.count() << "found in" << srcFolder;
    }

    //Loops, and copies the files
    foreach(QString file, files)
    {
        if(Stop)
        {
            qDebug() << "ABORTED";
            break;
            emit aborted();
        }

        QString src = QDir::toNativeSeparators(srcFolder + "/" + file);
        QString dest = QDir::toNativeSeparators(destFolder + "/" + file);

        emit statusText(tr("Copying to %1").arg(dest));


        //The validate process
        QByteArray srcChecksum;
        if(validateData())
        {
            //Checksum of source file.
            srcChecksum = fileChecksum(src,QCryptographicHash::Sha1);
        }

        //COPY
        if(copyFile(src,dest,overwriteExistingFiles))
        {
            qDebug() << "Validate data:" << validateData();
            if(validateData())
            {
                QByteArray destChecksum = fileChecksum(dest,QCryptographicHash::Sha1); //Checksum of the copied file.

                if(destChecksum == srcChecksum)
                {
                    emit onFileValidated(true,dest); //If valid
                }
                else
                {
                    emit onFileValidated(false,dest); // if not valid
                }
            }

        }

        //How many files that are copied, successful or failed.
        copiedFilesCount++;
        qDebug() << "CP Files Copied: " << copiedFilesCount;
        emit filesCopied(copiedFilesCount);
    }

    //Clears the files-list
    files.clear();

    //Finds alle subDirs
    QStringList subDirs = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    foreach(QString subDir, subDirs)
    {
        if(Stop)
        {
            break;
            emit aborted();
        }

        QString src = QDir::toNativeSeparators(srcFolder + "/" + subDir);
        QString dest = QDir::toNativeSeparators(destFolder + "/" + subDir);
        qDebug() << "CP: Entering folder" << subDir;
        copyFolder(src,dest);
    }
}

bool CopyThread::copyFile(QString srcFilename, QString destFilename, bool overwrite = false)
{
    QFile destFile(destFilename);
    destFile.open(QFile::WriteOnly);
    QFile srcFile(srcFilename);
    srcFile.open(QFile::ReadOnly);

    qDebug() << "File overwrite:" << overwrite;
    if(overwrite)
    {
        if(destFile.remove())
        {
            qDebug() << "File removed";
        }
        else
        {
            //return false;
            qDebug() << "File could not be removed";
        }

    }

    qDebug() << "File copy from" << srcFilename << "to" << destFilename;
    if(srcFile.copy(destFilename))
    {
        qDebug() << "File copied";
        emit onFileCopied(true,destFilename); //If file is copied successfully
        emit nextFile();
        srcFile.close();
        destFile.close();
        return true;
    }
    srcFile.close();
    destFile.close();
    emit onFileCopied(false,destFilename+ " " + srcFile.errorString()); //If file is copied UNsuccessfully
    emit nextFile();
    qDebug() << "File could not be copied";
    qDebug() << "ERROR: " << srcFile.error() << srcFile.errorString();
    return false;
}

void CopyThread::copyElement(QString srcElement, QString destElement, bool overwrite)
{
    QFileInfo elementInfo(srcElement);

    if(elementInfo.isDir())
    {
        copyFolder(srcElement,destElement,overwrite);
    }
    else if(elementInfo.isFile())
    {
        destElement = destElement + "/" + elementInfo.fileName();
        copyFile(srcElement,destElement,overwrite);
    }
}

QStringList CopyThread::sourceElements() const
{
    return _sourceElements;
}

void CopyThread::setSourceElements(const QStringList &sourceElements)
{
    qDebug() << "SourceElements set to" << sourceElements;

    _sourceElements = sourceElements;
}

bool CopyThread::validateData() const
{
    return _validateData;
}

void CopyThread::setValidateData(bool validateData)
{
    _validateData = validateData;
}

bool CopyThread::overwritefiles() const
{
    return _overwritefiles;
}

void CopyThread::setOverwritefiles(bool overwritefiles)
{
    _overwritefiles = overwritefiles;
}

void CopyThread::cancelProgress()
{
    this->Stop = true;
}

void CopyThread::copyNextFile()
{
    if(sourceElements().isEmpty())
    {
        //emit completed();
        qDebug() << "COPY THREAD" << "COMPLETED";
        return;
    }



//    //QMutex mutex;
//    QString sourceElement = sourceElements().at(0);
//    QStringList srcElements = sourceElements();
//    srcElements.removeFirst();
//    setSourceElements(srcElements);


//    qDebug() << "sourceElement" << sourceElement;
//    copyElement(sourceElement,destinationFolder(),overwritefiles());


    //mutex.unlock();

//    currentElementIndex = currentElementIndex + 1;

//    foreach(QString sourceElement, sourceElements())
//    {
//        QMutex mutex;
//        mutex.lock();
//            copyElement(sourceElement,destinationFolder(),overwritefiles());
//        mutex.unlock();
//        qDebug() << "Copying completed";
//    }
//    emit completed();
}

QString CopyThread::destinationFolder() const
{
    return _destinationFolder;
}

void CopyThread::setDestinationFolder(const QString destinationFolder)
{
    qDebug() << "Destination folder set to" << destinationFolder;
    _destinationFolder = destinationFolder;
}

//Method from: http://stackoverflow.com/questions/16383392/how-to-get-the-sha-1-md5-checksum-of-a-file-with-qt
QByteArray CopyThread::fileChecksum(const QString &fileName,
                        QCryptographicHash::Algorithm hashAlgorithm)
{
    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        QCryptographicHash hash(hashAlgorithm);
        if (hash.addData(&f)) {
            return hash.result();
        }
    }
    return QByteArray();
}
