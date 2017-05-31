#include "copyingdialog.h"
#include "ui_copyingdialog.h"
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QMessageBox>
#include "copyprogress.h"
#include <QTimer>
#include <QColor>
#include <QProcess>
#include <QThread>
#include <QAbstractButton>
#include <QPushButton>
#include "copyprocess.h"

CopyingDialog::CopyingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CopyingDialog)
{
    ui->setupUi(this);
    //QCoreApplication::processEvents();
    setOverwritefiles(false); //Should files be overwritten?
    setValidateData(true); //Validate data
    //QCoreApplication::processEvents();

    abortOpertions = false;


    setWindowFlags(Qt::FramelessWindowHint);
    setWindowFlags(Qt::WindowTitleHint);

   //Makes sure, that the Complete log is shown
    ui->toolBox->setCurrentIndex(0);

}

CopyingDialog::~CopyingDialog()
{
    delete ui;
}

QStringList CopyingDialog::outputDirectories() const
{
    return _outputDirectories;
}

void CopyingDialog::setOutputDirectories(const QStringList &outputDirectories)
{
    _outputDirectories = outputDirectories;
    ui->progressBar->setMaximum(_outputDirectories.count());

}

void copyFolder2(QString sourceFolder, QString destFolder)
{
    QDir sourceDir(sourceFolder);
    if(!sourceDir.exists())
        return;
    QDir destDir(destFolder);
    if(!destDir.exists())
    {
        destDir.mkdir(destFolder);
    }
    QStringList files = sourceDir.entryList(QDir::Files);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        QFile::copy(srcName, destName);
    }
    files.clear();
    files = sourceDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for(int i = 0; i< files.count(); i++)
    {
        QString srcName = sourceFolder + "/" + files[i];
        QString destName = destFolder + "/" + files[i];
        //copyFolder(srcName, destName);
    }
}

bool CopyingDialog::formatDrive(QString driveLetter, QString newLabel,bool autoYes)
{
//    if(newLabel.size()>10)
//    {
//        qDebug() << "New label is too long, must only be 11 characters";
//    }

//    //Checks if drive Exists
//    QDir drive(driveLetter);
//    if(!drive.exists())
//    {
//        qDebug() << "Drive" << driveLetter << "does not exists!";
//        return false;
//    }

//    //Removes slashes from drive letter, if any so D:/ => D:
//    driveLetter = driveLetter.remove("\\").remove("/");

//    QProcess shell;
//    QStringList arguments;
//    arguments << "/K" << "FORMAT" << driveLetter << "/FS:FAT32" << "/Q" << "/X" << "/V:nFDisk";
//    if(autoYes) arguments.append("/y");
//    arguments << "&&" << "LABEL" << driveLetter << newLabel;
//    arguments << "&&" << "exit";
//    if(shell.execute("cmd.exe",arguments) == 0)
//    {
//        qDebug() << "is Completed";
//        return true;
//    }
//    return false;

    return true;
}

int CopyingDialog::countFilesInSources(QStringList sourceElements)
{
    int filesCount = 0;
    foreach(QString source, sourceElements)
    {
        if(QFileInfo(source).isDir())
        {
            filesCount = filesCount + countEntries(source);
        }
        else if(QFileInfo(source).isFile())
        {
            filesCount++;
        }
    }

    return filesCount;
}


int CopyingDialog::countEntries(QString srcFolder)
{
    int filesCount = 0;
    QDirIterator it(srcFolder, QDir::AllDirs | QDir::NoDotDot,  QDirIterator::Subdirectories);
    while(it.hasNext())
    {
        if(abortOpertions)
        {
            return -1;
        }

        QString folder = it.next();

        filesCount = filesCount+ QDir(folder).entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Files).count();
        //filesCount = filesCount+ QDir(folder).entryList(QDir::NoDotAndDotDot | QDir::NoSymLinks | QDir::Dirs).count();
        qDebug() << folder << filesCount;

        emit onFilesCountChanged(filesCount);
        QApplication::processEvents();
    }

    return filesCount;
}

