#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "propertiesdialog.h"

mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mainWindow)
    , fileModel(new QFileSystemModel(this))
{
    ui->setupUi(this);
    listDrives(ui->listWidgetDrives);
    setupFileListView();
    setCentralWidget(centralWidget());
    ui->listViewFiles->setContextMenuPolicy(Qt::CustomContextMenu);
    connect (ui->listViewFiles, &QListView::customContextMenuRequested, this, &mainWindow::showContextMenu);
    QShortcut *shortcutCopy = new QShortcut(QKeySequence::Copy, this);
    QShortcut *shortcutCut = new QShortcut(QKeySequence::Cut, this);
    QShortcut *shortcutPaste = new QShortcut(QKeySequence::Paste, this);
    QShortcut *shortcutDelete = new QShortcut(QKeySequence::Delete, this);

    connect(shortcutCopy, &QShortcut::activated, this, &mainWindow::copy);
    connect(shortcutCut, &QShortcut::activated, this, &mainWindow::cut);
    connect(shortcutPaste, &QShortcut::activated, this, &mainWindow::paste);
    connect(shortcutDelete, &QShortcut::activated, this, &mainWindow::remove);
}

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
    ui->txtCurrentPath->setText(currentDir);
}

void mainWindow::on_btnCreateDir_clicked()
{
    int i = 0;
    while (true)
    {
        try
        {
            QString newPath = QDir::cleanPath(currentDir + QDir::separator() + "New Folder");;
            if (i != 0)
            {
                newPath += QString(" (%1)").arg(i);
            }
            QDir dir = newPath;
            if (!dir.exists(newPath))
            {
                dir.mkdir(newPath);
                break;
            }
            else
            {
                i++;
            }
        }
        catch (std::exception ex)
        {
            QMessageBox::critical(nullptr, "Error", "Operation unsuccessful, check if you have write permission. ", QMessageBox::Ok);
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
        ui->txtCurrentPath->setText(currentDir);
    }
}

void mainWindow::on_listViewFiles_doubleClicked()
{
    QModelIndex index = ui->listViewFiles->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    QString newDir = QDir::cleanPath(currentDir + QDir::separator() + itemText);
    if (QFileInfo::exists(newDir))
    {
        if (QFileInfo(newDir).isDir())
        {
            currentDir = newDir;
            ui->listViewFiles->setRootIndex(fileModel->index(currentDir));
            ui->txtCurrentPath->setText(currentDir);
        }
        else
        {
            mainWindow::open();
        }
    }
}

void mainWindow::on_listWidgetDrives_doubleClicked()
{
    QModelIndex index = ui->listWidgetDrives->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    itemText = itemText.left(itemText.lastIndexOf(" ("));
    QString newDir = QDir::cleanPath(itemText);
    if (QDir(newDir).exists())
    {
        currentDir = newDir;
        ui->listViewFiles->setRootIndex(fileModel->index(currentDir));
        ui->txtCurrentPath->setText(currentDir);
    }
}

void mainWindow::showContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listViewFiles->currentIndex();
    if (!index.isValid()) return;
    QMenu contextMenu(this);

    QAction *openAction = new QAction("Open", this);
    QAction *copyAction = new QAction("Copy", this);
    QAction *cutAction = new QAction("Cut", this);
    QAction *renameAction = new QAction("Rename", this);
    QAction *deleteAction = new QAction("Delete", this);
    QAction *propertiesAction = new QAction("Properties", this);

    connect (openAction, &QAction::triggered, this, &mainWindow::open);
    connect (copyAction, &QAction::triggered, this, &mainWindow::copy);
    connect (cutAction, &QAction::triggered, this, &mainWindow::cut);
    connect (renameAction, &QAction::triggered, this, &mainWindow::rename);
    connect (deleteAction, &QAction::triggered, this, &mainWindow::remove);
    connect (propertiesAction, &QAction::triggered, this, &mainWindow::properties);

    contextMenu.addAction(openAction);
    contextMenu.addAction(copyAction);
    contextMenu.addAction(cutAction);
    if (!clipboard.isEmpty())
    {
        QAction *pasteAction = new QAction("Paste", this);
        contextMenu.addAction(pasteAction);
        connect (pasteAction, &QAction::triggered, this, &mainWindow::paste);
    }
    contextMenu.addAction(renameAction);
    contextMenu.addAction(deleteAction);
    contextMenu.addAction(propertiesAction);

    contextMenu.exec(ui->listViewFiles->viewport()->mapToGlobal(pos));
}


void mainWindow::open ()
{
    QModelIndex index = ui->listViewFiles->currentIndex();
    QString itemText = index.data(Qt::DisplayRole).toString();
    QString path = QDir::cleanPath(currentDir + QDir::separator() + itemText);
    QFileInfo info(path);
    if (info.isDir())
    {
        on_listViewFiles_doubleClicked();
    }
    else
    {
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(path))) QMessageBox::warning(nullptr, "Error", "Operation unsuccessful, no app that can open selected file. ", QMessageBox::Ok);
    }
}

void mainWindow::copy ()
{
    clipboard.clear();
    QStringList list;
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())
        list.append(fileModel->filePath(index));
    foreach (QString item, list)
        clipboard.append(item);
    mv = false;
}

void mainWindow::cut ()
{
    clipboard.clear();
    QStringList list;
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())
        list.append(fileModel->filePath(index));
    foreach (QString item, list)
        clipboard.append(item);
    mv = true;
}

