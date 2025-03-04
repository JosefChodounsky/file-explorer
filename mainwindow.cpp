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

    connect(ui->searchTxt, &QLineEdit::returnPressed, this, [this]() {
        performSearch(ui->searchTxt->text());
    });
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
    try
    {
        QDir dir;
        dir.mkpath(getAvailableName(currentDir + QDir::separator() + "New Folder", ""));
    }
    catch (std::exception ex)
    {
        QMessageBox::critical(nullptr, "Error", "Operation unsuccessful, check if you have write permission. ", QMessageBox::Ok);
    }
}

void mainWindow::on_btnGoUp_clicked()
{
    QDir newDir(currentDir);
    newDir.cdUp();
    currentDir = newDir.absolutePath();
    ui->searchTxt->clear();
    performSearch(ui->searchTxt->text());
    ui->listViewFiles->setRootIndex(fileModel->index(currentDir));
    ui->txtCurrentPath->setText(currentDir);
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

QString mainWindow::getFilePath(const QModelIndex &index)
{
    QString path;
    if (!isQFileSystemModel())
    {
        path = index.data(Qt::UserRole).toString();
    }
    else
    {
        path = fileModel->filePath(index);
    }
    return QDir::cleanPath(path);
}

bool mainWindow::isQFileSystemModel()
{
    if (QStandardItemModel* standardModel = qobject_cast<QStandardItemModel*>(ui->listViewFiles->model()))
    {
        return false;
    }
    else
    {
        return true;
    }
}

void mainWindow::performSearch(const QString& keyword)
{
    if (keyword.isEmpty())
    {
        resetFileView();
        return;
    }

    QFileInfoList results = findFiles(currentDir, keyword);

    QStandardItemModel* searchModel = new QStandardItemModel(this);

    foreach (const QFileInfo& fileInfo, results)
    {
        QStandardItem* item = new QStandardItem;
        item->setText(fileInfo.fileName());
        item->setData(fileInfo.filePath(), Qt::UserRole);

        if (fileInfo.isDir())
        {
            item->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
        }
        else
        {
            item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
        }
        searchModel->appendRow(item);
    }
    ui->listViewFiles->setModel(searchModel);
}

void mainWindow::resetFileView()
{
    ui->listViewFiles->setModel(fileModel);
    ui->listViewFiles->setRootIndex(fileModel->index(currentDir));
}

QFileInfoList mainWindow::findFiles(const QString& startDir, const QString& keyword)
{
    QFileInfoList results;
    QDirIterator it(startDir, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
        it.next();
        QFileInfo info = it.fileInfo();
        if (info.fileName().contains(keyword, Qt::CaseInsensitive))
        {
            results.append(info);
        }
    }
    return results;
}

int mainWindow::isArchive()
{
    QStringList list;
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())
    {
        list.append(getFilePath(index));
        if (!index.isValid()) return 0;
    }
    if (list.length() == 1)
    {
        QFileInfo info(list[0]);
        if (info.completeSuffix() == "zip") return 1;
    }
    bool notArchive = true;
    foreach (QString item, list)
    {
        QFileInfo info(item);
        if (info.completeSuffix() == "zip") notArchive = false;
    }
    if (notArchive) return 2;
    return 0;
}