void CopyingDialog::closeEvent(QCloseEvent *event)
{


    //event->ignore();
}

void CopyingDialog::done(int r)
{
    //All processes are finished
    if(ui->progressBar->value() == ui->progressBar->maximum())
    {
        QDialog::done(QDialog::Accepted);
        return;
    }


    int msgbox = QMessageBox::question(this,tr("Confirm"),tr("If you close this window, all operations will be cancelled. Do you want to still close?"),QMessageBox::Yes|QMessageBox::No);
    if(msgbox==QMessageBox::Yes)
    {
        ui->abortAllOperationsButton->clicked(true);
        ui->abortAllOperationsButton->click();
        QDialog::done(QDialog::Accepted);
        return;
    }

    //If no is pressed
    return;
}

QStringList CopyingDialog::sourceElements() const
{
    return _sourceElements;
}

void CopyingDialog::setSourceElements(const QStringList &sourceElements)
{
    _sourceElements = sourceElements;
}

QString CopyingDialog::formatDiskLabel() const
{
    return _formatDiskLabel;
}

void CopyingDialog::setFormatDiskLabel(const QString &formatDiskLabel)
{
    _formatDiskLabel = formatDiskLabel;
}

bool CopyingDialog::formatDrives() const
{
    return _formatDrives;
}

void CopyingDialog::setFormatDrives(bool formatDrives)
{
    _formatDrives = formatDrives;
}

QStringList CopyingDialog::failedFilesSummary() const
{
    return _failedFilesSummary;
}

void CopyingDialog::appendFailedFilesSummary(QString summary)
{
    _failedFilesSummary.append(summary);
}

void CopyingDialog::setFailedFilesSummary(const QStringList &failedFilesSummary)
{
    _failedFilesSummary = failedFilesSummary;
}

bool CopyingDialog::validateData() const
{
    return _validateData;
}

void CopyingDialog::setValidateData(bool validateData)
{
    _validateData = validateData;
}

bool CopyingDialog::overwritefiles() const
{
    return _overwritefiles;
}

void CopyingDialog::setOverwritefiles(bool overwritefiles)
{
    _overwritefiles = overwritefiles;
}

void CopyingDialog::beginCopying()
{
    qDebug() << "Copy process started";

    int filesCount = 0;
    foreach (QString srcElement, sourceElements()) {
        if(QFileInfo(srcElement).isDir())
        {
            filesCount = filesCount + countEntries(srcElement);
        }
        else if(QFileInfo(srcElement).isFile())//Is File
        {
            //filesCount = filesCount +1;
        }

        if(abortOpertions || filesCount == -1)
        {
            onProcessCancelled("ABORTED!");
            return;
        }

        onFilesCountChanged(filesCount);
    }

    qDebug() << "filesCount" << filesCount;


    //Loops through each of the outputdirs
    foreach (QString outputdir, outputDirectories()) {
        //Creates the progressbar for each process
        CopyProgress *mProgress = new CopyProgress(this);
        mProgress->setHeaderLabel(QDir(outputdir).dirName());
        mProgress->setProgressMaximumValue(filesCount);

       ui->copyProgressLayout->addWidget(mProgress);

        //Creates a new instance of a copyprocess for each outputdir
        CopyProcess *mCopyProcess = new CopyProcess();
        mCopyProcess->setOutputDirectory(outputdir); //Outputdir
        mCopyProcess->setInputElements(sourceElements()); //Inputdir
        mCopyProcess->setOverwriteExistingFile(overwritefiles()); //Overwrite
        mCopyProcess->setValidate(validateData()); //Validate

        //Connections for mCopyProcess
        connect(mCopyProcess,SIGNAL(fileSuccessfullyCopied(QString,QString)),this,SLOT(onCopyingSuccessfull(QString,QString)));
        connect(mCopyProcess,SIGNAL(fileUnsuccessfullyCopied(QString,QString,QString)),this,SLOT(onCopyingError(QString,QString,QString)));
        connect(mCopyProcess,SIGNAL(fileCopyStarted(QString,QString)),this,SLOT(onFileCopyStarted(QString,QString)));
        connect(mCopyProcess,SIGNAL(folderSuccessfullyCreated(QString,QString)),this,SLOT(onDirCreated(QString,QString)));
        connect(mCopyProcess,SIGNAL(folderSuccessfullyCreated(QString,QString)),mProgress,SLOT(onItemCompleted()));
        connect(mCopyProcess,SIGNAL(fileSuccessfullyValidated(QString,QByteArray)),this,SLOT(onFileSuccessfulliValidated(QString,QByteArray)));


        connect(mCopyProcess,SIGNAL(processCanceled(QString)),this,SLOT(onProcessCancelled(QString)));

        //Progressbar
        connect(mCopyProcess,SIGNAL(fileSuccessfullyCopied(QString,QString)),mProgress,SLOT(onItemCompleted()));
        connect(mCopyProcess,SIGNAL(fileUnsuccessfullyCopied(QString,QString)),mProgress,SLOT(onItemCompleted()));
        connect(mProgress,SIGNAL(cancelProcess()),mCopyProcess,SLOT(cancelProcess()));
        connect(mCopyProcess,SIGNAL(processCompleted(QString)),this,SLOT(on_processCompleted(QString)));
        connect(mCopyProcess,SIGNAL(processCompleted(QString)),mProgress,SLOT(progressCompleted()));

        connect(ui->abortAllOperationsButton,SIGNAL(clicked(bool)),mProgress,SIGNAL(cancelProcess()));
        //

       QFuture<void> thread = QtConcurrent::run(mCopyProcess,&CopyProcess::start);
    }
}

