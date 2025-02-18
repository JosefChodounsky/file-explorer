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
    void properties();
    bool pasteRecursively(QString source, QString destination);
    int conflict(QString d);
    QFileSystemModel *fileModel;
    Ui::mainWindow *ui;

private slots:
    void on_btnCreateDir_clicked();
    void on_listViewFiles_doubleClicked();
    void on_btnGoUp_clicked();
    void on_listWidgetDrives_doubleClicked();
    //void keyPressEvent(QKeyEvent *e);
    void showContextMenu(const QPoint &pos);
};
#endif // MAINWINDOW_H
