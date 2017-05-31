#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStorageInfo>
#include <QDebug>
#include <QDir>
#include <QFileInfoList>
#include <QStorageInfo>
#include <QFileDialog>
#include <copyingdialog.h>
#include <QProcess>
#include <QFileSystemModel>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QTimer>
#include <QToolBar>

#include "formatdrivesdialog.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->setCurrentIndex(0);

    QToolBar *mToolbar = new QToolBar(this);
    this->addToolBar(mToolbar);

    mToolbar->addAction(ui->actionFormat_drives);
    mToolbar->addSeparator();
    mToolbar->addAction(ui->actionAbout);

    mToolbar->addAction(QIcon(":/icons/Built_with_Qt_RGB_logo_vertical.png"),tr("About &Qt"), &QApplication::aboutQt);

    MainWindow::showNormal();

    //updates and sets the treeViews



    ui->tabWidget->tabBar()->setVisible(false);
    updateTabNavigationButtons();

    ui->tabWidget->setCurrentIndex(1);
    refreshSourceTreeview();

    QTimer updateTimer;
    updateTimer.singleShot(500,ui->refreshListButton,SLOT(click()));

    ui->tabWidget->setCurrentIndex(2);
    updateTimer.singleShot(1000,ui->refreshListButton_2,SLOT(click()));
    ui->tabWidget->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::refreshTreeViews()
{
    refreshSourceTreeview();
    refreshDestinationTreeview();
    updateUserEnabled();

}

void MainWindow::refreshSourceTreeview()
{
    QFileSystemModel *model = new QFileSystemModel(this);
    model->setFilter(QDir::NoDotAndDotDot |
                            QDir::AllEntries);;
    model->setRootPath(QDir::homePath());

    ui->sourceTreeview->setModel(model);
    //QModelIndex rootIndex = model->index(QDir::cleanPath(QDir::homePath()));
    //ui->treeView->setRootIndex(rootIndex);
    ui->sourceTreeview->setSelectionMode(QAbstractItemView::ExtendedSelection);


    ui->sourceTreeview->setSortingEnabled(true);
    //const QSize availableSize = QApplication::desktop()->availableGeometry(ui->sourceTreeview).size();
    //ui->sourceTreeview->resize(availableSize / 2);
    ui->sourceTreeview->setColumnWidth(0, ui->sourceTreeview->width() / 3);
}

void MainWindow::refreshDestinationTreeview()
{
    QFileSystemModel *model = new QFileSystemModel(this);
    model->setFilter(QDir::NoDotAndDotDot |
                            QDir::AllDirs);;
    model->setRootPath(QDir::homePath());

    ui->destinationTreeview->setModel(model);
    //QModelIndex rootIndex = model->index(QDir::cleanPath(QDir::homePath()));
    //ui->treeView->setRootIndex(rootIndex);
    ui->destinationTreeview->setSelectionMode(QAbstractItemView::ExtendedSelection);


    ui->destinationTreeview->setSortingEnabled(true);
//    const QSize availableSize = QApplication::desktop()->availableGeometry(ui->destinationTreeview).size();
//    ui->destinationTreeview->resize(availableSize / 2);
    ui->destinationTreeview->setColumnWidth(0, ui->destinationTreeview->width() / 3);
}

bool MainWindow::hasSelections()
{
    if(ui->sourceTreeview->selectionModel()->model()->rowCount()>0)
    {
        if(ui->destinationTreeview->selectionModel()->model()->rowCount() > 0)
        {
            return true;
        }
    }

    return false;
}

void MainWindow::updateUserEnabled()
{
    qDebug() << hasSelections();
    ui->beginCopyingButton->setEnabled(hasSelections());
}



void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->destinationTreeview->setColumnWidth(0, ui->destinationTreeview->width() / 3);
    ui->sourceTreeview->setColumnWidth(0, ui->destinationTreeview->width() / 3);
}

void MainWindow::on_beginCopyingButton_released()
{
    if(QMessageBox::question(this,tr("Confirm operation"),tr("Sure you want to start this operation? It can not be aborted when its running"))!= QMessageBox::Yes)
    {
        return;
    }

    QStringList sourcesList;
    QStringList destinationsList;
    int row = -1;

    //Makes all source selection to a QStringList for later use.
    QFileSystemModel* sourceModel = (QFileSystemModel*)ui->sourceTreeview->model();
    foreach (QModelIndex index, ui->sourceTreeview->selectionModel()->selectedIndexes())
    {
        if (index.row()!=row && index.column()==0)
        {
            QFileInfo fileInfo = sourceModel->fileInfo(index);
            sourcesList.append(fileInfo.absoluteFilePath());
            row = index.row();
        }
    }

    //Makes all destination selection to a QStringList for later use.
    QFileSystemModel* destinationModel = (QFileSystemModel*)ui->destinationTreeview->model();
    foreach (QModelIndex index, ui->destinationTreeview->selectionModel()->selectedIndexes())
    {
        if (index.row()!=row && index.column()==0)
        {
            QFileInfo fileInfo = destinationModel->fileInfo(index);
            destinationsList.append(fileInfo.absoluteFilePath());
            row = index.row();
        }
    }

    foreach(QString dest, destinationsList)
    {
        foreach(QString src, sourcesList)
        {
            qDebug() << "src" << src << "dest" << dest;
            if(src == dest)
            {
                QMessageBox::critical(this,tr("Impossible selection"),tr("You have selected one or more sources directories as destination directories as well. It is not possible copying a directory to it self."),QMessageBox::Ok);
                return;
            }
        }
    }

    CopyingDialog *mDialog = new CopyingDialog(0);
    //mDialog->setInputDirectory(ui->inputBrowsePath->text());

    //Converts the list from a QList to QStringList
//    QStringList selectedDrives;
//    foreach(QListWidgetItem *drive, ui->drivesList->selectedItems())
//    {
//        selectedDrives.append(drive->text());
//    }

    //UNUSED
    //mDialog->setFormatDiskLabel(ui->formatedDiskLabel->text());
    mDialog->setFormatDrives(false);

    //Sets sourceElements
    mDialog->setSourceElements(sourcesList);

    //Overwrite existing files
    mDialog->setOverwritefiles(ui->overwriteFiles->isChecked());

    //Do validation
    mDialog->setValidateData(ui->validateData->isChecked());

    //Sets destinations
    mDialog->setOutputDirectories(destinationsList);

    //Modal and show
    mDialog->setModal(true);
    mDialog->show();
    mDialog->beginCopying();
}