void CopyingDialog::onFileCopied()
{

}

void CopyingDialog::onFileCopyStarted(QString item, QString output)
{
    ui->log->setTextColor(QColor( "black" ));
    ui->log->append(tr("Attemting to copy: %1 to %2").arg(item).arg(output));
}

void CopyingDialog::onFileValidated(bool valid, QString filename)
{
    if(valid)
    {
        ui->log->setTextColor(QColor( "blue" ));
        ui->log->append(tr("Validation successful: %1").arg(filename));
    }
    else
    {
        ui->log->setTextColor(QColor( "red" ));
        appendFailedFilesSummary(tr("Validation unsuccessful: %1").arg(filename));
        ui->log->append(tr("Validation unsuccessful: %1").arg(filename));
    }
}

void CopyingDialog::onFileUnsuccessfullyValidated(QString inFilename, QString outFilename, QByteArray InChecksum, QByteArray OutChecksum)
{
    ui->log->setTextColor(QColor( "red" ));
    ui->errorLog->setTextColor(QColor( "red" ));

    QString inchStr = InChecksum.toHex();
    QString outchStr = OutChecksum.toHex();


    QString text = tr("Validation unsuccessful. Checksums do not match inputfile %1 with checksum %3 and outputfile %2 with checksum %4").arg(inFilename).arg(outFilename).arg(QString(inchStr)).arg(QString(outchStr));

    ui->log->append(text);
    ui->errorLog->append(text);
}

void CopyingDialog::onFileSuccessfulliValidated(QString filename, QByteArray checksum)
{
    ui->log->setTextColor(QColor( "blue" ));
    QString checksumStr = checksum.toHex();
    ui->log->append(tr("Validation successful: %1 (Checksum: %2)").arg(filename).arg(checksumStr));
}

void CopyingDialog::copyingCompleted()
{   
//    bool threadsStillRunning = false;
//    foreach(CopyThread *thread,CopyThreads)
//    {
//        if(thread->isRunning())
//        {
//            threadsStillRunning = true;
//        }
//    }

//    if(!threadsStillRunning)
//    {
//        ui->log->append("");
//        ui->log->append("");
//        ui->log->append("..::"+tr("SUMMARY OF ERRORS")+"::..");
//        foreach(QString failedFile, failedFilesSummary())
//        {
//            ui->log->append(failedFile);
//        }
//    }

    //ui->totalProgressBar->setValue(ui->totalProgressBar->value()+1);
}


