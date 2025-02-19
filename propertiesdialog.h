#ifndef PROPERTIESDIALOG_H
#define PROPERTIESDIALOG_H

#include <QDialog>
#include <QObject>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFileInfo>
#include <QMimeDatabase>

class propertiesDialog : public QDialog
{
    Q_OBJECT
public:
    propertiesDialog(const QString &path, QWidget *parent = nullptr);
private:
    QString permissions(QFileInfo &info);
    QStringList getValues(const QString &path);
};

#endif // PROPERTIESDIALOG_H