void MainWindow::on_refreshListButton_clicked()
{
    refreshDestinationTreeview();
    refreshSourceTreeview();
}

void MainWindow::on_drivesList_clicked(const QModelIndex &index)
{
    ui->beginCopyingButton->setEnabled(true);
//    if(ui->drivesList->selectedItems().count() <= 0 )
//    {
//        ui->beginCopyingButton->setEnabled(false);
//    }
}

void MainWindow::on_formatDrive_clicked()
{
    //format [drive] [filesystem] [QuickFormat] [Force eject disk] [label]
    //format D: /FS:FAT32 /Q /X /V:DISK
}

void MainWindow::on_formatDrive_toggled(bool checked)
{
}


void MainWindow::on_sourceTreeview_clicked(const QModelIndex &index)
{
    updateUserEnabled();
}

void MainWindow::on_beginCopyingButton_clicked()
{

}

void MainWindow::on_actionFormat_drives_triggered()
{
    FormatDrivesDialog mDialog;
    mDialog.exec();

    refreshTreeViews();
}

void MainWindow::on_previousTabButton_clicked()
{
    ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()-1);

    updateTabNavigationButtons();
}

void MainWindow::on_nextTabButton_clicked()
{
    ui->tabWidget->setCurrentIndex(ui->tabWidget->currentIndex()+1);
    updateTabNavigationButtons();
}

void MainWindow::updateTabNavigationButtons()
{
    ui->nextTabButton->setEnabled(true);
    ui->previousTabButton->setEnabled(true);

    if(ui->tabWidget->currentIndex()<=0)
    {
        ui->previousTabButton->setEnabled(false);
    }

    if(ui->tabWidget->currentIndex()>=ui->tabWidget->count()-1)
    {
        summeryGenerator(); //Creat summary;
        ui->nextTabButton->setEnabled(false);
    }
}

void MainWindow::summeryGenerator()
{
    QStringList sourcesList;
    QStringList destinationsList;
    int row = -1;

    //Makes all source selection to a QStringList for later use.
    QFileSystemModel* sourceModel = (QFileSystemModel*)ui->sourceTreeview->model();
    foreach (QModelIndex index, ui->sourceTreeview->selectionModel()->selectedIndexes())
    {
        if (index.row()!=row && index.column()==0)
        {
            QFileInfo fileInfo = sourceModel->fileInfo(index);
            sourcesList.append(fileInfo.absoluteFilePath());
            row = index.row();
        }
    }

    //Makes all destination selection to a QStringList for later use.
    QFileSystemModel* destinationModel = (QFileSystemModel*)ui->destinationTreeview->model();
    foreach (QModelIndex index, ui->destinationTreeview->selectionModel()->selectedIndexes())
    {
        if (index.row()!=row && index.column()==0)
        {
            QFileInfo fileInfo = destinationModel->fileInfo(index);
            destinationsList.append(fileInfo.absoluteFilePath());
            row = index.row();
        }
    }

    ui->beginCopyingButton->setEnabled(true);
    if(destinationsList.count()<=0 || sourcesList.count() <= 0)
    {
        ui->summery->setText(tr("You MUST at least select one source and one destination! Try again"));
        ui->beginCopyingButton->setEnabled(false);
        return;
    }

    ui->summery->setText(tr("The following sources will be copied"));
    foreach(QString src, sourcesList)
    {
        ui->summery->append(QString("- %1").arg(src));
    }

    ui->summery->append("");
    ui->summery->append(tr("To the following destinations"));
    foreach(QString dest, destinationsList)
    {
        ui->summery->append(QString("- %1").arg(dest));
    }

    ui->summery->append("");
    ui->summery->append(tr("Options"));
    ui->summery->append("- "+tr("Overwrite existing files in destinations: %1").arg((ui->overwriteFiles->isChecked()) ? tr("Yes") : tr("No")));
    ui->summery->append("- "+tr("Validate copied files in destinations: %1").arg((ui->validateData->isChecked()) ? tr("Yes") : tr("No")));
    ui->summery->append("<br><br>");
    ui->summery->append(tr("When ready press '%1'").arg(ui->beginCopyingButton->text()));
}

void MainWindow::on_refreshListButton_2_clicked()
{
    on_refreshListButton_clicked();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::information(this,tr("About"),tr("Created by Ole Dahl Frandsen, 2017"));
}
