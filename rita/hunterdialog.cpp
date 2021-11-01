#include "hunterdialog.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDebug>
#include <QApplication>
#include <QAction>


HunterDialog::HunterDialog(QWidget *t, QObject *parent)
    : targetWidget(t)
    , QObject(parent)
{

    labelPrompt = new QLabel(tr("Search:"));
    searchText  = new QLineEdit();

    (searchButton = new QPushButton())->setIcon( QIcon(":/images/search.png"));

    connect(searchButton, SIGNAL(clicked(bool)), this, SLOT(search()));

    QAction *acnSearchFwd = new QAction(&dialog);
             acnSearchFwd->setShortcut(  QKeySequence::FindNext );

    QAction *acnSearchBack = new QAction(&dialog);
             acnSearchBack->setShortcut( QKeySequence::FindPrevious );

    connect( acnSearchFwd,  SIGNAL(triggered(bool)), this, SLOT(search()));
    connect( acnSearchBack, SIGNAL(triggered(bool)), this, SLOT(search()));

    searchButton->addAction( acnSearchFwd  );
    searchButton->addAction( acnSearchBack );

    QGridLayout *mainLayout = new QGridLayout;

    mainLayout->addWidget(labelPrompt,  0, 0);
    mainLayout->addWidget(searchText,   0, 1);
    mainLayout->addWidget(searchButton, 0, 2);

    dialog.setLayout(mainLayout);
    dialog.setModal(true);
//    dialog.exec();
}


void HunterDialog::search(){

    qDebug() << "search base";
}


HunterTable::HunterTable(QTableWidget *t, QObject *parent)
    : HunterDialog((QWidget*)t, parent){
//    qDebug() << "derived";
}

void HunterTable::search(){

    target()->clearSelection();

    if (lastSearch != searchText->text()){
        resultList = target()->findItems(
                        (lastSearch = searchText->text())
                        , Qt::MatchContains );
        cursor = 0;
    }

    int iNext;

    if ( resultList.count() ){

        iNext  =  1 + resultList.indexOf( cursor );
        iNext *=  iNext < resultList.count();

        (cursor = resultList[iNext])->setSelected(true);

    }

//    qDebug() << "Found:" << resultList.count();

}


HunterTree::HunterTree(QTreeWidget *t, QObject *parent)
    : HunterDialog((QWidget*)t, parent){
//    qDebug() << "derived";
}


void HunterTree::search(){

    target()->clearSelection();
    QString currentSearch = searchText->text().trimmed();

    if (lastSearch != currentSearch){

        resultList.clear();
        lastSearch= currentSearch;
        cursor    = 0;

        for (int c=0; c < target()->columnCount(); c++ )
            foreach (QTreeWidgetItem* i, target()->findItems(lastSearch, Qt::MatchRecursive|Qt::MatchContains, c))
                if (!resultList.contains(i)) resultList << i;
    }

    int iNext;

    if ( resultList.count() ){

        if ((QApplication::keyboardModifiers() == Qt::ControlModifier)
        ||  (QApplication::keyboardModifiers() == Qt::ShiftModifier)){

            iNext  = -1 + resultList.indexOf( cursor );
            iNext  = (iNext < 0) ? resultList.count() - 1: iNext;

        }else{
            iNext  =  1 + resultList.indexOf( cursor );
            iNext *=  iNext < resultList.count();
        }

        (cursor = resultList[iNext])->setSelected(true);
    }
}