void mainWindow::paste ()
{
    if (!clipboard.isEmpty())
    {
        try
        {
            foreach (const QString oldPath, clipboard)
            {                
                const QFileInfo info(oldPath);
                QString name = info.fileName();
                QString newPath = currentDir + QDir::separator() + name;
                if (QFile::exists(newPath))
                {
                    int response;
                    if (oldPath == newPath) response = 2;
                    else response = conflict(newPath);
                    if (response == 0) break;
                    if (response == 1) del(newPath);
                    if (response == 2)
                    {
                        int i = 1;
                        while (true)
                        {
                            newPath = currentDir + QDir::separator() + name + " (" + QString::number(i) + ")";
                            if (!QFileInfo::exists(newPath))
                            {
                                name += QString(" (%1)").arg(i);
                                break;
                            }
                            else
                            {
                                i++;
                            }
                        }
                    }
                }
                if (info.isDir())
                {
                    QDir destDir;
                    destDir.mkpath(newPath);
                    pasteRecursively(oldPath, newPath);
                }
                else
                {
                    if (!QFile::copy(oldPath, newPath))throw std::runtime_error("Error in copying of a file.");
                }
                if (mv) del(oldPath);
            }
        }
        catch (std::exception ex)
        {
            QMessageBox::critical(nullptr, "Error", "Operation unsuccessful, check if you have write permission. ", QMessageBox::Ok);
        }
    }
    clipboard.clear();
}

void mainWindow::rename ()
{
    try
    {
        foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())
        {
            bool ok;
            QString oldPath = fileModel->filePath(index);
            const QFileInfo info(oldPath);
            QString name = info.fileName();
            QString newName = QInputDialog::getText(this, "Rename file", "Enter new name for " + name, QLineEdit::Normal, name, &ok);
            if (ok && !newName.isEmpty())
            {
                QFile file(oldPath);
                file.rename(currentDir + QDir::separator() + newName);
            }
        }
    }
    catch (std::exception ex)
    {
        QMessageBox::critical(nullptr, "Error", "Operation unsuccessful, check if you have write permission. ", QMessageBox::Ok);
    }
}

void mainWindow::remove ()
{
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())
    {
        QString itemPath = fileModel->filePath(index);
        const QFileInfo info(itemPath);
        const QString name = info.fileName();

        try
        {
            if (info.isDir())
            {
                if (QMessageBox::warning(nullptr, "Warning", "Do you really want to remove " + name + "?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
                {
                    QDir dir(itemPath);
                    dir.removeRecursively();
                }
                else continue;
            }
            else
            {
                QFile file(itemPath);
                file.remove();
            }
        }
        catch (std::exception ex)
        {
            QMessageBox::critical(nullptr, "Error", "Operation unsuccessful, check if you have write permission. ", QMessageBox::Ok);
        }
    }
}

void mainWindow::del (QString d)
{
    const QFileInfo info(d);
    try
    {
        if (info.isFile())
        {            
            QFile file(d);
            if (file.exists()) file.remove();
        }
        else if (info.isDir())
        {            
            QDir dir(d);
            if (dir.exists()) dir.removeRecursively();
        }
    }
    catch (std::exception ex)
    {
        QMessageBox::critical(nullptr, "Error", "Operation unsuccessful, check if you have write permission for " + d + ". ", QMessageBox::Ok);
    }
}

void mainWindow::properties ()
{
    QStringList list;
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())list.append(fileModel->filePath(index));
    if (list.length() != 1) return;
    else
    {
        QString path = list[0];
        QFileInfo info(path);
        if (info.exists())
        {
            propertiesDialog dialog(path, this);
            dialog.exec();
        }
    }
}

bool mainWindow::pasteRecursively(QString source, QString destination)
{
    bool success = true;
    try
    {
        QDir dir(source);
        if (!dir.exists()) return false;
        foreach (QString d, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
        {
            QString destinationPath = destination + QDir::separator() + d;
            dir.mkpath(destinationPath);
            if (!pasteRecursively(source + QDir::separator() + d, destinationPath)) success = false;
        }
        foreach (QString f, dir.entryList(QDir::Files))
        {
            if (!QFile::copy(source + QDir::separator() + f, destination + QDir::separator() + f)) success = false;
        }
    }
    catch (std::exception ex)
    {
        QMessageBox::critical(nullptr, "Error", "Operation unsuccessful, check if you have write permission. ", QMessageBox::Ok);
    }
    return success;
}

int mainWindow::conflict(QString &d)
{
    QString type;
    const QFileInfo info(d);
    const QString name = info.fileName();
    if (info.isDir()) type = "Directory";
    else type = "File";
    QMessageBox msg;
    msg.setIcon(QMessageBox::Warning);
    msg.setWindowTitle("File Conflict");
    msg.setText(type + " " + d + " already exists. ");
    msg.setStandardButtons(QMessageBox::Discard | QMessageBox::Ignore | QMessageBox::Retry);
    msg.button(QMessageBox::Discard)->setText("Overwrite");
    msg.button(QMessageBox::Ignore)->setText("Rename");
    msg.button(QMessageBox::Retry)->setText("Skip");
    int input = msg.exec();
    if (input == QMessageBox::Discard)
    {
        return 1;
    }
    else if  (input == QMessageBox::Ignore)
    {
        return 2;
    }
    else return 0;
}


