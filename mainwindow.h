#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QListWidget>
#include <QListView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStorageInfo>
#include <QFileSystemModel>
#include <QMouseEvent>
#include <QAction>
#include <QMenu>
#include <QInputDialog>
#include <QMessageBox>
#include <QShortcut>
#include <QDesktopServices>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QDirIterator>

#include <QuaZip-Qt6-1.4/quazip/quazip.h>
#include <QuaZip-Qt6-1.4/quazip/quazipfile.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class mainWindow;
}
QT_END_NAMESPACE

class mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit mainWindow(QWidget *parent = nullptr);
    ~mainWindow();

private:
    void listDrives(QListWidget *listWidget);
    void setupFileListView();
    void open();
    void copy();
    void cut();
    void paste();
    void rename();
    void remove();
    void del(QString d);
    void zip();
    void addToZip(const QString &filePath, const QString &parentDir, QuaZip *zipArchive);
    void unzip();
    void properties();
    bool pasteRecursively(QString source, QString destination);
    bool isQFileSystemModel();
    void showContextMenu(const QPoint &pos);
    QString permissions(QFileInfo &info);
    int conflict(QString &d);
    int isArchive();
    QFileSystemModel *fileModel;
    Ui::mainWindow *ui;
    QString currentDir = QDir::homePath();
    QStringList clipboard;
    bool mv = false;
    QString getFilePath(const QModelIndex &index);
    void performSearch(const QString& keyword);
    void resetFileView();
    QFileInfoList findFiles(const QString& startDir, const QString& keyword);

private slots:
    void on_btnCreateDir_clicked();
    void on_listViewFiles_doubleClicked();
    void on_btnGoUp_clicked();
    void on_listWidgetDrives_doubleClicked();
};
#endif // MAINWINDOW_H
