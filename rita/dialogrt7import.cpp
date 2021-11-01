#include "dialogrt7import.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QAxFactory>

#include <QSettings>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QGridLayout>
#include <QLabel>
#include <QDebug>

int countCheckedChilds(QTreeWidgetItem* wi);

DialogRt7Import::DialogRt7Import(
        QString dialogTitle,
        QString dialogCaption,
        QStringList headerLabels,
        QObject *parent
        )
: listChooseWindow(
        dialogTitle, StringRecords(),
        QAbstractItemView::NoSelection,
        parent, false )
{
    listTree->setHeaderLabels( headerLabels );
    listTree->setExpandsOnDoubleClick(false);

    connect( listTree, SIGNAL(itemClicked(QTreeWidgetItem*,int))
             , this,     SLOT(itemToggle(QTreeWidgetItem*,int)));

    connect( listTree, SIGNAL(itemChanged(QTreeWidgetItem*,int))
             , this,     SLOT(itemToggle(QTreeWidgetItem*,int)));

    QGridLayout *mLayout =
            qobject_cast< QGridLayout * >( dialog.layout() );

    QLabel* caption = new QLabel(dialogCaption);

    mLayout->removeWidget(listTree);
    mLayout->removeWidget(buttonBox);

    mLayout->addWidget( caption,    0, 0 );
    mLayout->addWidget( listTree,   1, 0 );
    mLayout->addWidget( buttonBox,  2, 0 );

            okButton->disconnect();
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(applyImport()));
}


void DialogRt7Import::itemToggle(QTreeWidgetItem *wi, int c){

    Qt::CheckState cs = (wi->checkState(0) == Qt::Unchecked)
                                    ? Qt::Checked : Qt::Unchecked;
    listTree->blockSignals(true);

    wi->setCheckState(0, cs);

    while (   wi = wi->parent() ){
        int  chc = 0;
        for (int c=0; c < wi->childCount(); c++)
            chc  += wi->child( c )->checkState( 0 ) != Qt::Unchecked;

        wi->setCheckState( 0,
            (chc == wi->childCount())
            ?   Qt::Checked : ( chc ? Qt::PartiallyChecked : Qt::Unchecked ));
    }

    listTree->blockSignals(false);
}



void DialogRt7Import::itemDoubleClicked(QTreeWidgetItem* wi,int c){

    if (!wi->childCount())
        return;

    Qt::CheckState ch = countCheckedChilds(wi)
                        ? Qt::Unchecked : Qt::Checked;

    for( int c=0; c < wi->childCount(); c++ )
        wi->child(c)->setCheckState( 0, ch );
}


QString DialogRt7Import::searchRealTimePCR(){

    QSettings m("HKEY_CLASSES_ROOT\\RealTime_192_App\\shell\\open\\command",
                QSettings::NativeFormat );

    QString vs = m.value(".").toString();

    if ( vs.isEmpty() )
        return "";

    vs = vs.left( vs.length() - QString(" \"%1\"").length() );

    return vs;
}


void DialogRt7Import::toggleCheck(bool on){

    QTreeWidgetItem *wi;
    Qt::CheckState  qc = on ? Qt::Checked : Qt::Unchecked;

    for( int i=0; i < listTree->topLevelItemCount(); i++ ){

        wi = listTree->topLevelItem( i );

        for( int c=0; c < wi->childCount(); c++ )
            wi->child(c)->setCheckState( 0, qc );
    }
}


void DialogRt7Import::setRiTUsersTests(RiTUserTests sts){
    userTests = sts;
}


int DialogRt7Import::exec(){

    loadRealTimePCREntries();
    listTree->expandAll();
    listTree->resizeColumnToContents(0);
    listTree->resizeColumnToContents(1);

    dialog.setGeometry(
        qApp->desktop()->geometry().width() /2 - 300,
        qApp->desktop()->geometry().height()/2 - 200,  600, 400 );

    return dialog.exec();
}


