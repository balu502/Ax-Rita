#include "listchoosewindow.h"
#include "hunterdialog.h"

#include <QMessageBox>
#include <QGridLayout>
#include <QAction>
#include <QToolTip>

QTreeWidgetItem* listChooseWindow::newItem(QTreeWidgetItem *wix){

    QTreeWidgetItem *wi =
            (wix) ? wix : new QTreeWidgetItem( listTree );

    if (wi->text(0).isEmpty())
        wi->setText( 0, tr("New %1").arg(listTree->headerItem()->text(0) ) );

    wi->setFlags( wi->flags() | Qt::ItemIsEditable );

    listTree->addTopLevelItem( wi );
    listTree->scrollToItem(wi);

    emit itemInserted(wi);

    dialog.show();

    return wi;
}


void listChooseWindow::removeItems(){

    if ( listTree->selectedItems().count()
    &&   (QMessageBox::Yes == QMessageBox::question(
              0, tr("Delete %1")
                  .arg(listTree->headerItem()->text(0)),
              tr("Realy delete %1 item(s)?")
                  .arg(listTree->selectedItems().count()),
                  QMessageBox::Yes|QMessageBox::No))){

        foreach( QTreeWidgetItem *wi, listTree->selectedItems() )
            emit itemRemoved( listTree->takeTopLevelItem(
                                listTree->indexOfTopLevelItem( wi ) ),
                              listTree );
    }
}


listChooseWindow::listChooseWindow(
        QString                          dialogTitle,
        StringRecords                    fillList,
        QAbstractItemView::SelectionMode sMode,
        QObject                          *parent,
        bool                             isListEditable
        )
    : QObject(parent)
    , listTree      ( new QTreeWidget )
    , addButton     ( new QPushButton(tr("&Add")) )
    , delButton     ( new QPushButton(tr("&Delete")) )
    , buttonBox     ( new QDialogButtonBox ){

    listTree->setSelectionMode( sMode );

    buttonBox->addButton( QDialogButtonBox::Ok      );
    buttonBox->addButton( QDialogButtonBox::Cancel  );

    okButton    = buttonBox->button(QDialogButtonBox::Ok);
    cancelButton= buttonBox->button(QDialogButtonBox::Cancel);

    okButton->setText(     tr("Apply") );
    cancelButton->setText( tr("Cancel"));

    okButton->setDefault(true);

    if (isListEditable){
    buttonBox->addButton(addButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(delButton, QDialogButtonBox::ActionRole);
    }

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget( listTree,    0, 0 );
    mainLayout->addWidget( buttonBox,   1, 0 );

    dialog.setWindowTitle( dialogTitle );
    dialog.setLayout(mainLayout);
    dialog.setModal(true);
    dialog.setWindowFlags(  Qt::Dialog
                          | Qt::CustomizeWindowHint
                          | Qt::WindowTitleHint
                          | Qt::WindowCloseButtonHint
                          | Qt::WindowStaysOnTopHint
                          | Qt::WindowMaximizeButtonHint
                          );


    QAction         *acnFind = new QAction(listTree);
    dialog.addAction(acnFind );
                     acnFind->setShortcut( tr("Ctrl+F") );
             connect(acnFind, SIGNAL(triggered(bool)), this, SLOT(findItem()));


    if ( !fillList.empty() ){
        listTree->setHeaderLabels( fillList.first() );
        fillList.removeFirst();
    }else{
        delButton->setEnabled( false );
        okButton->setEnabled(  false );
    }

    foreach( QStringList rec, fillList ){
        QTreeWidgetItem *wi = new QTreeWidgetItem( listTree, rec );
        listTree->addTopLevelItem( wi );

        if ( isListEditable )
             wi->setFlags( wi->flags() | Qt::ItemIsEditable );
    }

    listTree->resizeColumnToContents(0);

    connect(okButton,  SIGNAL(pressed()),  &dialog, SLOT(accept()));
    connect(delButton, SIGNAL(pressed()),  this,    SLOT(removeItems()));
    connect(addButton, SIGNAL(pressed()),  this,    SLOT(newItem()));
    connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));
    connect(listTree,  SIGNAL(itemSelectionChanged()), this, SLOT(calcSelectedCount()));
}


void listChooseWindow::findItem(){

    HunterTree(listTree, this).dialog.exec();
}


void listChooseWindow::calcSelectedCount(){

    listTree->setToolTip(
            tr("%1 item(s) selected").arg(
                    listTree->selectedItems().count() ) );

    QToolTip::showText( listTree->mapToGlobal( QPoint( 0, 0 ) ), listTree->toolTip() );

}
