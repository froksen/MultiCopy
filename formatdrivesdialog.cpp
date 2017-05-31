#include "formatdrivesdialog.h"
#include "ui_formatdrivesdialog.h"
#include <QStorageInfo>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>

FormatDrivesDialog::FormatDrivesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FormatDrivesDialog)
{
    ui->setupUi(this);

    updateDrivesList();

    ui->formatButton->setEnabled(false);
}

FormatDrivesDialog::~FormatDrivesDialog()
{
    delete ui;
}

void FormatDrivesDialog::updateDrivesList()
{
    //Clears the list
    ui->drivesList->clear();

    //Appends to list
    if(!ui->showAllDrives->isChecked())
    {
        if(QStorageInfo::mountedVolumes().count() <= 0)
        {
            ui->drivesList->setEnabled(false);
            ui->drivesList->insertItem(0,tr("No mounted drives found!"));
        }

        foreach(QStorageInfo storage, QStorageInfo::mountedVolumes())
        {
            if (storage.isValid() && storage.isReady()) {
                if (!storage.isReadOnly()) {

                    //If path not containg WINDOWS directory
                    QString windowsPath = storage.rootPath()+QString("/Windows");
                    QDir windowsDir(windowsPath);
                    if(!windowsDir.exists())
                    {
                        int row = ui->drivesList->count();
                        ui->drivesList->insertItem(row,storage.rootPath());
                    }

                }
            }
        }
    }
    else
    {
        foreach(QFileInfo drive,QDir::drives())
        {
            int row = ui->drivesList->count();
            ui->drivesList->insertItem(row,drive.absolutePath());
        }
    }

    //If not drives found
    ui->drivesList->setEnabled(true);
    if(ui->drivesList->count() <= 0)
    {
        ui->drivesList->insertItem(0,tr("No drives found. Insert some and press '%1'").arg(ui->refreshListButton->text()));
        ui->drivesList->setEnabled(false);
    }
}

bool FormatDrivesDialog::formatDrive(QString driveLetter, QString newLabel,bool autoYes)
{
    if(newLabel.size()>10)
    {
        newLabel = newLabel.left(9);
        qDebug() << "New label is too long, must only be 11 characters. Been shortened to" << newLabel;
    }

    if(newLabel.size()<0 || newLabel.isEmpty())
    {
        newLabel = "NewLabel";
    }

    //Checks if drive Exists
    QDir drive(driveLetter);
    if(!drive.exists())
    {
        qDebug() << "Drive" << driveLetter << "does not exists!";
        return false;
    }

    //Removes slashes from drive letter, if any so D:/ => D:
    driveLetter = driveLetter.remove("\\").remove("/");

    shell = new QProcess(this);
    shell->setProcessChannelMode(QProcess::MergedChannels);

    //connect(shell,SIGNAL(finished(int)),this,SLOT(deleteLater()));
    connect(shell,SIGNAL(readyReadStandardOutput()),this,SLOT(onReadyReadStandardOutput()));
    connect(shell,SIGNAL(finished(int,QProcess::ExitStatus)),this,SLOT(onProgressFinished(int,QProcess::ExitStatus)));

    ui->log->append("**********************");
    ui->log->append("FORMATING DRIVE " + driveLetter);
    ui->log->append("**********************");

    QStringList arguments;
    arguments << "/K" << "FORMAT" << driveLetter << "/FS:FAT32" << "/Q" << "/X" << "/V:nFDisk";
    if(autoYes) arguments.append("/y");
    arguments << "&&" << "LABEL" << driveLetter << newLabel;
    arguments << "&&" << "exit";
    shell->start("cmd.exe",arguments);

//    if(shell->execute("cmd.exe",arguments) == 0)
//    {
//        qDebug() << "is Completed";
//        return true;
//    }
//    return false;
}

void FormatDrivesDialog::doFormating()
{
    //Progressbar Max
    ui->progressBar->setMaximum(ui->drivesList->selectedItems().count());

    ui->closeButton->setEnabled(false);
    ui->formatButton->setEnabled(false);
    ui->drivesList->setEnabled(false);
    ui->showAllDrives->setEnabled(false);
    ui->refreshListButton->setEnabled(false);
    ui->newLabel->setEnabled(false);

    //Loops through the items
    foreach(QListWidgetItem *item, ui->drivesList->selectedItems())
    {
        formatDrive(item->text(),ui->newLabel->text(),true);
    }


}

void FormatDrivesDialog::on_showAllDrives_clicked()
{
    updateDrivesList();
}

void FormatDrivesDialog::on_refreshListButton_clicked()
{
    updateDrivesList();
}

void FormatDrivesDialog::on_formatButton_clicked()
{
    if(QMessageBox::question(this,"Confirm operation","Sure you want to start this operation? It can not be aborted when its running")!= QMessageBox::Yes)
    {
        return;
    }

    doFormating();
}

void FormatDrivesDialog::onReadyReadStandardOutput()
{
    qDebug() << "readyOut";
    QProcess *p = (QProcess *)sender();
    QByteArray buf = p->readAllStandardOutput();

    ui->log->append(buf);
    //qDebug() << buf;
}

void FormatDrivesDialog::onProgressFinished(int exitcode, QProcess::ExitStatus exitstatus)
{
    ui->progressBar->setValue(ui->progressBar->value()+1);

    if(ui->progressBar->value()==ui->progressBar->maximum())
    {
        ui->closeButton->setEnabled(true);
        ui->formatButton->setEnabled(true);
        ui->drivesList->setEnabled(true);
        ui->showAllDrives->setEnabled(true);
        ui->refreshListButton->setEnabled(true);
        ui->newLabel->setEnabled(true);

    }
}

void FormatDrivesDialog::on_showAllDrives_clicked(bool checked)
{
    if(checked)
    {
        QMessageBox msgbox;
        msgbox.setWindowTitle(tr("WARNING"));
        msgbox.setIcon(QMessageBox::Warning);
        msgbox.setText(tr("Proceed with caution. If this checkbox is checked all drives is available for formating. This will potentially ease ALL data from the drives (and system drives) and CAN´T be undone!"));
        msgbox.exec();
    }
}

void FormatDrivesDialog::on_closeButton_clicked()
{
    this->close();
}

void FormatDrivesDialog::on_drivesList_itemSelectionChanged()
{
    ui->formatButton->setEnabled(false);
    if(ui->drivesList->selectedItems().count()>0)
    {
        ui->formatButton->setEnabled(true);
    }

}
