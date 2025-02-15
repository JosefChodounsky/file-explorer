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
    QFileSystemModel *fileModel;
    Ui::mainWindow *ui;

private slots:
    void on_btnCreateDir_clicked();
    void on_listViewFiles_doubleClicked();
    void on_btnGoUp_clicked();
};
#endif // MAINWINDOW_H