void mainWindow::showContextMenu(const QPoint &pos)
{
    QModelIndex index = ui->listViewFiles->currentIndex();
    if (!index.isValid()) return;
    int archive = isArchive();
    QMenu contextMenu(this);

    QAction *copyAction = new QAction("Copy", this);
    QAction *cutAction = new QAction("Cut", this);
    QAction *renameAction = new QAction("Rename", this);
    QAction *deleteAction = new QAction("Delete", this);    

    connect (copyAction, &QAction::triggered, this, &mainWindow::copy);
    connect (cutAction, &QAction::triggered, this, &mainWindow::cut);
    connect (renameAction, &QAction::triggered, this, &mainWindow::rename);
    connect (deleteAction, &QAction::triggered, this, &mainWindow::remove);

    if (ui->listViewFiles->selectionModel()->selectedIndexes().length() == 1)
    {
        QAction *openAction = new QAction("Open", this);
        connect (openAction, &QAction::triggered, this, &mainWindow::open);
        contextMenu.addAction(openAction);
    }
    contextMenu.addAction(copyAction);
    contextMenu.addAction(cutAction);
    if (!clipboard.isEmpty() && isQFileSystemModel())
    {
        QAction *pasteAction = new QAction("Paste", this);
        contextMenu.addAction(pasteAction);
        connect (pasteAction, &QAction::triggered, this, &mainWindow::paste);
    }
    contextMenu.addAction(renameAction);
    contextMenu.addAction(deleteAction);
    if (archive == 1)
    {
        QAction *unzipAction = new QAction("Extract here", this);
        connect (unzipAction, &QAction::triggered, this, &mainWindow::unzip);
        contextMenu.addAction(unzipAction);
    }

    if (archive == 2)
    {
        QAction *zipAction = new QAction("Compress to .zip file", this);
        connect (zipAction, &QAction::triggered, this, &mainWindow::zip);
        contextMenu.addAction(zipAction);
    }
    if (ui->listViewFiles->selectionModel()->selectedIndexes().length() == 1)
    {
        QAction *propertiesAction = new QAction("Properties", this);
        connect (propertiesAction, &QAction::triggered, this, &mainWindow::properties);
        contextMenu.addAction(propertiesAction);
    }
    contextMenu.exec(ui->listViewFiles->viewport()->mapToGlobal(pos));
}


void mainWindow::open ()
{
    QModelIndex index = ui->listViewFiles->currentIndex();    
    QString path = getFilePath(index);
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
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())
        clipboard.append(QDir::cleanPath(getFilePath(index)));
    mv = false;
}

void mainWindow::cut ()
{
    clipboard.clear();
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())
        clipboard.append(QDir::cleanPath(getFilePath(index)));
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
                const QString baseName = info.baseName();
                const QString completeSuffix = info.completeSuffix();
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
                        QString suffix;
                        if (completeSuffix != "")suffix = "." + completeSuffix;
                        else suffix = "";
                        newPath = getAvailableName(currentDir + QDir::separator() + baseName, suffix);
                    }
                }
                if (info.isDir())
                {                    
                    if (!currentDir.startsWith(oldPath))
                    {
                        QDir dir;
                        dir.mkpath(newPath);
                        pasteRecursively(oldPath, newPath);
                    }
                    else
                    {
                        QMessageBox::critical(nullptr, "Error", "Can't paste a directory in itself.  ", QMessageBox::Ok);
                    }
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
            QString oldPath = getFilePath(index);
            const QFileInfo info(oldPath);
            QString path = info.absolutePath();
            QString name = info.fileName();
            QString newName = QInputDialog::getText(this, "Rename file", "Enter new name for " + name, QLineEdit::Normal, name, &ok);
            if (ok && !newName.isEmpty())
            {
                QFile file(oldPath);
                file.rename(path + QDir::separator() + newName);
            }
        }
        if (!isQFileSystemModel()) performSearch(ui->searchTxt->text());;
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
        QString path = getFilePath(index);
        const QFileInfo info(path);
        const QString name = info.fileName();

        try
        {
            if (info.isDir())
            {
                if (QMessageBox::warning(nullptr, "Warning", "Do you really want to remove " + name + "?", QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok)
                {
                    QDir dir(path);
                    dir.removeRecursively();
                }
                else continue;
            }
            else
            {
                QFile file(path);
                file.remove();
            }
        }
        catch (std::exception ex)
        {
            QMessageBox::critical(nullptr, "Error", "Operation unsuccessful, check if you have write permission. ", QMessageBox::Ok);
        }
    }
    if (!isQFileSystemModel()) performSearch(ui->searchTxt->text());
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

void mainWindow::zip()
{
    int i = 0;
    QString zipPath;
    zipPath = getAvailableName(currentDir + QDir::separator() + "output", ".zip");
    QuaZip zipArchive(zipPath);
    if(!zipArchive.open(QuaZip::mdCreate))
    {
        QMessageBox::critical(nullptr, "Error", "Failed to create " + zipPath, QMessageBox::Ok);
        return;
    }
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())
    {
        QString filePath = getFilePath(index);
        addToZip(filePath, "", &zipArchive);
    }
    zipArchive.close();
}

