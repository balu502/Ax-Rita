#ifndef HUNTERDIALOG_H
#define HUNTERDIALOG_H

#include <QObject>
#include <QDialogButtonBox>
#include <QDialog>
#include <QTableWidget>
#include <QTreeWidget>


QT_BEGIN_NAMESPACE
class QLabel;
class QLineEdit;
class QPushButton;
QT_END_NAMESPACE


class HunterDialog : public QObject{
//////////////////////////////////////////////////////////////
    Q_OBJECT

public:
    HunterDialog(QWidget *t, QObject *parent = 0);

    QDialog      dialog;
    QWidget     *targetWidget;

    QLabel      *labelPrompt;
    QLineEdit   *searchText;
    QPushButton *searchButton;

    QString      lastSearch;

public slots:
    virtual void search();
};


class HunterTable : public HunterDialog{
//////////////////////////////////////////////////////////////
    Q_OBJECT

public:
    HunterTable(QTableWidget *t, QObject *parent = 0);

    QTableWidget* target(){ return qobject_cast< QTableWidget * >( targetWidget ); }

    QList<QTableWidgetItem *> resultList;
    QTableWidgetItem         *cursor;

public slots:
    void search();
};


class HunterTree : public HunterDialog{
//////////////////////////////////////////////////////////////
    Q_OBJECT

public:
    HunterTree(QTreeWidget *t, QObject *parent = 0);

    QTreeWidget* target(){ return qobject_cast< QTreeWidget * >( targetWidget ); }

    QList<QTreeWidgetItem *> resultList;
    QTreeWidgetItem         *cursor;

public slots:
    void search();
};

#endif // HUNTERDIALOG_H