QStringList DialogRt7Import::loadRealTimePCREntries(){

    QString pfx= "TestINFO_";
    QString ptx= ".ini";
    QDir  rtDir= QFileInfo( searchRealTimePCR() ).absoluteDir();
    QStringList etrs = rtDir.entryList( QStringList() << pfx+"*"+ptx );
    QStringList ops;
    QString     usn;
    QTreeWidgetItem *wiu, *wit;
    QFont   boldFont;
            boldFont.setBold(true);

    foreach( QString en, etrs ){
        usn = en;
        usn.remove(0, pfx.length());
        usn.chop(     ptx.length() );

        bool userExists = userTests.contains( usn );

        //if ( !users.values().contains( usn ) ){

        QString ufPth = QString("%1/%2").arg(rtDir.absolutePath()).arg(en);
        wiu = new QTreeWidgetItem( listTree, QStringList() << usn );
        wiu->setData(0, Qt::UserRole,   ufPth);
        wiu->setData(0, Qt::UserRole+1, usn  );
        wiu->setText(2, tr( userExists? "Already exists": "Will be created" ));
        wiu->setCheckState( 0, userExists? Qt::Unchecked : Qt::Checked  );

        listTree->addTopLevelItem( wiu );

        QHash< QString, RiTEntryCache* > uinIndexedCache;

        if ( userExists )
            foreach (RiTEntryCache* ie, userTests[ usn ])
                uinIndexedCache[ ie->uinIncome() ] = ie;

        QSettings uss( ufPth, QSettings::IniFormat );

        foreach (QString t, uss.childGroups()){
            ops << QString("%1 - %2").arg( usn ).arg( t );

            QStringList testDef = t.split(" ");

            wiu->addChild( wit = new QTreeWidgetItem( testDef ) );
            wit->setData(0, Qt::UserRole,   testDef[0] );
            wit->setData(0, Qt::UserRole+1, t );

            if( !userExists || !uinIndexedCache.contains( testDef[0] ) ){
                    wit->setCheckState( 0, Qt::Checked  );
                    wit->setText(2, tr("For import"));

            }else{  wit->setCheckState( 0, Qt::Unchecked );
                    wit->setText(2, tr("Replace"));
                    wit->setFont(0, boldFont);
            }
        }
    }

    okButton->setEnabled(ops.count());

    return  ops;
}

int countCheckedChilds(QTreeWidgetItem* wi){
    int cc = 0;
    for( int c=0; c < wi->childCount(); c++ )
        cc += wi->child(c)->checkState(0) == Qt::Checked;

    return cc;
}

void DialogRt7Import::applyImport(){

    okButton->setEnabled(false);
    qApp->processEvents();

    QString     targetDir= "conv";
    QString     toolp    = "InConverterConsole.exe";
    ListTWItems exItems  = extractInifiles( targetDir );
    QString     fXml;
    QFileInfo   fi;
    QProcess    conv;

    toggleCheck(false);

    foreach (QTreeWidgetItem *wi, exItems){

        fi.setFile( wi->data(0, Qt::UserRole+2).toString() );
        fXml = QString("%1/%2.xml").arg( fi.absolutePath() ).arg( fi.baseName() );

        QFile(fXml).remove();

        conv.start( toolp, QStringList()
            << "conv" << fi.filePath() << fXml );

        if (!conv.waitForStarted(5000)){
            wi->setData(0, Qt::UserRole+3, "Unable to start converter" );
            wi->setText(2, tr("Unable to start converter"));

        }else{

            conv.write( "\r\n" );

            if (!conv.waitForFinished(2000)){

                //qDebug() << "Converter faulted";
                wi->setData(0, Qt::UserRole+3, "Converter faulted" );
                wi->setText(2, tr("Converter faulted"));

            }else{
                //qDebug() << "Converter successed";
                wi->setData(0, Qt::UserRole+3, fXml);
                wi->setText(2, tr("Converter successed"));
            }
        }

        qDebug() << wi->text(2);

        emit importCompleted( wi );
    }
}



ListTWItems DialogRt7Import::extractInifiles(QString xdirName){

    ListTWItems ret, unzd;
    QTreeWidgetItem *wi, *wit;
    QString     unm, upth, opth, tst;
    QSettings   *oss;
    QDir        convDir( QAxFactory::serverDirPath() );

    convDir.mkdir( xdirName );
    convDir.cd( xdirName );

    for( int i=0; i < listTree->topLevelItemCount(); i++ ){

        wi = listTree->topLevelItem( i );

        if ((wi->checkState(0) == Qt::Unchecked)
        && !countCheckedChilds( wi )){

            unzd << wi;
            continue;
        }

//        if (!countCheckedChilds( wi )){
//            unzd << wi;
//            continue;
//        }

        upth = wi->data(0, Qt::UserRole  ).toString();
        unm  = wi->data(0, Qt::UserRole+1).toString();
        opth = QString("%1/%2.ini")
                    .arg( convDir.absolutePath() ).arg( unm );
        ret <<  wi;
                wi->setData(0, Qt::UserRole+2, opth);

        QFile( opth ).remove();
        QFile( upth ).copy( opth );

        oss = new QSettings( opth,  QSettings::IniFormat );

        for( int c=0; c < wi->childCount(); c++ ){

            wit = wi->child(c);

            if (wit->checkState(0) == Qt::Checked)
                continue;

            tst = wit->data(0, Qt::UserRole+1).toString();
            oss->remove( tst );
            unzd << wit;
        }

        oss->sync();
        oss->deleteLater();
    }

    clearTWItems( unzd );

    return  ret;
}


void DialogRt7Import::clearTWItems( ListTWItems remList ){

    ListTWItems upar;
    QTreeWidgetItem *wi;

    while( remList.count() )
        if ( (wi = remList.takeLast())->childCount() )
              upar << wi;
        else  wi->parent()->removeChild( wi );

    while( upar.count() )
        if( wi = upar.takeLast() )
            delete listTree->takeTopLevelItem(
                    listTree->indexOfTopLevelItem( wi ) );
}