void mainWindow::addToZip(const QString &filePath, const QString &parentDir, QuaZip *zipArchive)
{
    QFileInfo fileInfo(filePath);
    QString inZipPath;
    if (parentDir.isEmpty())
    {
        inZipPath = fileInfo.fileName();
    } else
    {
        inZipPath = parentDir + QDir::separator() + fileInfo.fileName();
    }
    if (fileInfo.isDir())
    {
        if (!inZipPath.isEmpty())
        {
            QuaZipFile dirZipFile(zipArchive);
            QuaZipNewInfo dirInfo(inZipPath + QDir::separator(), filePath);
            if (dirZipFile.open(QIODevice::WriteOnly, dirInfo))
            {
                dirZipFile.close();
            }
        }
        QDir dir(filePath);
        QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        foreach(const QFileInfo &entry, entries)
        {
            addToZip(entry.absoluteFilePath(), inZipPath, zipArchive);
        }
    } else
    {
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly))
        {
            QuaZipFile zipFile(zipArchive);
            QuaZipNewInfo newInfo(inZipPath, filePath);
            if (zipFile.open(QIODevice::WriteOnly, newInfo))
            {
                const qint64 chunkSize = 4 * 1024 * 1024; // 4MiB
                QByteArray buffer;
                buffer.resize(chunkSize);
                qint64 bytesRead;
                while ((bytesRead = file.read(buffer.data(), chunkSize)) > 0)
                {
                    zipFile.write(buffer.data(), bytesRead);
                }
                zipFile.close();
            }
            file.close();
        }
    }
}

void mainWindow::unzip()
{
    QModelIndexList index = ui->listViewFiles->selectionModel()->selectedIndexes();
    QString zipPath = getFilePath(index[0]);
    QFileInfo info(zipPath);
    int i = 0;
    QString newPath = getAvailableName(currentDir + QDir::separator() + info.baseName(), "");
    QDir dir;
    dir.mkpath(newPath);

    QuaZip zipArchive(zipPath);
    if(!zipArchive.open(QuaZip::mdUnzip))
    {
        QMessageBox::critical(nullptr, "Error", "Failed to open " + zipPath, QMessageBox::Ok);
        return;
    }
    QStringList filePaths;
    for (bool b = zipArchive.goToFirstFile(); b; b = zipArchive.goToNextFile())
    {
        QString temp = zipArchive.getCurrentFileName();
        if (temp.endsWith('/')) dir.mkpath(newPath + QDir::separator() + temp);
        else filePaths.append(temp);
    }
    foreach (QString filePath, filePaths)
    {
        QFile file(newPath + QDir::separator() + filePath);
        if (zipArchive.setCurrentFile(filePath))
        {
            if (file.open(QIODevice::WriteOnly))
            {
                QuaZipFile zipFile(&zipArchive);
                if (zipFile.open(QIODevice::ReadOnly))
                {
                    const qint64 chunkSize = 4 * 1024 * 1024; // 4MiB
                    QByteArray buffer;
                    buffer.resize(chunkSize);
                    qint64 bytesRead;
                    while ((bytesRead = zipFile.read(buffer.data(), chunkSize)) > 0)
                    {
                        file.write(buffer.data(), bytesRead);
                    }
                    file.close();
                }
                zipFile.close();
            }
        }
    }
    zipArchive.close();
}

void mainWindow::properties ()
{
    QStringList list;
    foreach(const QModelIndex &index, ui->listViewFiles->selectionModel()->selectedIndexes())list.append(getFilePath(index));
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

QString mainWindow::getAvailableName(QString path, QString suffix)
{
    int i = 0;
    QString newPath = path + suffix;
    while (true)
    {
        if (!QFileInfo::exists(newPath))
        {
            break;
        }
        else
        {
            i++;
            newPath = path + " (" + QString::number(i) + ")" + suffix;
        }
    }
    return newPath;
}
