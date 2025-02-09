#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    listDrives(ui->listWidgetDrives);
    setupFileListView();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::listDrives(QListWidget *listWidget)
{
    listWidget->clear();

    for (const QStorageInfo &storage : QStorageInfo::mountedVolumes())
    {
        if (storage.isValid() && !storage.isReadOnly())
        {
            QString path = storage.rootPath();
            if(path.startsWith("/run") || path.startsWith("/boot"))
            {
                continue;
            }
            QString temp = QString("%1   %2 GiB").arg(storage.rootPath()).arg(storage.bytesTotal() / 1073741824);
            listWidget->addItem(temp);
        }
    }
}

void MainWindow::setupFileListView()
{
    QString homeDir = QDir::homePath();
    QFileSystemModel *fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(homeDir);
    ui->listViewFiles->setModel(fileModel);
    ui->listViewFiles->setRootIndex(fileModel->index(homeDir));
}

