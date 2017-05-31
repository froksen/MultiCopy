#include "copyprocess.h"
#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QDirIterator>

CopyProcess::CopyProcess(QObject *parent) : QObject(parent)
{
    _cancelProcess = false;
}

QStringList CopyProcess::inputElements() const
{
    return _inputElements;
}

void CopyProcess::setInputElements(const QStringList &inputElements)
{
    qDebug() << "inputElements " << inputElements;
    _inputElements = inputElements;
}

QString CopyProcess::outputDirectory() const
{
    return _outputDirectory;
}

void CopyProcess::setOutputDirectory(const QString &outputDirectory)
{
    _outputDirectory = outputDirectory;
}

void CopyProcess::start()
{
    qDebug() << "Started" << outputDirectory();
    startCopying();
}

bool CopyProcess::overwriteExistingFile() const
{
    return _overwriteExistingFile;
}

void CopyProcess::setOverwriteExistingFile(bool overwriteExistingFile)
{
    _overwriteExistingFile = overwriteExistingFile;
}

void CopyProcess::onBytesWritten(qint64 bytes)
{
    qDebug() << "ONBYTESWRITTEN";
    qDebug() << "BYTES WRITTEN" << bytes;
}

void CopyProcess::cancelProcess()
{
    _cancelProcess = true;
}

bool CopyProcess::validate() const
{
    return _validate;
}

void CopyProcess::setValidate(bool validate)
{
    _validate = validate;
}

void CopyProcess::startCopying()
{
    copyElements(inputElements(),outputDirectory());
    processCompleted(outputDirectory());
}

void CopyProcess::copyElements(QStringList inputElements, QString outputPath)
{
    foreach(QString inputElement, inputElements)
    {
        //Cancel process
        if(_cancelProcess)
        {
           emit processCanceled(outputPath);
           return;
        }

        qDebug() << "Working on inputElement" << inputElement;
        emit fileCopyStarted(inputElement,outputPath);
        QFileInfo inputElementInfo(inputElement) ;
        if(inputElementInfo.isDir())
        {
            qDebug() << inputElement << "is directory";
            copyFolder(inputElement,outputPath,overwriteExistingFile());
        }
        else if(inputElementInfo.isFile())
        {
            qDebug() << inputElement << "is file";
            QString outputFile = outputPath + "/" + inputElementInfo.fileName();
            bool cpResult = copyFile(inputElement,outputFile,overwriteExistingFile());
            if(cpResult)
            {
                qDebug() << "Successfully copied" <<inputElementInfo.fileName() << "to" << outputPath;
                emit fileSuccessfullyCopied(inputElement,outputPath);

                //IF VALIDATE IS ENABLED
                if(validate())
                {
                    //Calulate Checksums
                    QByteArray inputFileChecksum = fileChecksum(inputElement, QCryptographicHash::Md5);
                    QByteArray outputFileChecksum = fileChecksum(outputFile,QCryptographicHash::Md5);

                    if(inputFileChecksum == outputFileChecksum)
                    {
                        qDebug() << "Successfully validated:" << inputElement;
                        qDebug() << "CHECKSSUMS";
                        qDebug() << " - INPUTFILE" << inputElement << inputFileChecksum;
                        qDebug() << " - OUTPUTFILE" << outputFile << outputFileChecksum;

                        emit fileSuccessfullyValidated(outputFile,inputFileChecksum);
                    }
                    else
                    {
                        qDebug() << "Unsuccessfully validated:" << inputElement;
                        emit fileUnsuccessfullyValidated(inputElement,outputFile,inputFileChecksum, outputFileChecksum);
                        qDebug() << "CHECKSSUMS";
                        qDebug() << " - INPUTFILE" << inputElement << inputFileChecksum.toHex();
                        qDebug() << " - OUTPUTFILE" << outputFile << outputFileChecksum.toHex();
                    }
                }


            }
            else
            {
                qDebug() << "Unsuccessfully copied" <<inputElementInfo.fileName() << "to" << outputPath << "ERROR: " << lastError;
                emit fileUnsuccessfullyCopied(inputElement,outputPath,lastError);
            }
        }
    }
}

bool CopyProcess::copyFile(QString input, QString output, bool overwrite)
{
  QFile inputFile(input);
  QFile outputFile(output);

  connect(&outputFile,SIGNAL(bytesWritten(qint64)),this,SLOT(onBytesWritten(qint64)));

  //Makes sure that Inputfile exists
  if(!inputFile.exists())
  {
      qDebug() << "Inputfile" << input << "not found!";
      return false;
  }

  //Checks if outputfile exists
  if(outputFile.exists())
  {
      if(overwrite)
      {
          if(!outputFile.remove())
          {
              qDebug() << "Unable to remove " << output << "therefore overwrite is impossible";
              return false;
          }
      }
  }

  //Copies the file
  bool cpRlt = inputFile.copy(output);
  lastError = inputFile.errorString();
  return cpRlt;
}

void CopyProcess::copyFolder(QString input, QString output, bool overwrite)
{
    QDir outputDir(output);
    QDir inputDir(input);

    qDebug() << "Trying to Copy Folder" << input << "to" << output;

    //Checks if outputfolder Exists
    if(!outputDir.exists())
    {
        qDebug() << "Output dir" << output << "does not exists. Process canceled";
        return;
    }

   QString newOutput = output + "/" + inputDir.dirName();
   QDir newDir(newOutput);
   if(!newDir.exists())
   {
       newDir.mkdir(newDir.absolutePath());
       qDebug() << "Created new direcory" << newDir.absolutePath() << "in" << output;
   }

   emit folderSuccessfullyCreated(input, output);

    QStringList allEntries = inputDir.entryList(QDir::NoSymLinks | QDir::AllEntries | QDir::NoDotAndDotDot);

    //For getting absolutFilePath
    QStringList tmpList;
    foreach(QString entry, allEntries)
    {
        QString absFilePath = input + "/" + entry;
        tmpList.append(absFilePath);
    }

    allEntries = tmpList;

    copyElements(allEntries,newOutput);
}

QByteArray CopyProcess::fileChecksum(const QString &fileName,
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

