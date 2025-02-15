#include "mainwindow.h"
#include "./ui_mainwindow.h"

mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mainWindow)
    , fileModel(new QFileSystemModel(this))
{
    ui->setupUi(this);
    listDrives(ui->listWidgetDrives);
    setupFileListView();
    setCentralWidget(centralWidget());
}

static QString currentDir = QDir::homePath();

mainWindow::~mainWindow()
{
    delete ui;
}

void mainWindow::listDrives(QListWidget *listWidget)
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
            QString temp = QString("%1 (%2 GiB)").arg(storage.rootPath()).arg(storage.bytesTotal() / 1073741824);
            listWidget->addItem(temp);
        }
    }
}

void mainWindow::setupFileListView()
{
    fileModel->setRootPath(currentDir);
    ui->listViewFiles->setModel(fileModel);
    ui->listViewFiles->setRootIndex(fileModel->index(QDir::homePath()));
}

void mainWindow::on_btnCreateDir_clicked()
{
    int i = 0;
    while (true)
    {
        QString newFolderPath = QDir::cleanPath(currentDir + QDir::separator() + "New Folder");;
        if (i != 0)
        {
            newFolderPath += QString(" (%1)").arg(i);
        }
        QDir dir = newFolderPath;
        if (!dir.exists(newFolderPath))
        {
            dir.mkdir(newFolderPath);
            break;
        }
        else
        {
            i++;
        }
    }
}

void mainWindow::on_btnGoUp_clicked()
{
    QDir newDir(currentDir);
    if (newDir.cdUp())
    {
        currentDir = newDir.absolutePath();
        ui->listViewFiles->setRootIndex(fileModel->index(currentDir));
    }
}

void mainWindow::on_listViewFiles_doubleClicked()
{
    QModelIndex index = ui->listViewFiles->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    QString newDir = QDir::cleanPath(currentDir + QDir::separator() + itemText);
    if (QDir(newDir).exists())
    {
        currentDir = newDir;
        ui->listViewFiles->setRootIndex(fileModel->index(currentDir));
    }
}

