#include "propertiesdialog.h"

propertiesDialog::propertiesDialog(const QString &path, QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Properties");
    QLabel *nameLabel = new QLabel("Name: ");
    QLabel *kindLabel = new QLabel("Kind: ");
    QLabel *locationLabel = new QLabel("Location: ");
    QLabel *createdLabel = new QLabel("Created: ");
    QLabel *modifiedLabel = new QLabel("Modified: ");
    QLabel *accessedLabel = new QLabel("Accessed: ");
    QLabel *permissionsLabel = new QLabel("Permissions: ");
    QLabel *sizeLabel = new QLabel("Size: ");

    QStringList list = getValues(path);
    QLabel *nameValue = new QLabel(list[0]);
    QLabel *kindValue = new QLabel(list[1]);
    QLabel *locationValue = new QLabel(list[2]);
    QLabel *createdValue = new QLabel(list[3]);
    QLabel *modifiedValue = new QLabel(list[4]);
    QLabel *accessedValue = new QLabel(list[5]);
    QLabel *permissionsValue = new QLabel(list[6]);
    QLabel *sizeValue = new QLabel(list[7]);

    QVBoxLayout *labelsLayout = new QVBoxLayout;
    labelsLayout->addWidget(nameLabel);
    labelsLayout->addWidget(kindLabel);
    labelsLayout->addWidget(locationLabel);
    labelsLayout->addWidget(createdLabel);
    labelsLayout->addWidget(modifiedLabel);
    labelsLayout->addWidget(accessedLabel);
    labelsLayout->addWidget(permissionsLabel);
    labelsLayout->addWidget(sizeLabel);

    QVBoxLayout *valuesLayout = new QVBoxLayout;
    valuesLayout->addWidget(nameValue);
    valuesLayout->addWidget(kindValue);
    valuesLayout->addWidget(locationValue);
    valuesLayout->addWidget(createdValue);
    valuesLayout->addWidget(modifiedValue);
    valuesLayout->addWidget(accessedValue);
    valuesLayout->addWidget(permissionsValue);
    valuesLayout->addWidget(sizeValue);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addLayout(labelsLayout);
    mainLayout->addLayout(valuesLayout);

    setLayout(mainLayout);
}

QStringList propertiesDialog::getValues(const QString &path)
{
    QFileInfo info(path);
    QMimeDatabase database;
    QMimeType mimeType = database.mimeTypeForFile(info);
    QString timeCreated;
    QStringList result;
    if (info.birthTime().isValid()) timeCreated = info.birthTime().toString(Qt::ISODate);
    else timeCreated = "Unknown";
    result.append(info.fileName());
    result.append(mimeType.comment());
    result.append(info.absolutePath());
    result.append(timeCreated);
    result.append(info.lastModified().toString(Qt::ISODate));
    result.append(info.lastRead().toString(Qt::ISODate));
    result.append(permissions(info));
    result.append(QString::number(info.size()) + " Bytes");
    return result;
}

QString propertiesDialog::permissions(QFileInfo &info)
{
    QString output = "";
    if (info.permission(QFile::ReadOwner))output += "r";
    else output += "-";
    if (info.permission(QFile::WriteOwner))output += "w";
    else output += "-";
    if (info.permission(QFile::ExeOwner))output += "x";
    else output += "-";
    if (info.permission(QFile::ReadGroup))output += "r";
    else output += "-";
    if (info.permission(QFile::WriteGroup))output += "w";
    else output += "-";
    if (info.permission(QFile::ExeGroup))output += "x";
    else output += "-";
    if (info.permission(QFile::ReadOther))output += "r";
    else output += "-";
    if (info.permission(QFile::WriteOther))output += "w";
    else output += "-";
    if (info.permission(QFile::ExeOther))output += "x";
    else output += "-";
    return output;
}
