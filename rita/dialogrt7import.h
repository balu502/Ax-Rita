#ifndef DIALOGRT7IMPORT_H
#define DIALOGRT7IMPORT_H

#include <QWidget>
#include "listchoosewindow.h"
#include "rita.h"

typedef QList< QTreeWidgetItem* > ListTWItems;

class DialogRt7Import : public listChooseWindow
{
    Q_OBJECT

    QStringList     loadRealTimePCREntries();
    ListTWItems     extractInifiles(QString xdirName);
    void            clearTWItems(ListTWItems remList);

    RiTUserTests userTests;

public:
    DialogRt7Import(QString dialogTitle,
        QString     dialogCaption,
        QStringList headerLabels = QStringList(),
        QObject     *parent = 0 );

    QString searchRealTimePCR();
    void    setRiTUsersTests( RiTUserTests );
    int     exec();

private slots:
    void applyImport();
    void toggleCheck(bool on);
    void itemDoubleClicked(QTreeWidgetItem *wi, int c);

    void itemToggle(QTreeWidgetItem *wi, int c);
signals:
    void importCompleted(QTreeWidgetItem*);
};

#endif // DIALOGRT7IMPORT_H