void CopyingDialog::on_buttonBox_rejected()
{
////    QMessageBox msgbox;
////    msgbox.setText(tr("Waiting for copying processes to stop! Please wait."));
////    msgbox.setStandardButtons(0);
////    msgbox.show();

//    foreach(CopyThread *thread, CopyThreads)
//    {
////        if(!thread->isRunning())
////        {
////            break;
////        }

//        thread->cancelProgress();

////        QTimer stoppingTimer;
////        stoppingTimer.setSingleShot(true);
////        stoppingTimer.setInterval(10000);
////        stoppingTimer.start();

////        while(thread->isRunning())
////        {
////            if(!stoppingTimer.isActive())
////            {
////                qDebug() << "Stopping process timeout, force stop!";
////                thread->quit();
////                break;
////            }

////            QCoreApplication::processEvents();
////            qDebug() << "Waiting for process " << thread << "to stop";
////        }
////        qDebug() << "Thread" << thread << "stopped!";
//    }

    //    //msgbox.hide();
}

void CopyingDialog::onFilesCountChanged(int filesCount)
{
    QApplication::processEvents();
    //ui->log->setPlainText("--->"+tr("Counting items: %1").arg(filesCount)+"<---");
    ui->log->setText("--->"+tr("Counting items: %1").arg(filesCount)+"<---");
    qDebug() << "Counting items" << filesCount;
    QApplication::processEvents();
}

void CopyingDialog::onCopyingSuccessfull(QString item,QString output)
{
    ui->log->setTextColor(QColor( "green" ));
    QString text = tr("Successfully copied: %1 to %2").arg(item).arg(output);
    ui->log->append(text);

    //ui->totalProgressBar->setValue(ui->totalProgressBar->value() +1 );
}

void CopyingDialog::onDirCreated(QString dirName, QString output)
{
    ui->log->setTextColor(QColor( "green" ));
    QString text = tr("Successfully created directory: %1 to %2").arg(dirName).arg(output);
    ui->log->append(text);
}

void CopyingDialog::onCopyingError(QString item, QString output, QString errorText)
{
    ui->log->setTextColor(QColor( "red" ));
    QString text = tr("Unsuccessfully copied: %1 to %2. ERROR: %3").arg(item).arg(output).arg(errorText);
    ui->log->append(text);

    ui->errorLog->setTextColor(QColor( "red" ));
    ui->errorLog->append(text);

   // ui->totalProgressBar->setValue(ui->totalProgressBar->value() +1 );
}

void CopyingDialog::onThreadFinished()
{
    // ui->totalProgressBar->setValue(ui->totalProgressBar->value()+1);
}

void CopyingDialog::onProcessCancelled(QString desc)
{
    ui->log->setTextColor(QColor( "red" ));
    QString text = tr("Process cancelled for output directory: %1").arg(desc);
    ui->log->append(text);

    ui->errorLog->setTextColor(QColor( "red" ));
    ui->errorLog->append(text);
}

void CopyingDialog::on_abortAllOperations_clicked()
{
    qDebug() << "ABORT ALL OPERATIONS CLICKED!";
}

void CopyingDialog::on_buttonBox_clicked(QAbstractButton *button)
{
//    switch(ui->buttonBox->buttonRole(button))
//    {
//       case QDialogButtonBox::Close:
//        if(QMessageBox::question(this,tr("Confirm"),tr("If you close this window, all operations will be cancelled. Do you want to still close?"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes)
//        {
//            emit ui->abortAllOperationsButton->clicked(true);
//        }

//        QDialog::reject();
//    }



}

void CopyingDialog::on_abortAllOperationsButton_clicked()
{
    abortOpertions = true;
}

void CopyingDialog::on_processCompleted(QString processDesc)
{
    ui->progressBar->setValue(ui->progressBar->value()+1);
}
