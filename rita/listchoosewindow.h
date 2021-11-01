#ifndef LISTCHOOSEWINDOW_H
#define LISTCHOOSEWINDOW_H

#include <QWidget>
#include <QObject>
#include <QTreeWidget>
#include <QDialogButtonBox>
#include <QDialog>
#include <QPushButton>

typedef QList< QStringList > StringRecords;

class listChooseWindow : public QObject
{
    Q_OBJECT

public:

    listChooseWindow(
            QString         dialogTitle,
            StringRecords   fillList,
            QAbstractItemView::SelectionMode sMode,
            QObject         *parent = 0,
            bool            isListEditable = true );

    QList<QVariant>     retFields;
    QTreeWidget         *listTree;
    QDialogButtonBox    *buttonBox;
    virtual int         exec(){ return dialog.exec(); }
    QDialog             dialog;


public slots:
    void findItem();

protected:
    QPushButton         *addButton;
    QPushButton         *delButton;
    QPushButton         *okButton;
    QPushButton         *cancelButton;


protected slots:
    virtual QTreeWidgetItem* newItem(QTreeWidgetItem *wix=0 );
    virtual void             removeItems();
    virtual void             calcSelectedCount();

signals:
    void itemInserted(QTreeWidgetItem *wi);
    void itemRemoved (QTreeWidgetItem *wi, QTreeWidget *tw );
};


#endif // LISTCHOOSEWINDOW_H
