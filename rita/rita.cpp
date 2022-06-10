#include "rita.h"
#include <QApplication>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QListWidget>
#include <QTreeWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QTabWidget>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QStatusBar>
#include <QInputDialog>
#include <QRegExp>
#include <QException>
#include <QFileDialog>
#include <QDomDocument>
#include <QTextStream>
#include <QSettings>
#include <QStyleFactory>
#include <QStandardPaths>
#include <QLibraryInfo>


#include <my_config.h>
#include <my_global.h>
#include <mysql.h>

#include "listchoosewindow.h"
#include "dialogrt7import.h"
#include "newaccountdialog.h"

#include <QAxFactory>

//#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <psapi.h>
#pragma comment( lib, "psapi.lib" )


#define  TC_MUSERS  -1
#define  TC_EFILE   -3
#define  TC_REMOVE  -2

#define  SC_IFILE   -1

#define SR_DATA 0+Qt::UserRole
#define SR_ACID 1+Qt::UserRole
#define SR_USR  2+Qt::UserRole


#define base64Encode(s) QString().append( QByteArray().append(s).toBase64() )
#define base64Decode(s) QByteArray::fromBase64( s ).data()

#define BEIX(i,t,o) (QString("%1_%2_%3").arg(i).arg(t).arg(o))
#define CQSTR(s)    s.toStdString().c_str()

#define CINFO(i) QMessageBox::information(NULL, tr("Info"), i)

MYSQL       *mysql = 0;
MYSQL_RES   *results;
MYSQL_ROW   record;
//QRegExp     rxNamePass("[\\da-zA-Z_][\\da-zA-Z_]*");
QRegExp     rxNamePass("[\\da-zA-Z_]*");

Qt::WindowFlags WNORMFLAGS
            = Qt::Window;

Qt::WindowFlags WTOPFLAGS
            = Qt::Dialog
            | Qt::CustomizeWindowHint
            | Qt::WindowTitleHint
            | Qt::WindowCloseButtonHint
            | Qt::WindowStaysOnTopHint
            ;

Qt::WindowFlags WSPLASHFLAGS
            = Qt::SplashScreen
            | Qt::CustomizeWindowHint
            | Qt::WindowStaysOnTopHint
            ;

typedef QHash< QString, QString >   StringHash;

int ritAppCounter = 0;

QHash <qint64, QDateTime> appHeartbeat;

char op0[] = "mysql_test";
char op1[] = "--defaults-file=my.ini";

char gp0[] = "client";
char gp1[] = "mysqld";

static char *server_options[]= { op0, op1, NULL };
int             num_elements = (sizeof(server_options) / sizeof(char *)) - 1;
static char *server_groups[] = { gp0, gp1, NULL };

/*
static char *server_options[]= { "mysql_test", "--defaults-file=my.ini", NULL };
int             num_elements = (sizeof(server_options) / sizeof(char *)) - 1;
static char *server_groups[] = { "client", "mysqld" "mysql", NULL };
*/


QString str_escape( QString v ){

    QString ret;
    QChar   bs('\'');

    for( int c=0; c<v.length(); c++ ){
        ret += v[c];
        if (v[c] == bs)
            ret  += bs;
    }

    return  ret;
}


bool string_to_file( QString saveText, QString path ){

    QFile fileO( path );

    if ( !fileO.open( QIODevice::WriteOnly | QIODevice::Truncate ) )
        return false;

    QTextStream streamO( &fileO );

    foreach( QString s, saveText.split("\n") )
        streamO << s << "\n";

    fileO.close();

    return  true;
}


QString file_to_string( QString path ){

    QFile file( path );

    if (!file.open(QIODevice::ReadOnly))
        return "";

    QString rs =file.readAll();
                file.close();
    return  rs;
}


//RiTApplication::RiTApplication(QWidget *parent)
//    : QWidget   (parent)

RiTApplication::RiTApplication(QObject *parent)
    : QObject   (parent)
    , dialog        (0)
    , isDBConnected (0)
    , appIsAxServer (false)
    , _currentLogin (0)
    , _modifiedLogin(0)
    , muDialog      (0)
    , pmpDialog     (0)
    , cxName        (0)
    , edPwd1        (0)
    , edPwd2        (0)
    , hbTimer       (0)
    , pidClient     (0)
    {
//    , ritaTranslator(new QTranslator()){

    app     = qApp;
    baseDir = QAxFactory::serverDirPath();

    ritAppCounter++;
    setObjectName( "RITA_from_QAxFactory" );

    initSettings();
}


bool RiTApplication::startHeartBeat( qlonglong pidC, int rate ){

    if (!pidC)      return false;
    if (rate<0)     return false;
    if (!pidClient) pidClient = pidC;
    if (!clientProcessExists()){ return pidClient=0; }
    if (hbTimer)    killTimer( hbTimer );

    hbTimer = startTimer( rate );

    QTimer::singleShot(rate/2, this, SLOT(triggerHeartBeat()));

    return  true;
}



void RiTApplication::timerEvent( QTimerEvent *event ){

    QDateTime cTime = QDateTime::currentDateTime();
    appHeartbeat[ pidClient ] = cTime;

    if (( lastClentTrigger.msecsTo(cTime) > 5000 )
    &&  !clientProcessExists()){

        killTimer( hbTimer );
                   hbTimer = 0;
        quit();
    }
}


bool RiTApplication::clientProcessExists(){

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
        return  true;


    cProcesses = cbNeeded / sizeof(DWORD);

    for ( i = 0; i < cProcesses; i++ )
        if( aProcesses[i] == pidClient )
            return true;

    return  false;
}


//void RiTApplication::timerEvent(QTimerEvent *event){

//    appHeartbeat[ (qint64) this ]
//            = QDateTime::currentDateTime();
//}


void RiTApplication::triggerHeartBeat(){
    lastClentTrigger = QDateTime::currentDateTime();
}


void RiTApplication::initStyles(){

    if (!dialog)
        return;

    QString fcss = QFile::exists( baseDir+"/rita.css" )
                ? file_to_string( baseDir+"/rita.css" )
                : file_to_string( ":/styles/rita.css" );

    if (fcss.length()){
        dialog->setStyleSheet( fcss );

        fcss = dialog->styleSheet();

        QVariant pp;

        if ((pp = property("fontcss")).isValid()){
            QString fDef = QString("QWidget { %1 }\n").arg( pp.toString() );
            fcss =  fDef + fcss;

            dialog->setStyleSheet( fcss );
            qApp->processEvents();

            //CINFO(fDef);
        }
    }

    dialog->setStyle(
        QStyleFactory::create("Fusion"));    
}


void RiTApplication::initSettings(){

//    CINFO( app->applicationName() );
//    setAppNameDomain();

    settings = new QSettings( QSettings::UserScope,
                    app->organizationName(),
                    app->applicationName() );
}


int RiTApplication::userCbxIndex(int userId){

    for ( int i=0; i < cbxUsers->count(); i++ )
        if ( userId == cbxUsers->itemData(i).toInt() )
            return i;

    return -1;
}


bool RiTApplication::setCurrentLogin(RiTUser *u, QString passwd){

    if (passwd.length())                        //TODO:
        _currentLogin = u;

    return true;
}


bool RiTUser::setRiTPriv(QString p, bool v){

    _priv[ p ] = v;
    return  true;
}


bool RiTUser::setRiTProp(QString p, QString v){

    _prop[ p ] = v;
    return  true;
}


bool RiTUser::setRiTEntry(int e, QString u, QString t, QString c, int pa, int p, int ow){

    RiTEntryCache*
        entry = new RiTEntryCache( this );
        entry->setRiTEntry(e,u,t,c,pa,p,ow);

    QString x = BEIX(u,t,ow);

    _cache[  _indexCache[ x ] = e  ] = entry;

    return  true;
}


RiSplashInfo::RiSplashInfo(QWidget *parent, QString msg, int timeoutClose)
    : QDialog( parent,  WSPLASHFLAGS )
    , tClose(  timeoutClose  ){

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    label = new QLabel( msg, this );
    label->setFrameShape(  QFrame::Box     );
    label->setAlignment(   Qt::AlignCenter );

    verticalLayout->addWidget( label );
    resize(400, 80);
}


void RiSplashInfo::appendLabel(QString s){

    label->setText( label->text() + s );
    qApp->processEvents();
}


void RiSplashInfo::setWindowFlag( Qt::WindowType flag, bool fen ){
    Qt::WindowFlags flags = windowFlags() | flag & fen;
    setWindowFlags( flags );
}


void RiSplashInfo::show(){

    setWindowFlag(Qt::SplashScreen,         true);
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    QDialog::show();

    qApp->processEvents();

    if( tClose )
        QTimer::singleShot(tClose, this, SLOT(hide()));
}


int RiSplashInfo::exec(){

    if( tClose )
        QTimer::singleShot(tClose, this, SLOT(hide()));

    setWindowFlag(Qt::SplashScreen,         true);
    setWindowFlag(Qt::WindowStaysOnTopHint, true);
    return QDialog::exec();
}


void RiSplashInfo::hide(){

    setWindowFlag(Qt::SplashScreen,         false);
    setWindowFlag(Qt::WindowStaysOnTopHint, false);
    QDialog::hide();
}


bool RiTApplication::createBackup(
        QString toolp, QString sp, QString dp ){

    return 0 >= QProcess::execute(toolp,
                    QStringList() << "a" << dp << sp );
}


bool RiTApplication::dbConnect(){

    RiSplashInfo *wtDialog  =
        new RiSplashInfo( 0, tr("Initialise database."), 20000);

    wtDialog->show();
    wtDialog->appendLabel(".1");

    QDir bir( baseDir );
         bir.cdUp();

    QString ddata = bir.path() + "/data";
    QString toolp =    baseDir + "/7-Zip/7z.exe";

    if ( QFile::exists( ddata )
    &&   QFile::exists( toolp ) ){

        QString bpath = bir.path() + "/data.bak.zip";

        if (!QFile::exists( bpath )
        ||  (QFileInfo( bpath ).lastModified()
                < QDateTime::currentDateTime().addDays(-30) )){

            bool bOk = createBackup( toolp, ddata, bpath );

            qDebug() << "Create backup result:" << bOk;
            wtDialog->appendLabel(".");
        }

    } else
        QDir().mkpath(ddata+"/mysql");

    wtDialog->appendLabel(".2");

    if ( !QFile::exists("my.ini") ){

        QString myIniText = file_to_string(":/sql/my.ini" );
                myIniText = myIniText.replace( "%DATADIR%", ddata  );
                myIniText = myIniText.replace( "%BASEDIR%", bir.path() );

        string_to_file( myIniText, "my.ini" );
        wtDialog->appendLabel(".");
    }

    wtDialog->appendLabel(".3");

    bool isConnected = false;
    int cErr= mysql_library_init(num_elements, server_options, server_groups);
    mysql   = mysql_init(NULL);

    wtDialog->appendLabel(".4");

    if (cErr || (NULL == mysql)){
        wtDialog->hide();
        wtDialog->deleteLater();

        QMessageBox::warning(NULL, tr("Warning"),
            tr("Warning! Could not connect with database!")
            + tr("\n Databse driver fault (%1, %2)")
                    .arg(cErr).arg(NULL == mysql));

        return  false;
    }

    mysql_options(mysql, MYSQL_READ_DEFAULT_GROUP,  "client");
    mysql_options(mysql, MYSQL_OPT_USE_EMBEDDED_CONNECTION, NULL);

    wtDialog->appendLabel(".5");

    if (!(mysql = mysql_real_connect(
              mysql, NULL,NULL,NULL,
              "mysql", 0, NULL,
              CLIENT_MULTI_STATEMENTS ))){

        wtDialog->hide();
        wtDialog->deleteLater();

        QMessageBox::warning(NULL, tr("Warning"),
            tr("Warning! Could not connect with database!"));

    }else if (!dbChkExists()){

        dbBuild();

        if (!dbChkExists()){

            wtDialog->hide();
            QMessageBox::warning(NULL, tr("Warning"),
                tr("Warning! Could not access database!"));

        }else
            isConnected = true;
    }else   isConnected = true;


    emit afterDbConnected(isDBConnected = isConnected);

    if ( isDBConnected ){
        foreach( QString       k, ctmPriv.keys() )
            registerFPrivilege(k, ctmPriv[ k ]);

        ctmPriv.clear();
    }

    wtDialog->hide();
    wtDialog->deleteLater();

    return  isDBConnected;
}


bool RiTApplication::dbDisconnect(){

    if (mysql) mysql_close( mysql );

//    mysql_library_end();
    mysql = 0;

    return true;
}


bool RiTApplication::dbChkExists(){
    return  ( 0 == mysql_select_db(mysql, "rita") );
}


bool RiTApplication::dbBuild(){

    QString sqlCmd  =
        QFile::exists(      baseDir + "/rita_create.sql" )
          ? file_to_string( baseDir + "/rita_create.sql" )
          : file_to_string(      ":/sql/rita_create.sql" );

    qDebug() << "Starting database creating script";

    try{
        int wc = mysql_warning_count(mysql);
        int rs = mysql_query( mysql, CQSTR(sqlCmd) );

        if (wc != mysql_warning_count(mysql))
            qDebug() << mysql_error(mysql);

        qDebug() << "Database successfuly created";

    }catch( ... ){

        qWarning() << "Error! Cannot create database";
        return  false;
    }

    return true;
}


int RiTApplication::dbSelectUsers(){

//    cbxUsers->clear();
    users.clear();

    mysql_query(mysql, "SELECT idruser, name FROM ruser ");

    int ixf = -1;

    if ( results = mysql_store_result(mysql))
    while(record = mysql_fetch_row(results)) {

        ixf = QString(record[0]).toInt();

//        cbxUsers->insertItem(ixf, record[1], ixf );
        users.insert( ixf, record[1] );
    }


    mysql_free_result(results);

    emit usersSelected();

    return users.count();
}


void RiTApplication::onUsersSelected(){

    cbxUsers->clear();

    foreach(int uid, users.keys())
        cbxUsers->insertItem(uid,
                             (users[uid]=="guest")? tr("Guest"): users[uid],
                             //users[uid],
                             uid );


    int lastUId = settings->value("main/lastLoginId", 0).toInt();

    if ( -1 != (lastUId = userCbxIndex(lastUId)))
        cbxUsers->setCurrentIndex( lastUId );
    else
        if ( cbxUsers->count() )
             cbxUsers->setCurrentIndex( 0 );

}

QString substituteEnvPaths(QString str){

    QRegExp rx("(%(\\w|\\d)+%)");
    QStringList list;
    QString s = str;
    int pos = 0;

    while ((pos = rx.indexIn(str, pos)) != -1) {
        list << rx.cap(1);
        pos += rx.matchedLength();

//        qDebug() << "Found" << rx.cap(1);
    }

    foreach(QString en, list){

        QString ev = qgetenv( CQSTR(en.remove(QChar('%'))) );

        if (ev.length())
            str.replace( "%"+en+"%", ev );
    }

//    return  QFile::exists( str )
//            ? QFileInfo( str ).absoluteFilePath() : s ;
    return  str;
}

int RiTApplication::dbSelectProps(){

    props.clear();
    QString pn, pv;

    mysql_query(mysql, "SELECT prop, valdefault FROM ruprop");

    if ( results = mysql_store_result(mysql))
    while(record = mysql_fetch_row(results)){

        pn = record[0];
        pv = record[1];

        if( pv.contains('%')
        &&  (pn.startsWith("file:") || pn.startsWith("dir:")))
             props.insert( pn, substituteEnvPaths( pv ) );
        else props.insert( pn, pv );
    }

    mysql_free_result(results);
    return props.count();
}


PrivFields makePrivFields( int g, QString n, bool v ){
    PrivFields  rp;
                rp.idgroup  = g;
                rp.group    = n;
                rp.enabled  = v;
    return      rp;
}


bool RiTApplication::registerFPrivilege(
        const QString name, PrivFields p ){

    return registerPrivilege(name, p.enabled, p.idgroup, p.group);
}


bool RiTApplication::registerPPrivilege(
        const QString name,  bool valDefault) const {

    return registerPrivilege( name, valDefault, 1, "" );
}


bool RiTApplication::registerPrivilege(
        const QString   name,
        bool            valDefault,
        int             igp,
        const QString   gname   ) const  {

    if( !isDBConnected ){
        ctmPriv[ name ] = makePrivFields( igp, gname, valDefault );
        return true;
    }

    if ( !igp ) return false;

    QString qsqry = QString(
            " INSERT INTO rupriv(`priv`, `idgroupriv`, `valdefault`) "
            " VALUES('%1', %2, %3) "
            " ON DUPLICATE KEY UPDATE `valdefault` = %3 ")
            .arg(name).arg(igp).arg(valDefault);

    if ( gname.length() ){
         qsqry = QString(
            " INSERT INTO groupriv(`idgroupriv`, `grname`) "
            " VALUES(%1, '%2') "
            " ON DUPLICATE KEY UPDATE `grname` = %2 ")
            .arg( igp ).arg( gname );

         mysql_query( mysql, CQSTR(qsqry) );

         return registerPrivilege( name, valDefault, igp, "" );
    }

    mysql_query(  mysql, CQSTR(qsqry) );
    return  mysql_affected_rows( mysql );
}

QHash <QString, PrivFields>
            RiTApplication::ctmPriv;
StringHash  RiTApplication::trPriv;
StringHash  RiTApplication::trProp;


bool RiTApplication::trPrivilege(QString name, QString trans) const{

    trPriv[ name ] = trans;
    return  true;
}


bool RiTApplication::trProperty(QString name, QString trans) const{

    trProp[ name ] = trans;
    return  true;
}


int RiTApplication::dbSelectPrivs(){

    privs.clear();

    mysql_query(mysql,
                " SELECT priv, r.idgroupriv, grname, valdefault "
                " FROM rupriv r "
                " LEFT OUTER JOIN groupriv g "
                " ON r.idgroupriv = g.idgroupriv ");

    if ( results = mysql_store_result(mysql) )
    while(record = mysql_fetch_row(results))
        privs.insert(   record[ 0 ],
            makePrivFields(
                QString(record[ 1 ]).toInt()
            ,   QString(record[ 2 ])
            ,   QString(record[ 3 ]).toInt() ) );

    mysql_free_result( results );

    return privs.count();
}


bool RiTApplication::tryAutoLogIn(){

    return tryLogIn(true);
}


bool RiTApplication::tryLogIn(bool autoLogin){

    if (qApp->keyboardModifiers() & Qt::AltModifier ){
        openConsoleSQL();
        return false;
    }

    QString uName;      // = cbxUsers->currentText();
    QString uPwd = "";  //  = edPasswd->text();
    bool    sglOn     = privs[ "@SINGLE_LOGIN" ].enabled;
    bool    uAccepted = false;
    int     uid;

    if ( autoLogin && !dialog && sglOn ){
//    && (1 == users.count())){

        //uName = users.values().first();
        uName = users.values().contains("guest")
                ? "guest" : users.values().first();

//        uPwd  = "";
//        uName = settings->value( "main/lastLogin",
//                    users.values().first() ).toString();
    }else{

        if ( dialog ){
            //uName = cbxUsers->currentText();
            uName = users[ cbxUsers->currentData().toInt() ];
            uPwd  = edPasswd->text();
        } else
            return false;
    }

    if ( rxNamePass.exactMatch( uName+uPwd ) ){
        QString dr =
            QString(" SELECT idruser, name, pwd"
                    ", created, admin, 0"
                    ", pwd=%2 as chp "
                     " FROM ruser "
                     " WHERE name='%1'")
                    .arg( uName )
                    .arg( sglOn ? "pwd"
                                : QString("password('%1')").arg(uPwd) );

        mysql_query(mysql, CQSTR(dr) );

        if((results = mysql_store_result(mysql))
        && (record  = mysql_fetch_row(results))) {

            uid      = QString(record[0]).toInt();
            uAccepted= QString(record[6]).toInt();
//            uAccepted= privs[ "@SINGLE_LOGIN" ].enabled
//                    || QString(record[6]).toInt();

            (!uAccepted) && (!autoLogin)
            && RiSplashInfo( dialog,
                    tr("Please input correct password"),
                    2000 ).exec();

        } else RiSplashInfo( dialog,
                    tr("User '%1' does not exists").arg(uName),
                    2000 ).exec();

        mysql_free_result(results);

        uAccepted && dbReadUser( uid, &_currentLogin );

    }else RiSplashInfo( dialog,
            tr("Password is not accepted"), 2000 ).exec();

    emit afterLogIn( uAccepted );

    if ( uAccepted && _currentLogin ){

        emit afterAdminLogIn( _currentLogin->isAdmin() );

        settings->setValue("main/lastLoginId",  uid  );
        settings->setValue("main/lastLogin",    uName);

        if (dialog && appIsAxServer)
            dialog->accept();
    }else
        if( dialog ) {
            edPasswd->setFocus();
        }

    return  uAccepted;
}


void RiTApplication::openConsoleSQL(){

    QDialog     *sqlDialog= new QDialog(muDialog);
    QGridLayout *dLayout  = new QGridLayout(sqlDialog);
    QLineEdit   *edName   = new QLineEdit(sqlDialog);
    QListWidget *lsResult = new QListWidget(sqlDialog);

    dLayout->addWidget(edName,  0, 0);
    dLayout->addWidget(lsResult,1, 0);

    sqlDialog->setLayout( dLayout );
    sqlDialog->setObjectName("QDialog_sqlDialog");

    edName->setClearButtonEnabled(true);
    edName->setProperty("widget_results", (int) lsResult );

    connect(edName,  SIGNAL(returnPressed()), this, SLOT(runSqlCommand()));

    sqlDialog->exec();
}


void RiTApplication::runSqlCommand(){

    if (!sender())
        return;

    QLineEdit*   wLineEdit = qobject_cast<QLineEdit*>( sender() );
    QObject*     wTmp      = (QObject*) wLineEdit->property("widget_results").toInt();
    QListWidget* wListRes  = qobject_cast<QListWidget*>( wTmp );
    QString      cmdText   = wLineEdit->text().trimmed();


    wListRes->clear();
    wListRes->addItem( cmdText );
    wListRes->addItem( "---------------" );


    if (cmdText.toLower().startsWith("setting")){

        QStringList cms = cmdText.split(" ");

        if (cms.count() < 4){
            wListRes->addItem("Setting command Format - [settings <type> <name> <value>]");
            wListRes->addItem("<type> := int|real|string|bool");
            wListRes->addItem("<name> - setting's name ");
            wListRes->addItem("---------");

            QStringList snks = settings->allKeys();

            foreach(QString s, snks)
                wListRes->addItem( QString("%1 (%2) = %3").arg(s)
                                   .arg(settings->value(s).typeName())
                                   .arg(settings->value(s).toString()));
            return;
        }

        if (cms[1].toLower() == "int")    settings->setValue(cms[2], cms[3].toInt()   ); else
        if (cms[1].toLower() == "real")   settings->setValue(cms[2], cms[3].toFloat() ); else
        if (cms[1].toLower() == "bool")   settings->setValue(cms[2],(cms[3]=="true")? true: false);
                                    else  settings->setValue(cms[2], cms[3] );

        wListRes->addItem( cms[2] +" = "+ settings->value(cms[2], "unknown").toString() );
        settings->sync();
        return;
    }


    if (cmdText.toLower().startsWith("patch")){

        QString fileName = QFileDialog::getOpenFileName(
                    muDialog,   tr("MySQL patch file"),
                    QString(),  tr("SQL file (*.sql)"));

        if (fileName.isEmpty()) return;
        cmdText = file_to_string( fileName );
    }


    try{

        mysql_query( mysql, CQSTR(cmdText) );

        results = mysql_store_result(mysql);

        if (results){

            int cf  = mysql_field_count(mysql);
            int cc;
            QString line;

            MYSQL_FIELD* fields = mysql_fetch_fields(results);

            for (cc=0; cc<cf; cc++)
                line += "["+QString( fields[cc].name ) + "] ";

            wListRes->addItem(line);


            while((record = mysql_fetch_row(results))){
                line = "";
                for (cc=0; cc<cf; cc++)
                    line += QString(record[cc]) + "\t";

                wListRes->addItem(line);
            }

            mysql_free_result(results);

        }else{
            QString erd = QString("Error while executing SQL command\n%1")
                                .arg( mysql_error(mysql) );
            qDebug() << erd;
            wListRes->addItem(erd);
        }

    }catch( QException &e ){

        qDebug() << "Error while executing SQL command";
        wListRes->addItem("Error while executing SQL command");
    }

}

bool RiTApplication::dbReadUser(int uid, RiTUser* *rUser){


    {   mysql_query( mysql,
            QString("SELECT * FROM ruser WHERE idruser=%1" )
                     .arg( uid ).toStdString().c_str() );

        QString user, ucdate;
        bool    uadm;

        if((results = mysql_store_result(mysql))
        && (record  = mysql_fetch_row(results))){

            user    = QString(record[1]);
            ucdate  = QString(record[3]);
            uadm    = QString(record[4]).toInt();

            if (*rUser){ delete *rUser;  *rUser = 0; }

             *rUser = new RiTUser(this);
            (*rUser)->setRiTUser(uid, user, uadm, ucdate);

        }else
            qDebug() << "Warning! No users found with id" << uid;

        mysql_free_result(results);

        if( uid == -1 )
            return false;
    }


    {   mysql_query( mysql,
           QString("SELECT p.prop, ifnull(propval,valdefault) "
                    "FROM ruprop p left outer join accprop a "
                    "on (p.prop = a.prop and idruser=%1)" )
                     .arg( uid ).toStdString().c_str() );

        if ( results  = mysql_store_result(mysql))
        while((record = mysql_fetch_row(results)))
            (*rUser)->setRiTProp( QString(record[0]), record[1] );

        mysql_free_result(results);
    }


    {   mysql_query( mysql,
            QString("SELECT p.priv, ifnull(prival,valdefault) "
                    "FROM rupriv p left outer join accpriv a "
                    "on (p.priv = a.priv and idruser=%1)" )
                     .arg( uid ).toStdString().c_str() );

        QString pvn;

        if ( results  = mysql_store_result(mysql))
        while((record = mysql_fetch_row(results))){

            pvn = QString(record[0]);

            (*rUser)->setRiTPriv( pvn,
                        (pvn.startsWith("@") && privs.contains( pvn ))
                          ? privs[ pvn ].enabled
                          : QString(record[1]).toInt()
                    );
        }

        mysql_free_result(results);
    }

    dbReadUserCache( rUser );

    return  true;
}


//int RiTApplication::dbReadUserCache(RiTUser* *rUser){

//    mysql_query( mysql,
//        QString("SELECT idaccache, identry, typentry, created FROM accache WHERE idruser=%1" )
//                    .arg( (*rUser)->userId() ).toStdString().c_str() );

//    results = mysql_store_result(mysql);
//    int cnt = 0;

//    while((record = mysql_fetch_row(results)))
//        cnt +=  (*rUser)->setRiTEntry(
//                    QString(record[0]).toInt(), QString(record[1]),
//                    QString(record[2]),         QString(record[3]));

//    mysql_free_result(results);

//    //return  (*rUser)->cache().size();
//    return cnt;
//}


int RiTApplication::dbReadUserCache(int u, QList< RiTEntryCache* > *entries ){

    mysql_query( mysql,
        QString(" SELECT idaccache, identry, typentry, created, permaskall, 1, idruser"
                " FROM   accache WHERE  idruser=%1 or permaskall"
                " UNION"
                " SELECT s.idaccache, e.identry, e.typentry, "
                        "e.created, e.permaskall, s.permask, e.idruser"
                " FROM   sharcache s"
                " INNER JOIN accache e ON ("
                    "s.idruser=%1 "
                    "and s.idaccache = e.idaccache "
                    "and s.permask)"
                    ).arg( u ).toStdString().c_str() );
    int cnt = 0;

    if ( results = mysql_store_result(mysql))
    while(record = mysql_fetch_row(results)){

        RiTEntryCache *entry = new RiTEntryCache(this);

        cnt += entry->setRiTEntry(
                QString(record[0]).toInt(), QString(record[1]),
                QString(record[2]),         QString(record[3]),
                QString(record[4]).toInt(), QString(record[5]).toInt(),
                QString(record[6]).toInt());

        entries->append( entry );
    }

    mysql_free_result(results);

    //return  (*rUser)->cache().size();
    return cnt;
}


int RiTApplication::dbReadUserCache(RiTUser* *rUser){

    QList< RiTEntryCache* > entries;
    dbReadUserCache( (*rUser)->userId(), &entries );
    int cnt = 0;

    foreach( RiTEntryCache* e, entries )
        cnt +=  (*rUser)->setRiTEntry(      e->idEntry(),
                            e->uinIncome(), e->typeEntry(),
                            e->dtCreated(), e->permitionsAll(),
                            e->permitions(),e->idOwner() );
    qDeleteAll( entries );
                entries.clear();
    return cnt;
}


/*
int RiTApplication::dbReadUserCache(RiTUser* *rUser){

    mysql_query( mysql,
        QString(" SELECT idaccache, identry, typentry, created, permaskall, NULL, idruser"
                " FROM   accache"
                " WHERE  idruser=%1 or permaskall"
                " UNION"
                " SELECT s.idaccache, e.identry, e.typentry, e.created, NULL, s.permask, e.idruser"
                " FROM   sharcache s"
                " INNER JOIN accache e ON (s.idruser=%1 and s.idaccache = e.idaccache and s.permask)")
                    .arg( (*rUser)->userId() ).toStdString().c_str() );

    results = mysql_store_result(mysql);
    int cnt = 0;

    while((record = mysql_fetch_row(results)))
        cnt +=  (*rUser)->setRiTEntry(
                    QString(record[0]).toInt(), QString(record[1]),
                    QString(record[2]),         QString(record[3]),
                    QString(record[4]).toInt(), QString(record[5]).toInt(),
                    QString(record[6]).toInt()
                );

    mysql_free_result(results);

    //return  (*rUser)->cache().size();
    return cnt;
}

*/


QString RiTApplication::getEntryData(int eId){

    mysql_query( mysql,
        QString("SELECT dataentry FROM accache "
                "WHERE  idaccache= %1 " )
                    .arg( eId ).toStdString().c_str() );

    if ((results = mysql_store_result(mysql))
    &&  (record  = mysql_fetch_row(results)))
        return base64Decode( record[0] );

    return  QString();
}


QString RiTApplication::getEntryData(RiTEntryCache *e){
    return  getEntryData( e->idEntry() );
}


void RiTApplication::setEntryData(RiTEntryCache *e, QString d){

    QString qry = QString(
        "UPDATE accache   "
        "  SET  dataentry = '%2'"
        "  WHERE idaccache=  %1 " );

    mysql_query( mysql,
         qry.arg( e->idEntry() )
            .arg( base64Encode( d ) )
                .toStdString().c_str() );

    int afr = mysql_affected_rows( mysql );

    qDebug() << "setEntryData Affected rows:" << afr
             << "dsize" << d.size();
}


void RiTApplication::buildLayout(){

    QVBoxLayout *dLayout = new QVBoxLayout(dialog);
    QDialogButtonBox *dBx= new QDialogButtonBox(Qt::Horizontal, dialog);
                 cbxUsers= new QComboBox(dialog);   cbxUsers->setView(new QListView());
                 edPasswd= new QLineEdit(dialog);
//                 sbInfo  = new QStatusBar(dialog);

    QLabel      *picDecor= new QLabel(dialog);
                 picDecor->setPixmap( QPixmap(":/img/dnalogin.png") );

    dLayout->addWidget(picDecor);    dLayout->addWidget(cbxUsers);
    dLayout->addWidget(edPasswd);    dLayout->addWidget(dBx, 0, Qt::AlignHCenter);
//    dLayout->addWidget(sbInfo);

    dialog->setLayout( dLayout );
    dialog->setWindowTitle( tr("Authorization") );
    dialog->setObjectName("QDialog_dialog");
    dialog->setFixedSize(310, 200);

//    sbInfo->setSizeGripEnabled(false);

    edPasswd->setEchoMode(QLineEdit::Password);

    if( privs["@SINGLE_LOGIN"].enabled ){
        edPasswd->setReadOnly(true);
        edPasswd->setPlaceholderText( tr("Free password mode") );
    }else{
        edPasswd->setFocus();
        edPasswd->setToolTip(tr("Enter password here"));
        edPasswd->setPlaceholderText( edPasswd->toolTip() );
    }

    QPushButton *btLogin= dBx->addButton( tr("Enter"),      QDialogButtonBox::ActionRole );
                 btEdit = dBx->addButton( tr("Settings"),   QDialogButtonBox::ActionRole );
    QPushButton *btCancel=dBx->addButton( tr("Cancel"),     QDialogButtonBox::RejectRole );
//    QPushButton *btNext = dBx->addButton( tr("Next"),     QDialogButtonBox::AcceptRole );

    btLogin->setEnabled(false);     btLogin->setDefault(true);
    btEdit->setEnabled( false);     btEdit->setVisible( _currentLogin || !appIsAxServer );

//    btEdit->setVisible( !appIsAxServer );
//    btNext->setEnabled( false);

    connect( cbxUsers,  SIGNAL(currentIndexChanged(int)),  edPasswd,SLOT(clear()) );
    connect( this,      SIGNAL(afterDbConnected(bool)),    btLogin, SLOT(setEnabled(bool)));
    connect( this,      SIGNAL(afterLogIn(bool)),          btEdit,  SLOT(setEnabled(bool)));
    connect( this,      SIGNAL(afterLogIn(bool)),          btEdit,  SLOT(setVisible(bool)));
//    connect( this,      SIGNAL(afterLogIn(bool)),          btNext,  SLOT(setEnabled(bool)));
    connect( this,      SIGNAL(afterLogIn(bool)),          btLogin, SLOT(setDisabled(bool)));
    connect( this,      SIGNAL(afterLogIn(bool)),          edPasswd,SLOT(setDisabled(bool)));
    connect( this,      SIGNAL(afterLogIn(bool)),          cbxUsers,SLOT(setDisabled(bool)));
//    connect( this,      SIGNAL(afterAdminLogIn(bool)),     cbxUsers,SLOT(setEnabled(bool)));
    connect( this,      SIGNAL(usersSelected()),          this,     SLOT(onUsersSelected()));
    connect( edPasswd,  SIGNAL(returnPressed()),           this,    SLOT(tryLogIn()));
    connect( btLogin,   SIGNAL(clicked()),                 this,    SLOT(tryLogIn()));
    connect( btEdit,    SIGNAL(clicked()),                 this,    SLOT(editUserDialog()));
    connect( btCancel,  SIGNAL(clicked()),                 dialog,  SLOT(reject()));

//    connect( btNext,    SIGNAL(clicked()),                 dialog,  SLOT(accept()));
//    connect( btNext,    SIGNAL(clicked()),                 this,    SLOT(dataPump()));

    initStyles();
//    initFont();
}

//void RiTApplication::initFont(){

//    QVariant pp;
//    QFont    ff = dialog->font();
//    bool     fs=false;

//    if ((pp = property("font")).isValid() && (fs=true))
//        ff.setFamily(pp.toString());

//    if ((pp = property("fontpointsize")).isValid()&& (fs=true))
//        ff.setPointSize(pp.toInt());

//    if ((pp = property("fontweight")).isValid() && (fs=true))
//        ff.setWeight( pp.toInt() );

//    if(!fs)
//        return;

//    dialog->setFont( ff );
//}


bool RiTApplication::setupUser(){

    if(!_currentLogin)
        return false;

    editUserDialog();
    return true;
}


QString RiTApplication::searchRealTimePCR(){

    QSettings m("HKEY_CLASSES_ROOT\\RealTime_192_App\\shell\\open\\command",
                QSettings::NativeFormat );

    QString vs = m.value(".").toString();

    if ( !vs.isEmpty() )
        vs = vs.left( vs.length() - QString(" \"%1\"").length() );
    else
        qDebug() << "Warning! Empty path of RealTime app";

    return vs;
}


QStringList RiTApplication::loadRealTimePCREntries(){

    QString pfx= "TestINFO_";
    QString ptx= ".ini";
    QDir  rtDir= QFileInfo( searchRealTimePCR() ).absoluteDir();
    QStringList etrs = rtDir.entryList( QStringList() << pfx+"*"+ptx );
    QStringList ops;
    QString usn;

    foreach( QString en, etrs ){
        usn = en;
        usn.remove(0, pfx.length());
        usn.chop(     ptx.length() );

        //if ( !users.values().contains( usn ) ){
        {

            QSettings uss( QString("%1/%2").arg(rtDir.absolutePath()).arg(en),
                            QSettings::IniFormat );

            foreach (QString t, uss.childGroups())
                ops << QString("%1 - %2").arg( usn ).arg( t );
        }
    }

    return  ops;
}


void RiTApplication::importRiTUsers(){

    DialogRt7Import *dialog7Import
            = new DialogRt7Import( tr("Import Rt7")
                    , tr("Select RealTime_PCR Tests for import")
                    , QStringList() << tr("Operator/Test") << tr("Version") << tr("Notes")
                    , muDialog );

    RiTUserTests utss;

    foreach( int c, users.keys() ){
        RiTCaches        entries;
        RiTCaches        tests;
        RiTEntryCache    *e;
        dbReadUserCache( c, &entries );

        foreach( e, entries )
            if ( e->typeEntry() == "test" )
                 tests << e;

        utss[ users[ c ] ] = tests;
    }

    connect( dialog7Import, SIGNAL(importCompleted(QTreeWidgetItem*)),
             this, SLOT(dialog7ItemImportCompleted(QTreeWidgetItem*)));

    dialog7Import->setRiTUsersTests( utss );
    dialog7Import->exec();
    dialog7Import->deleteLater();

    return;
}


void RiTApplication::dialog7ItemImportCompleted(QTreeWidgetItem* wi){

    QString xUser = wi->data(0, Qt::UserRole+1).toString();
    QString xFile = wi->data(0, Qt::UserRole+3).toString();

    if (!QFile::exists(xFile)){
        qDebug() << "No user" << xUser << "tests converted";
        //return;
    }

    StringHash  inTests = importEntries( xFile );

    if (inTests.isEmpty()){
        qDebug() << "No user" << xUser << "tests converted";
        //return;
    }


    if ( !users.values().contains( xUser )){

        addRiTUser( xUser );

        if ( !users.values().contains( xUser )){
            qDebug() << "Could not create user" << xUser;
            return;
        }else
            qDebug() << "User" << xUser << "created";
    }


    RiTUser         *u = userObject( users.key( xUser ) );
    RiTEntryCache   *ce;
    QTreeWidgetItem* wit;

    foreach(QString t, inTests.keys()){

        ce = u->addCacheEntry( t, "test" );
        ce->setData( inTests[ t ] );

        for( int c=0; c < wi->childCount(); c++ ){
            wit = wi->child(c);

            if (t == wit->data(0, Qt::UserRole).toString()){
                wit->setCheckState(0, Qt::Checked);
                wit->setText(2, tr("Accepted"));
                break;
            }
        }

        qDebug() << t << "Test converted for" << xUser;
    }
}


void RiTApplication::editUserDialog(){

    bool isAdmin = _currentLogin->isAdmin();
    int  muId    = (muDialog && muDialog->isVisible())
                            ? cxName->currentData().toInt()
                            : _currentLogin->userId()
                            ;   //cbxUsers->currentData().toInt();

    //qDebug() << "muDialog" << (0!=muDialog) << (muDialog && muDialog->isVisible());

    if(!dbReadUser(muId, &_modifiedLogin))
        return;

    if(!muDialog){

        if(!dialog){
            dialog = new QDialog(0, WTOPFLAGS);
            buildLayout();
        }

        muDialog = new QDialog(dialog, WTOPFLAGS);
        muDialog->setWindowTitle(dialog->windowTitle()+QString(" - %1").arg(tr("Settings")));
        muDialog->setObjectName("QDialog_muDialog");
        muDialog->setFont( dialog->font() );

        QGridLayout *dLayout = new QGridLayout(muDialog);
        QDialogButtonBox *dBx1= new QDialogButtonBox(Qt::Horizontal, muDialog);
        QDialogButtonBox *dBx2= new QDialogButtonBox(Qt::Horizontal, muDialog);

        cxName = new QComboBox(muDialog);          cxName->setView(new QListView());
                                                   cpName = new QLabel(tr("Operator"));
        edPwd1 = new QLineEdit(muDialog);  QLabel *cpPwd1 = new QLabel(tr("New password"));
        edPwd2 = new QLineEdit(muDialog);  //QLabel *cpPwd2 = new QLabel(tr("Confirm"));

        QPushButton *btOperator = new QPushButton( tr("Additional"), muDialog );
                     btOperator->setToolTip(tr("Account management"));
                     //btOperator->setMaximumWidth( 30 );
                     btOperator->setStyleSheet("QPushButton::menu-indicator"
                                               "{width:0;height:0;padding:0;margin:0;}");
        foreach( int u, users.keys() ){
            cxName->addItem( (users[u]=="guest")? tr("Guest"): users[u], u );
            if (u==muId)
                cxName->setCurrentIndex( cxName->count() - 1 );
        }

        //cxName->setEditable(true);
        cxName->setMinimumWidth(100);

        QTabWidget *pages = new QTabWidget(muDialog);

        lsPerm = new QListWidget(muDialog); //QLabel *cpPerm = new QLabel(tr("Permitions"));
        lsProp = new QListWidget(muDialog); //QLabel *cpProp = new QLabel(tr("Properties"));

        pages->addTab(lsPerm, tr("Permissions"));
        pages->addTab(lsProp, tr("Properties"));


        QWidget *nameFrame= new QWidget;
        QWidget *pwdFrame = new QWidget;
        QHBoxLayout *hbxN = new QHBoxLayout;
        QHBoxLayout *hbxP = new QHBoxLayout;

        nameFrame->setLayout(hbxN);
        pwdFrame->setLayout(hbxP);

        hbxN->addWidget( cxName    , 60 );
        hbxN->addWidget( btOperator, 40 );

        hbxP->addWidget( edPwd1 );
        hbxP->addWidget( edPwd2 );

        dLayout->setVerticalSpacing(0);

        dLayout->addWidget( cpName,     0, 0);
        dLayout->addWidget( nameFrame,  0, 1, 1, 2 );
        dLayout->addWidget( cpPwd1,     1, 0 );
        dLayout->addWidget( pwdFrame,   1, 1, 1, 2 );
        dLayout->addWidget( pages,      2, 0, 1, 3 );

        dLayout->addWidget( dBx1, 3, 0, 1, 1, Qt::AlignLeft   );
        dLayout->addWidget( dBx2, 3, 1, 1, 2, Qt::AlignRight  );

        QRect g = settings->value("main/geometryUdialog",
                        QRect(QPoint(100, 100), //dialog->geometry().topLeft(),
                              QSize(400, 400))).toRect();

//        QRect   g = dialog->geometry();
//                g.setSize( QSize(400, 400) );

        muDialog->setLayout( dLayout );
        muDialog->setGeometry( g );

        QValidator *valPwd = new QRegExpValidator(rxNamePass, this);

        edPwd1->setToolTip(tr("Enter password here"));
        edPwd1->setPlaceholderText( edPwd1->toolTip() );
        edPwd1->setEchoMode(QLineEdit::Password);
        edPwd1->setValidator(valPwd);
        //edPwd1->setMinimumWidth(100);

        edPwd2->setToolTip(tr("Confirm password"));
        edPwd2->setPlaceholderText( edPwd2->toolTip() );
        edPwd2->setEchoMode(QLineEdit::Password);
        edPwd2->setValidator(valPwd);
        //edPwd2->setMinimumWidth(100);

        QPushButton *btSave = dBx2->addButton( tr("Apply"),  QDialogButtonBox::ActionRole );

        QMenu   *opMenu = new QMenu(btOperator);
        QAction *acImport;

        chxIsAdm = opMenu->addAction( tr("Admin privileges") );
        chxIsAdm->setCheckable(true);

        opMenu->addAction( tr("Add new user"), this, SLOT(addRiTUser()) )
                ->setIcon(QIcon(":/img/images/add_user.png"));

        acImport =
          opMenu->addAction( tr("Load RealTime users"), this, SLOT(importRiTUsers()) );

        opMenu->addAction( tr("Delete operator"),  this, SLOT(delRiTUser()) )
                ->setIcon(QIcon(":/img/images/delete_user.png"));

        opMenu->addAction( tr("Apply preset"),  this, SLOT(openPresetRiTUser()) )
                ->setIcon(QIcon(":/img/images/open_file.png"));

        opMenu->addAction( tr("Save preset"),   this, SLOT(savePresetRiTUser()) )
                ->setIcon(QIcon(":/img/images/save_file.png"));

        btOperator->setMenu(opMenu);
        btOperator->setEnabled(isAdmin);

        //acImport->setVisible( QFile::exists( searchRealTimePCR() ) );
        acImport->setVisible( false );

//        QPushButton *btNew  = dBx->addButton( tr("Add new"),QDialogButtonBox::ActionRole );
//        QPushButton *btDel  = dBx->addButton( tr("Delete"), QDialogButtonBox::ActionRole );


        QPushButton *btCancel=dBx2->addButton( tr("Cancel"), QDialogButtonBox::RejectRole );
        QPushButton *btPump = dBx1->addButton( tr("Data"),   QDialogButtonBox::ActionRole );
//        QPushButton *btSqlCon=dBx->addButton( tr("SQL"),    QDialogButtonBox::ActionRole );


        cxName->setEnabled( isAdmin );  lsPerm->setEnabled( isAdmin );  lsProp->setEnabled( isAdmin );
//        btNew->setEnabled(  isAdmin );   btDel->setEnabled( isAdmin );
//        btSqlCon->setVisible( isAdmin );

        bool displayBP = settings->value("main/displayBP", false).toBool();

        btPump->setVisible( isAdmin && displayBP );
//        btPump->setVisible( isAdmin );

//        connect(lsPerm,  SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(modifPerm(QListWidgetItem*)));
//        connect(lsProp,  SIGNAL(doubleClicked(QModelIndex)),    this, SLOT(modifProp()));
        connect(lsProp,  SIGNAL(clicked(QModelIndex)),    this, SLOT(modifProp()));

        connect(btSave,  SIGNAL(clicked()), this,    SLOT(saveRiTUser()));
//        connect(btNew,   SIGNAL(clicked()), this,    SLOT(addRiTUser()));
//        connect(btDel,   SIGNAL(clicked()), this,    SLOT(delRiTUser()));
        connect(btCancel,SIGNAL(clicked()), muDialog,   SLOT(reject()));
        connect(muDialog, SIGNAL(rejected()), dialog,   SLOT(accept()));

//        connect(btSqlCon,SIGNAL(clicked()), this,    SLOT(openConsoleSQL()));
        connect(btPump,  SIGNAL(clicked()),                 this,   SLOT(dataPump()));
        connect(cxName,  SIGNAL(currentIndexChanged(int)),  this,   SLOT(editUserDialog()));

        connect( this,  SIGNAL(afterAdminLogIn(bool)),  btOperator, SLOT(setEnabled(bool)));
        connect( this,  SIGNAL(afterAdminLogIn(bool)),  cxName,     SLOT(setEnabled(bool)));
        connect( this,  SIGNAL(afterAdminLogIn(bool)),  lsPerm,     SLOT(setEnabled(bool)));
        connect( this,  SIGNAL(afterAdminLogIn(bool)),  lsProp,     SLOT(setEnabled(bool)));
        connect( this,  SIGNAL(afterAdminLogIn(bool)),  btPump,     SLOT(setEnabled(bool)));
    }

    if (muDialog->isHidden()){

        cxName->blockSignals(true);

        int ci = cbxUsers->currentData().toInt();

        for(int i=0; i < cxName->count(); i++ )
            if (cxName->itemData(i).toInt() == ci){
                cxName->setCurrentIndex(i);
                break;
            }

        cxName->blockSignals(false);
    }

    edPwd1->setEnabled( _modifiedLogin->userName() != "guest" );
    edPwd2->setEnabled( _modifiedLogin->userName() != "guest" );

    chxIsAdm->setChecked(_modifiedLogin->isAdmin() );
    cpName->setText(     _modifiedLogin->isAdmin()
                            ? tr("Administrator") : tr("Operator") );

//    lsPerm->setSortingEnabled(true);
//    lsProp->setSortingEnabled(true);

    viewProps();
    viewPerms();

    muDialog->isVisible()
        ? muDialog->show()
        : muDialog->exec();
}



bool RiTApplication::viewProps(){

    QListWidgetItem *wi;
    QStringList ppn;
    QString     ph;
    QString     pfx;
    QString     pvl;

    lsProp->clear();

    //qgetenv();

    foreach (QString p, _modifiedLogin->attrNames() ){

        ppn = p.startsWith("file:")
           || p.startsWith("dir:")
           || p.startsWith("num:")
           || p.startsWith("str:")
               ? p.split(":")
               : (QStringList() << "str" << p);

        pfx = ppn.first();
        ppn.removeFirst();
        ph = ppn.join(':');
        pvl= _modifiedLogin->getAttr( p );

        ph = trProp.contains( p ) ? trProp[ p ]: ph ;

        //qDebug() << "Prop "<< ph << "(" << pfx <<"), val" << pvl;

        wi = new QListWidgetItem( QString("%1 - %2").arg( ph )
                        .arg( (QStringList() << "dir" << "file").contains(pfx)
                              ? ( QFile::exists(pvl)? pvl : "..." )
                              : ( pvl.isEmpty() ? tr("Define..."): pvl ) )
                    );

        wi->setData( Qt::ToolTipRole, tr("Click to change"));
        wi->setData( Qt::UserRole, p );

        lsProp->addItem( wi );
    }

    lsProp->sortItems();
    return true;
}


bool RiTApplication::viewPerms(){

    lsPerm->clear();
    lsPerm->blockSignals(true);

    QHash <QString, QList< QListWidgetItem* > > wiGrouped;
    QString g;
    QListWidgetItem *wi;
    QHash <int, QString> ixGrp;

    foreach (QString p, _modifiedLogin->privNames() ){

        g = privs[ p ].group;

        if ( !wiGrouped.contains( g ) ){
              wiGrouped[ g ] = QList< QListWidgetItem* >();
              ixGrp[ privs[ p ].idgroup ] = g;
        }

        wi = new QListWidgetItem ( trPriv.contains(p) ? trPriv[ p ] : p );

        wi->setData( Qt::UserRole, p );
        wi->setCheckState(( p.startsWith("@") ? privs[p].enabled : _modifiedLogin->getPriv( p ))
                           ? Qt::Checked : Qt::Unchecked  );

        wiGrouped[ g ].append( wi );
    }

    int  ig;
    QList<int> iKs = ixGrp.keys();
         qSort(iKs);

     foreach (ig, iKs){

         g = ixGrp[ ig ];

         lsPerm->addItem(
             wi = new QListWidgetItem ( trPriv.contains(g)? trPriv[g]: g ) );

         QFont wf = wi->font();
               wf.setBold(true);

             wi->setData( Qt::UserRole + 1, privs[ g ].idgroup );
             wi->setFont( wf );

         foreach (wi, wiGrouped[ g ])
             lsPerm->addItem( wi );
     }

    lsPerm->blockSignals(false);

    return true;
}


bool RiTApplication::saveRiTUser(){

    QString pwd = edPwd1->text();
    int     uid = _modifiedLogin->userId();
    bool    adm = chxIsAdm->isChecked();// chxIsAdm->checkState() == Qt::Checked;


    if (!rxNamePass.exactMatch( pwd )){
        QMessageBox::warning( muDialog, tr("Check input"),
            tr("Please check Your input.\n"
               "Password may contains\n"
               "0-1, a-z and A-Z letters"));
        return false;
    }


    if ((_modifiedLogin->userId() == _currentLogin->userId())
    &&   _modifiedLogin->isAdmin()
    &&   !adm ){

        QMessageBox::warning( muDialog, tr("Save"),
            tr("Your cannot cast admin privs for Yourself."));
        chxIsAdm->setChecked(true);
        return false;
    }

    if ( pwd != edPwd2->text() ){
        QMessageBox::warning( muDialog, tr("Check password"),
            tr("Password not confirmed! Please check Your input."));
        return false;
    }

    QString qry = QString("UPDATE ruser SET admin = %1 WHERE idruser=%2" );

    if (!pwd.isEmpty()){
            qry = QString("UPDATE ruser SET admin = %1, pwd = password('%3') WHERE idruser=%2" );
            edPwd1->clear();
            edPwd2->clear();
    }

    mysql_query( mysql,
        qry.arg( adm ).arg( uid ).arg( pwd )
            .toStdString().c_str() );

    int s = (mysql_field_count(mysql) == 0);

    if ( _currentLogin->isAdmin() ){

        for( int i=0;  i < lsPerm->count(); i++ )
            s += modifPerm( lsPerm->item(i) );

        dbSelectPrivs();
        editUserDialog();
    }


    muDialog->accept();

    // s && RiSplashInfo( muDialog, tr( "Operation completed" ), 1000 ).exec();

    return true;
}

int confirmCustom(QWidget *parent, const QString &title,
                  const QString& text,
                  QString& button0text=RiTApplication::tr("Apply"),
                  QString& button1text=RiTApplication::tr("Cancel")){

    QMessageBox mc(parent);
    mc.setWindowTitle( title );
    mc.setText( text );
    mc.setIcon(QMessageBox::Question);
    mc.addButton( button0text, QMessageBox::AcceptRole );
    mc.addButton( button1text, QMessageBox::RejectRole );

    mc.exec();

    return mc.result();
}


bool RiTApplication::delRiTUser(){

    int     uid = _modifiedLogin->userId();
    QString unm = _modifiedLogin->userName();

    if( _currentLogin->userId() == uid ){
        QMessageBox::warning(muDialog,
            tr("Delete account"),
            tr("You cannot delete Yourself!"));

        return false;
    }

    if ( QMessageBox::AcceptRole
         != confirmCustom( muDialog, tr("Delete account"),
                tr("Delete account")+QString(" '%1'.\n").arg(unm)+
                tr("Do You confirm the action?"))){

        return false;
    }


    QString qry = QString(
        "DELETE FROM accache WHERE idruser = %1;"
        "DELETE FROM accprop WHERE idruser = %1;"
        "DELETE FROM accpriv WHERE idruser = %1;"
        "DELETE FROM ruser   WHERE idruser = %1;" );

    mysql_query( mysql, CQSTR(qry.arg( uid )) );

    dbSelectUsers();

    if( users.contains( uid ) ){
        QMessageBox::warning(muDialog, tr("Delete account"),
                    tr("Cannot delete account '%1'.").arg(unm));
        return  false;
    }

    cxName->removeItem( cxName->currentIndex() );
    QMessageBox::information(muDialog, tr("Delete account"),
                tr("Account '%1' deleted successfuly.").arg(unm));

    //btOperator->setMenu(opMenu); app->processEvents();

    return true;
}


bool RiTApplication::openPresetRiTUser(){

    QFileDialog pfDialog( muDialog,  tr("Open profile file"),
                          "profiles",tr("Profile file (*.upf)"));
//                          QString(),tr("Profile file (*.upf)"));
                pfDialog.setOption( QFileDialog::DontUseNativeDialog, true );

    QCheckBox   *cb = new QCheckBox(tr("&Apply for new users"), &pfDialog);
                 cb->setToolTip(tr("Set this profile for users created after"));

    QHBoxLayout *hbl= new QHBoxLayout();
    QGridLayout *box= dynamic_cast< QGridLayout* >(pfDialog.layout());
    QString      fileName;

    pfDialog.setLabelText(QFileDialog::FileType,tr("Files of type:"));
    pfDialog.setLabelText(QFileDialog::LookIn,  tr("Look In:"));
    pfDialog.setLabelText(QFileDialog::FileName,tr("File name:"));
    pfDialog.setLabelText(QFileDialog::Accept,  tr("&Open"));
    pfDialog.setLabelText(QFileDialog::Reject,  tr("Cancel"));

    hbl->addWidget( cb );
    box->addLayout( hbl, box->rowCount(), 0, 1, -1 );

    if ( QDialog::Accepted != pfDialog.exec() )
        return false;

    fileName = pfDialog.selectedFiles().first();

    QListWidgetItem *ipr;
    QString         pnm;
    QStringList     recordP;
    QStringList     entriesP = file_to_string( fileName ).split("\n");
    QHash< QString, QListWidgetItem* >
                    privsE;

    for ( int i=0;  i < lsPerm->count(); i++ ) {
        ipr = lsPerm->item( i );
        privsE[ ipr->data(Qt::UserRole).toString() ] = ipr;
    }

    foreach (QString e, entriesP) {

        recordP = e.split("=");
        pnm     = recordP.at(0).trimmed();

        if ( !privsE.contains( pnm )
        ||  (recordP.length() < 2 )
        ||  pnm.startsWith("@"))
                continue;

        ipr = privsE[ pnm ];
        ipr->setCheckState( (recordP.at(1).trimmed() == "1")
                                ? Qt::Checked : Qt::Unchecked );
        if( cb->isChecked()
        &&  privs.contains(pnm) ){
            privs[ pnm ].enabled = (ipr->checkState() == Qt::Checked);
            registerFPrivilege(pnm, privs[ pnm ]);
        }
    }

    return  true;
}


bool RiTApplication::savePresetRiTUser(){

    QFileDialog pfDialog( muDialog,   tr("Save profile file"),
//                          QString(),  tr("Profile file (*.upf)"));
                          "profiles",  tr("Profile file (*.upf)"));


    pfDialog.setOption( QFileDialog::DontUseNativeDialog, true );

    pfDialog.setLabelText(QFileDialog::FileType,tr("Files of type:"));
    pfDialog.setLabelText(QFileDialog::LookIn,  tr("Look In:"));
    pfDialog.setLabelText(QFileDialog::FileName,tr("File name:"));
    pfDialog.setLabelText(QFileDialog::Accept,  tr("&Save"));
    pfDialog.setLabelText(QFileDialog::Reject,  tr("Cancel"));

    if (!pfDialog.exec()) return false;

    QString fileName = pfDialog.selectedFiles().first();
            fileName = fileName.toLower().endsWith("upf") ? fileName : (fileName+".upf");

    QString wstr, pnm;
    QListWidgetItem *ipr;

    for ( int i=0;  i < lsPerm->count(); i++ ) {
        ipr = lsPerm->item( i );

        if( !(pnm = ipr->data(Qt::UserRole).toString()).length()
        ||    pnm.startsWith("@"))
            continue;

        wstr += QString("%1=%2\n")
                .arg( pnm )
                .arg( (ipr->checkState() == Qt::Checked)? "1": "0" );
    }

    return  string_to_file(wstr, fileName);
}


bool RiTApplication::modifProp(){

    QListWidgetItem *ipr;

    if (!(ipr = lsProp->currentItem()))
        return false;

    bool    ok;
    QString pn = ipr->data(Qt::UserRole).toString();
    QString tp = trProp.contains( pn ) ? trProp[ pn ]: pn ;
    QString pv = _modifiedLogin->getAttr(pn);

    QString nv;

    if (pn.startsWith("file:"))
        ok = (nv = QFileDialog::getOpenFileName(
                    muDialog, tr("Open File"), pv)).count();
    else
    if (pn.startsWith("dir:")){
        ok = (nv = QFileDialog::getExistingDirectory(
                    muDialog, tr("Select directory"),pv)).count();
    }else{
        QInputDialog piDg( muDialog );

        piDg.setWindowTitle( tr("Property") );
        piDg.setLabelText(tp);
        piDg.setOkButtonText(tr("Apply"));
        piDg.setCancelButtonText( tr("Cancel") );

        if (pn.startsWith("num:")){

            piDg.setInputMode( QInputDialog::DoubleInput );
            piDg.setDoubleValue( pv.toDouble() );
            piDg.setDoubleMinimum( -2147483647 );
            piDg.setDoubleMaximum(  2147483647 );
            piDg.setDoubleDecimals( 3 );

        }else{
            piDg.setTextValue( pv );
        }

        if (ok = (piDg.exec() == QDialog::Accepted))
            nv = (piDg.inputMode() == QInputDialog::DoubleInput)
                    ? QString("%1").arg( piDg.doubleValue() )
                    : piDg.textValue();
    }

    ok && _modifiedLogin->setAttr(pn, nv);

//    dbReadUser( _modifiedLogin->userId(), &_modifiedLogin );
    editUserDialog();

    return true;
}


bool RiTApplication::modifPerm( QListWidgetItem* ipr ){

//    QListWidgetItem *ipr = lsPerm->currentItem();

    if (!ipr) return false;

    QString pn = ipr->data(Qt::UserRole).toString();
    bool    pv = ipr->checkState();

    setRiTPriv( _modifiedLogin->userId(), pn, pv );

    return true;
}


//bool RiTApplication::modifPerm( QListWidgetItem* ipr ){

////    QListWidgetItem *ipr = lsPerm->currentItem();

//    if (!ipr) return false;

//    QString pn = ipr->data(Qt::UserRole).toString();
//    bool    pv = _modifiedLogin->getPriv(pn);

//    setRiTPriv(  _modifiedLogin->userId(), pn, !pv );

////    dbReadUser(  _modifiedLogin->userId(), &_modifiedLogin );
//    editUserDialog();

//    return true;
//}


bool RiTApplication::addRiTUser(QString unm){

    //QString unm = cxName->currentText();// text();
//            unm = unm.isEmpty() ? cxName->currentText() : unm ;// text();

    QString pwd = "111"; //edPwd1->text();
    bool    adm = false; //chxIsAdm->checkState() == Qt::Checked;

    if ( unm.trimmed().isEmpty()
    ||   users.values().contains( unm.toLower() )){

        NewAccountDialog dt( users.values(), dialog );
        bool    dOk = dt.exec() == QDialog::Accepted;

        QString text= dt.getNewValue();

                      dt.deleteLater();

        if ( !dOk  ||  text.isEmpty() )
            return  false;

        unm = text;
    }


    if (!rxNamePass.exactMatch( unm+pwd )){

        QMessageBox::warning( muDialog, tr("Check input"),
            tr("Please check Your input.\n"
               "Name and password should contain\n"
               "0-1, a-z and A-Z letters"));
        return false;
    }


//    if ( pwd.isEmpty()
//    ||  (pwd != edPwd2->text()) ){

//        QMessageBox::warning( muDialog, tr("Check password"),
//            tr("Password not confirmed! Please check Your input."));
//        return false;
//    }

    QString qry = QString(
                "INSERT INTO ruser (name, pwd, created, admin)  "
                "   VALUES ('%1', password('%2'), now(), %3);   " );

    mysql_query( mysql, CQSTR(qry.arg( unm ).arg( pwd ).arg( adm )) );

    int afr = mysql_affected_rows( mysql );

    qDebug() << "Affected rows:" << afr;

    if (afr>0) {

        dbSelectUsers();
        int     unn     = -1;
        int     uid     = users.key( unm, unn );
        QStringList propsLst;

        qDebug() << "New uid:" << uid;

        if (uid != unn){

            QString pv;

            foreach( QString pr, props.keys()){

                pv = props[ pr ];

                if (pv.contains("%RITUSER%"))
                    pv = pv.replace("%RITUSER%", unm);


                if (pr.startsWith("dir:")){

                    QFileInfo pvIf = QFile(pv);

                    if(!pvIf.exists())
                        QDir().mkpath( pv );

                    if (pvIf.exists())
                        pv = pvIf.absoluteFilePath();
                }

                setRiTProp(uid, pr, pv);

                if (trProp.contains( pr ))
                     propsLst << QString("%1 - %2").arg(trProp[ pr ]).arg( pv );
                else propsLst << QString("%1 - %2").arg(pr).arg( pv );
            }

            cxName->addItem( (users[uid]=="guest")? tr("Guest"): users[uid], uid );
            cxName->setCurrentIndex( cxName->count() - 1 );

            QMessageBox::information(muDialog, tr("New operatop"),
                        tr("New operator '%1' appended.").arg(unm)
                      + tr("\nInitial password is: '%1'").arg(pwd)
                      + tr("\n\n%1:\n%2").arg(tr("Properties"))
                                         .arg(propsLst.join("\n")));

            edPwd1->clear();
            edPwd2->clear();
        }        
    }else   QMessageBox::warning(muDialog, tr("New operatop"),
                tr("Something wrong, cannot append operator '%1'.").arg(unm));


//    muDialog->hide();

    return afr && (mysql_field_count(mysql) == 0);
}


RiTApplication::~RiTApplication(){

                                            //qDebug() << "Rita close (1)";
//    if ( !--ritAppCounter )
//        dbDisconnect();
                                            //qDebug() << "Rita close (2)";
    if (muDialog){

        settings->setValue("main/geometryUdialog",
                           muDialog->geometry());
        muDialog->deleteLater();
        muDialog = 0;
    }
                                            //qDebug() << "Rita close (2.1)";
    if (dialog){
        dialog->deleteLater();
        dialog = 0;
    }


                                            //qDebug() << "Rita close (3)";
    if (        _currentLogin ){
        delete  _currentLogin;
                _currentLogin = 0;
    }
//                                            //qDebug() << "Rita close (4)";
    if (        _modifiedLogin ){
        delete  _modifiedLogin;
                _modifiedLogin = 0;
    }
                                            //qDebug() << "Rita close (5)";
    settings->sync();
    settings->deleteLater();
                                            //qDebug() << "Rita close (6)";
//    ritaTranslator->deleteLater();
                                            //qDebug() << "Rita close (complete)";
}

bool RiTApplication::translatePrivs(){

    trProperty( "dir:PR_HOME",  tr("Operator's home dir")   );
    trProperty( "str:PR_EMAIL", tr("Contact e-mail")        );

    trPrivilege("Common",           tr("Common"));
//        trPrivilege("Modify protocol",  tr("Modify protocol"));
    trPrivilege("Execution",        tr("Execution"));
    trPrivilege("Interface",        tr("Interface"));
    trPrivilege("Confirmations",    tr("Confirmations"));

    registerPrivilege( "ENABLE_SELECT_FN",  false, 3, "");
    registerPrivilege( "COPY_ONLINE",       false, 3, "");

    QHash<QString, QString> trP;

    trP["IGNORE_EXPOSURE"]  = tr("Skip expositions validation");

    trP["IGNORE_PROGRAM"]   = tr("Skip amplification programms check");
    trP["IGNORE_VOLUME"]    = tr("Skip volume validation");
    trP["IGNORE_MIN_LEVEL"] = tr("Skip min level checking");

    trP["ENABLE_SELECT_FN"] = tr("Select a folder for the results");
    trP["ENABLE_ADD_ANALYSIS"]=tr("Enable advanced analysis");
    trP["ENABLE_PAGE_RUN"]  = tr("Show page Run");
    trP["ENABLE_PAGE_SETUP"]= tr("Show page Setup");
    trP["ENABLE_CROSSTABLE"]= tr("Show cross-table");
    trP["COPY_ONLINE"]      = tr("Saving optical measurement data to a "
                                    "file during protocol execution");

    trP["ENABLE_CMD"]       = tr("Enable command line interface");
    trP["SAVE_LOCATION_WIN"]= tr("Remember windows location");

    trP["CHANGE_APP_PREF"]  = tr("Change application preferences");
    trP["CHANGE_ASYS_PREF"] = tr("Change analysis preferences");
    trP["CHANGE_DEVICE_PREF"]=tr("Change device preferences");
    trP["CONTROL_RUN"]      = tr("Control run process");
    trP["EDIT_PROTOCOL"]    = tr("Edit protocol data");
    trP["EDIT_TEST"]        = tr("Edit test data");
    trP["MASK_DATA"]        = tr("Mask protocol data");
    trP["COPY_BLOCK_TEST"]  = tr("Copy block test");
    trP["@SINGLE_LOGIN"]    = tr("Single user application");

    foreach(QString pp, trP.keys())
        !trPriv.contains(pp) && trPrivilege(pp, trP[ pp ]);

    return true;
}


void RiTApplication::execute(bool isAx){

    appIsAxServer = isAx;

    //ritaTranslator->isEmpty()
    ritaTranslator.isEmpty()
        && setLocale( settings->value( "main/locale", "en" ).toString() );


    if (!isDBConnected && dbConnect()){

        dbSelectProps();
        dbSelectPrivs();
        translatePrivs();

        dbSelectPrivs();
        dbSelectUsers();

        if ( (users.count() == 1)
                &&  appIsAxServer
                &&  tryLogIn( true ) )
            return;
    }


    if(!dialog){
        dialog = new QDialog(0, WTOPFLAGS);
        buildLayout();

//        dialog->winId();
    }

    emit afterDbConnected(isDBConnected);

    if ( isDBConnected ){
        emit usersSelected();
        emit afterLogIn( _currentLogin );
    }

    acceptAppInput( dialog->winId() );
    dialog->exec();
}



void RiTApplication::acceptAppInput( WId winH ){

    //set up a generic keyboard event
    INPUT keyInput;
    keyInput.type = INPUT_KEYBOARD;
    keyInput.ki.wScan = 0; //hardware scan code for key
    keyInput.ki.time = 0;
    keyInput.ki.dwExtraInfo = 0;

    //set focus to the hWnd (sending Alt allows to bypass limitation)
    keyInput.ki.wVk = VK_MENU;
    keyInput.ki.dwFlags = 0;   //0 for key press
    SendInput(1, &keyInput, sizeof(INPUT));

    SetForegroundWindow((HWND) winH); //sets the focus

    keyInput.ki.wVk = VK_MENU;
    keyInput.ki.dwFlags = KEYEVENTF_KEYUP;  //for key release
    SendInput(1, &keyInput, sizeof(INPUT));
}

void RiTApplication::logIn(bool isAx){
    return execute( isAx );
}


void RiTApplication::logOut(bool isAx){

    _currentLogin = 0;
    _modifiedLogin= 0;

    execute( appIsAxServer = isAx );
}



void RiTApplication::dataPump(){

//    if (app->keyboardModifiers() & Qt::AltModifier)
//        openConsoleSQL();

    if (!_currentLogin)
        execute(appIsAxServer);

    if (!_currentLogin
    ||  !_currentLogin->isAdmin())
        return;

//    int  muId    =  cbxUsers->currentData().toInt();

    if(!pmpDialog){

        pmpDialog = new QDialog(0, WTOPFLAGS);
        pmpDialog->setWindowTitle(tr("Data"));

//        exDialog = new QDialog(muDialog, WTOPFLAGS);
//        exDialog->setWindowTitle(muDialog->windowTitle());
        pmpDialog->setObjectName("QDialog_exDialog");

        QGridLayout *dLayout = new QGridLayout(pmpDialog);
        QDialogButtonBox *dBx1= new QDialogButtonBox(Qt::Horizontal, pmpDialog);
        QDialogButtonBox *dBx2= new QDialogButtonBox(Qt::Horizontal, pmpDialog);

        cbSource = new QComboBox(pmpDialog);
        cbSource->setView(new QListView());
        QLabel *cpSource= new QLabel(tr("Source"));

        cbTarget = new QComboBox(pmpDialog);
        cbTarget->setView(new QListView());
        QLabel *cpTarget= new QLabel(tr("Target"));

        lsSource= new QTreeWidget(pmpDialog);
        lsSource->addTopLevelItem( new QTreeWidgetItem( QStringList() << tr("Tests")) );
        lsSource->topLevelItem(0)->setCheckState(0, Qt::Unchecked);
        lsSource->headerItem()->setHidden(true);
        lsSource->setItemsExpandable(false);
        lsSource->setStyleSheet(
                "QTreeWidget::branch:has-children{image: url(:images/empty.png);} " );

//        lsSource= new QListWidget(pmpDialog);
        lsTarget= new QListWidget(pmpDialog);

//        lsSource->setSelectionMode( QAbstractItemView::ExtendedSelection );
        lsTarget->setSelectionMode( QAbstractItemView::ExtendedSelection );

        dLayout->addWidget(cpSource,0, 0);  dLayout->addWidget(cpTarget, 0, 1);
        dLayout->addWidget(cbSource,1, 0);  dLayout->addWidget(cbTarget, 1, 1);
        dLayout->addWidget(lsSource,2, 0);  dLayout->addWidget(lsTarget, 2, 1);

//        dLayout->addWidget(dBx, 3, 0, 1, 2);
        dLayout->addWidget(dBx1,    3, 0, 1, 1, Qt::AlignLeft   );
        dLayout->addWidget(dBx2,    3, 1, 1, 1, Qt::AlignRight  );


        pmpDialog->setLayout( dLayout );

//        QPushButton *btRfrsh= dBx->addButton( tr("Refresh"),QDialogButtonBox::ActionRole );
        QPushButton *btSave = dBx1->addButton( tr("Copy"),   QDialogButtonBox::ActionRole );
        QPushButton *btAccs = dBx1->addButton( tr("Access"), QDialogButtonBox::ActionRole );
        QPushButton *btRemove=dBx1->addButton( tr("Remove"), QDialogButtonBox::ActionRole );
        QPushButton *btCancel=dBx2->addButton( tr("Close"),  QDialogButtonBox::RejectRole );

        connect(btSave,  SIGNAL(clicked()), this,    SLOT(onDataPump()));
        connect(btAccs,  SIGNAL(clicked()), this,    SLOT(onDataAccess()));
        connect(btRemove,SIGNAL(clicked()), this,    SLOT(onDataRemove()));
        connect(btCancel,SIGNAL(clicked()), pmpDialog,SLOT(reject()));

        connect(cbSource, SIGNAL(currentIndexChanged(int)), this, SLOT(onSourceSelected()));
        connect(cbTarget, SIGNAL(currentIndexChanged(int)), this, SLOT(onTargetSelected()));

        connect(lsSource, SIGNAL(itemChanged(QTreeWidgetItem*,int)),
                    this, SLOT(onSourceItemChanged(QTreeWidgetItem*,int)));


//        connect(btRfrsh,  SIGNAL(clicked(bool)), this, SLOT(onSourceSelected()));
//        connect(btRfrsh,  SIGNAL(clicked(bool)), this, SLOT(onTargetSelected()));
    }

    cbSource->clear();
    cbTarget->clear();

    foreach( int c, users.keys() ){
        cbSource->addItem( users[c], c );
        cbTarget->addItem( users[c], c );
    }

    cbSource->addItem( tr("Import from file.."),SC_IFILE ); // -1

    cbTarget->addItem( tr("* Multiple users"),  TC_MUSERS );
    cbTarget->addItem( tr("Export to file.."),  TC_EFILE );
//    cbTarget->addItem( tr("- Removing"),        TC_REMOVE );


    cbSource->setCurrentIndex(0);
    cbTarget->setCurrentIndex(0);


    pmpDialog->isVisible()
            ? pmpDialog->show()
            : pmpDialog->exec();
}


QList< QTreeWidgetItem* >
        RiTApplication::treeListCheckedItems( QTreeWidget* tw ){


    QList< QTreeWidgetItem* > lstChs;
    QTreeWidgetItem *i;

    for( int c=0; c < tw->topLevelItem(0)->childCount(); c++ )
        if ((i = tw->topLevelItem(0)->child(c))->checkState(0) == Qt::Checked )
            lstChs << i;

    return  lstChs;
}


QList<QVariant> RiTApplication::usersIndex() const{

    QList<QVariant> rvl;

    foreach(int u, users.keys()) rvl << u;

    return rvl;
}

RiTUser *RiTApplication::userObject(int uix){

    RiTUser *ur = 0;
    dbReadUser(uix, &ur);
    return ur;
}


void RiTApplication::onDataRemove(){


    QList <int> entryR; // for removing
//    QListWidgetItem *i;
    QTreeWidgetItem *i;

    int sCode = cbSource->currentData().toInt();

    foreach( i, treeListCheckedItems(lsSource) )
        entryR << i->data( 0, 1 + Qt::UserRole ).toInt();

//    foreach( i, lsSource->selectedItems() )
//        entryR << i->data( 1 + Qt::UserRole ).toInt();

    if ((SC_IFILE == sCode)
    ||  entryR.empty())
        return;


    if ( QMessageBox::Cancel == QMessageBox::warning(
            pmpDialog, tr("Removing")+tr(" %1 item(s)").arg(entryR.count()),
                   tr("Entries will be removed from source.\n"
                      "Do you want to complete?"),
                   QMessageBox::Ok | QMessageBox::Cancel,
                   QMessageBox::Cancel)
         )return;


    foreach( int r, entryR ){

        QString qry = QString(
            "DELETE FROM accache WHERE idaccache = %1 ");

        mysql_query( mysql, CQSTR(qry.arg( r )) );

        int afr = mysql_affected_rows( mysql );

        qDebug() << "Affected rows:" << afr;

        QMessageBox::information(pmpDialog, tr("Removing"),
                    tr( "Removed %1 item(s)").arg(afr) );
    }

    onSourceSelected();
    onTargetSelected();
}



/*
void RiTApplication::onDataRemove(){


    QList <int> entryR; // for removing
    QListWidgetItem *i;
    int sCode = cbSource->currentData().toInt();

    foreach( i, lsSource->selectedItems() )
        entryR << i->data( 1 + Qt::UserRole ).toInt();

    if ((SC_IFILE == sCode)
    ||  entryR.empty())
        return;


    if ( QMessageBox::Cancel == QMessageBox::warning(
            pmpDialog, tr("Removing")+tr(" %1 item(s)").arg(entryR.count()),
                   tr("Entries will be removed from source.\n"
                      "Do you want to complete?"),
                   QMessageBox::Ok | QMessageBox::Cancel,
                   QMessageBox::Cancel)
         )return;


    foreach( int r, entryR ){

        QString qry = QString(
            "DELETE FROM accache WHERE idaccache = %1 ");

        mysql_query( mysql,
            qry.arg( r ).toStdString().c_str() );

        int afr = mysql_affected_rows( mysql );

        qDebug() << "Affected rows:" << afr;

        QMessageBox::information(pmpDialog, tr("Removing"),
                    tr( "Removed %1 item(s)").arg(afr) );
    }

    onSourceSelected();
    onTargetSelected();
}


void RiTApplication::onDataPump(){

    QHash <QString, QString> dataS;
    QList <int> usersD;
    QList <int> entryR; // for removing
    QListWidgetItem *i;

    if (lsSource->selectedItems().empty()){

        if (lsSource->count())
            lsSource->selectAll();
        else
            onSourceSelected();
    }

    bool cancelItem;
    int  d;

    foreach( i, lsSource->selectedItems() ){

        cancelItem = false;

        for(d=0; d < lsTarget->count(); d++){       // find first item with same text
                                                    //
            if (lsTarget->item(d)->text() == i->text()){

                cancelItem = QMessageBox::No
                        == QMessageBox::warning(
                                pmpDialog, tr("Export.."),
                                tr("Rewrite '%1' entry?").arg(i->text()),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No );
                break;
            }
        }

        if (cancelItem) continue;

        dataS[ i->text() ] = i->data( Qt::UserRole ).toString();
        entryR << i->data( 1 + Qt::UserRole ).toInt();
    }

    int tCode = cbTarget->currentData().toInt();
    int sCode = cbSource->currentData().toInt();


    if ( TC_MUSERS == tCode ){
        foreach( i, lsTarget->selectedItems() )
            usersD << i->data( Qt::UserRole ).toInt();
    }else
//    if ( TC_REMOVE == tCode ){

//        if ((SC_IFILE == sCode)
//        ||  entryR.empty())
//            return;

//        if ( QMessageBox::Cancel == QMessageBox::warning(
//                exDialog, tr("Removing"),
//                       tr("Entries will be removed.\n"
//                          "Do you want to complete?"),
//                       QMessageBox::Ok | QMessageBox::Cancel,
//                       QMessageBox::Cancel)
//             )return;


//        foreach( int r, entryR ){

//            QString qry = QString(
//                "DELETE FROM accache WHERE idaccache = %1 ");

//            mysql_query( mysql,
//                qry.arg( r ).toStdString().c_str() );

//            int afr = mysql_affected_rows( mysql );

//            qDebug() << "Affected rows:" << afr;

//            QMessageBox::information(exDialog, tr("Removing"),
//                        tr( "Removed %1 item(s)").arg(afr) );
//        }


//        onSourceSelected();
//        onTargetSelected();

//        return;

//    }else
    if ( TC_EFILE == tCode ){

        QString fileName = QFileDialog::getSaveFileName(
                    pmpDialog,  tr("Export File"), "test.xrt",
                    tr("DNA-Technology tests (*.xrt);;XML files (*.xml)"));

        if (fileName.isEmpty()) return;

        QDomDocument doc("TESTs"), frag("item");
        QDomElement root;

        doc.appendChild(root = doc.createElement("TESTs"));

        foreach( QString ds, dataS.keys() ){
            QString dd( QByteArray().fromBase64( dataS[ds].toUtf8() ));

            frag.setContent( dd )
            && root.appendChild( frag.firstChild() ).isNull();
        }

        QFile       fileO; fileO.setFileName(fileName);
        QTextStream streamO( &fileO );

        if (fileO.open(QIODevice::WriteOnly|QIODevice::Truncate)){
            doc.save(streamO, 1);
            fileO.close();

            QMessageBox::information( dialog, tr("Export.."),
                        tr( "Operation completed" ) + ".\n" +
                        tr( "Exported %1 item(s)").arg(dataS.count()));

        }else
            QMessageBox::warning( dialog, tr("Export.."),
                        tr("Error while export, cannot write file"));

        return;

    }else
        usersD << cbTarget->currentData().toInt();


    foreach( int u, usersD )
        foreach( QString ds, dataS.keys() ){

            QString qry = QString(
                "INSERT INTO accache (idruser, identry, typentry, created, dataentry)  "
                "   VALUES (%1, '%2', 'test', now(), '%3') "
                "   ON DUPLICATE KEY UPDATE created = now(), dataentry='%3' ");

            mysql_query( mysql,
                qry.arg( u ).arg( ds ).arg( dataS[ds] )
                    .toStdString().c_str() );

            int afr = mysql_affected_rows( mysql );

            qDebug() << "Affected rows:" << afr;
        }


    QMessageBox::information(pmpDialog, tr("Export.."),
        tr( "Exported %1 item(s)").arg(dataS.count())
                     + ((usersD.count()>1)
                        ? tr(".\nFor %1 users").arg(usersD.count())
                        : tr(".\nFor user '%1'").arg(users[ usersD.first() ])) );


    if (sCode > 0)
        onSourceSelected();

    onTargetSelected();
}
*/


void RiTApplication::onDataPump(){

    StringHash  dataS;
    QList <int> usersD;
    QList <int> entryR; // for removing
    QTreeWidgetItem *i;

    if ( treeListCheckedItems( lsSource ).empty() ){

        if (lsSource->topLevelItem(0)->childCount())
            for( int c=0; c < lsSource->topLevelItem(0)->childCount(); c++ )
                lsSource->topLevelItem(0)->child(c)->setCheckState( 0, Qt::Checked );
        else
            onSourceSelected();
    }

    bool cancelItem;
    int  d;
    int  tCode = cbTarget->currentData().toInt();
    int  sCode = cbSource->currentData().toInt();

    foreach( i, treeListCheckedItems( lsSource ) ){

        cancelItem = false;

        for(d=0; d < lsTarget->count(); d++){       // find first item with same text
                                                    //
            if (lsTarget->item(d)->text() == i->text(0)){

                cancelItem = QMessageBox::No
                        == QMessageBox::warning(
                                pmpDialog, tr("Export.."),
                                tr("'%1' entry already exists. Continue operation?").arg(i->text(0)),
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No );
                break;
            }
        }

        if (cancelItem) continue;

        QString iData = (sCode == SC_IFILE)
                                ? i->data( 0, SR_DATA ).toString()
                                : base64Encode( getEntryData( i->data( 0, SR_ACID ).toInt()) );

        dataS[ i->text(0) ] = iData;

        entryR << i->data( 0, SR_ACID ).toInt();

        qDebug() << dataS[ i->text(0) ];
    }


    if ( TC_MUSERS == tCode ){
        foreach( QListWidgetItem *i, lsTarget->selectedItems() )
            usersD << i->data( Qt::UserRole ).toInt();
    }else
//    if ( TC_REMOVE == tCode ){

//        if ((SC_IFILE == sCode)
//        ||  entryR.empty())
//            return;

//        if ( QMessageBox::Cancel == QMessageBox::warning(
//                exDialog, tr("Removing"),
//                       tr("Entries will be removed.\n"
//                          "Do you want to complete?"),
//                       QMessageBox::Ok | QMessageBox::Cancel,
//                       QMessageBox::Cancel)
//             )return;


//        foreach( int r, entryR ){

//            QString qry = QString(
//                "DELETE FROM accache WHERE idaccache = %1 ");

//            mysql_query( mysql,
//                qry.arg( r ).toStdString().c_str() );

//            int afr = mysql_affected_rows( mysql );

//            qDebug() << "Affected rows:" << afr;

//            QMessageBox::information(exDialog, tr("Removing"),
//                        tr( "Removed %1 item(s)").arg(afr) );
//        }


//        onSourceSelected();
//        onTargetSelected();

//        return;

//    }else
    if ( TC_EFILE == tCode ){

        QString fileName = QFileDialog::getSaveFileName(
                    pmpDialog,  tr("Export File"), "test.xrt",
                    tr("DNA-Technology tests (*.xrt);;XML files (*.xml)"));

        if (fileName.isEmpty()) return;

        QDomDocument doc("TESTs"), frag("item");
        QDomElement root;

        doc.appendChild(root = doc.createElement("TESTs"));

        foreach( QString ds, dataS.keys() ){
//            QString dd( QByteArray().fromBase64( dataS[ds].toUtf8() ));

            QString dd = base64Decode( dataS[ds].toUtf8() );

            frag.setContent( dd )
            && root.appendChild( frag.firstChild() ).isNull();
        }

        QFile       fileO; fileO.setFileName(fileName);
        QTextStream streamO( &fileO );

        if (fileO.open(QIODevice::WriteOnly|QIODevice::Truncate)){
            doc.save(streamO, 1);
            fileO.close();

            QMessageBox::information( dialog, tr("Export.."),
                        tr( "Operation completed" ) + ".\n" +
                        tr( "Exported %1 item(s)").arg(dataS.count()));

        }else
            QMessageBox::warning( dialog, tr("Export.."),
                        tr("Error while export, cannot write file"));

        return;

    }else
        usersD << cbTarget->currentData().toInt();


    RiTUser      *usr = 0;
    RiTEntryCache *ce = 0;

    foreach( int u, usersD ){

        if(!(usr = userObject(u))) continue;

        int afr = 0;

        foreach( QString ds, dataS.keys() ){

            if ( ce = usr->addCacheEntry( ds, "test" ) ){
                 ce->setData( dataS[ds] );
                afr++;
            }
        }

        qDebug() << "Affected rows:" << afr;
    }


//    foreach( int u, usersD )
//        foreach( QString ds, dataS.keys() ){

//            QString qry = QString(
//                "INSERT INTO accache (idruser, identry, typentry, created, dataentry)  "
//                "   VALUES (%1, '%2', 'test', now(), '%3') "
//                "   ON DUPLICATE KEY UPDATE created = now(), dataentry='%3' ");

//            mysql_query( mysql,
//                qry.arg( u ).arg( ds ).arg( dataS[ds] )
//                    .toStdString().c_str() );

//            int afr = mysql_affected_rows( mysql );

//            qDebug() << "Affected rows:" << afr;
//        }


    QMessageBox::information(pmpDialog, tr("Export.."),
        tr( "Exported %1 item(s)").arg(dataS.count())
                     + ((usersD.count()>1)
                        ? tr(".\nFor %1 users").arg(usersD.count())
                        : tr(".\nFor user '%1'").arg(users[ usersD.first() ])) );


    if (sCode > 0)
        onSourceSelected();

    onTargetSelected();
}


void RiTApplication::onDataAccess(){

    QList <int> usersD;
    QList <int> entryN; // native items for grant access
    QList <int> entryF; // foreign items for revoke access
    QTreeWidgetItem *i;

    if ( treeListCheckedItems( lsSource ).empty() ){

        if (lsSource->topLevelItem(0)->childCount())
            for( int c=0; c < lsSource->topLevelItem(0)->childCount(); c++ )
                lsSource->topLevelItem(0)->child(c)->setCheckState( 0, Qt::Checked );
        else
            onSourceSelected();
    }

    int tCode = cbTarget->currentData().toInt();
    int sCode = cbSource->currentData().toInt();


    foreach( i, treeListCheckedItems( lsSource ) ){
        if (  sCode == i->data( 0, SR_USR  ).toInt() )
             entryN << i->data( 0, SR_ACID ).toInt();
        else entryF << i->data( 0, SR_ACID ).toInt();
    }


    if ( TC_EFILE == tCode ){
        return;
    }else
    if ( TC_MUSERS == tCode ){
        foreach( QListWidgetItem *i, lsTarget->selectedItems() )
            usersD << i->data( Qt::UserRole ).toInt();
    }else   usersD << tCode;


    int qb = QMessageBox::question(pmpDialog, tr("Share data"),
                    tr("Please select Share access operation"),
                    QMessageBox::Open | QMessageBox::Close | QMessageBox::Abort,
                    QMessageBox::Abort);

    if (qb == QMessageBox::Abort)
        return;

    bool    opGrant = (qb == QMessageBox::Open) && !entryN.empty();
    bool    opRevoke= (qb == QMessageBox::Close)&&(!entryF.empty() || !entryN.empty());


    if (opGrant){

        if ( usersD.count() == lsTarget->selectedItems().count() ) // grant for all
            foreach(int e, entryN)
                setEntryPermitions( e, 1 );
        else
            foreach(int e, entryN)
                foreach( int u, usersD)
                    if ( u != sCode )
                        setEntryPermitions( e, 1, u );
    }


    if (opRevoke){

        if (!entryN.empty()){

            if ( usersD.count() == lsTarget->selectedItems().count() ) // revoke for all
                setEntryPermitions( entryN.first(), 0 );

            else foreach( int u, usersD)
                    if ( u != sCode )
                        setEntryPermitions( entryN.first(), 0, u );
        }else
        if (!entryF.empty())
                setEntryPermitions( entryF.first(), 0, sCode );
    }


//    QMessageBox::information(pmpDialog, tr("Export.."),
//        tr( "Exported %1 item(s)").arg(dataS.count())
//                     + ((usersD.count()>1)
//                        ? tr(".\nFor %1 users").arg(usersD.count())
//                        : tr(".\nFor user '%1'").arg(users[ usersD.first() ])) );


    if (sCode > 0)
        onSourceSelected();

    onTargetSelected();
}


StringHash RiTApplication::importEntries( QString filePath ){

    QDomDocument doc("TESTs");
    StringHash retHash;

    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return  retHash;
    if (!doc.setContent(&file)) {
        file.close();
        return  retHash;
    }
    file.close();

    QDomElement  docElem = doc.documentElement();
    QDomNode     n;
    QDomElement  nt;
    QString      nv;
    QDomElement  e;
    QDomNodeList items;

    if ( "tests" == docElem.tagName().toLower() )
         items = docElem.elementsByTagName("item");
    else
    if ( "protocol" == docElem.tagName() ){
         items = docElem.elementsByTagName("tests");

         if (items.count())
             items = items.at(0).toElement().elementsByTagName("item");
         else items= QDomNodeList();
    }


//    qDebug() << "tagName" << docElem.tagName();

    for( int i=0; i < items.count(); i++  ){

        n = items.at(i);

        if(!(e = n.toElement()).isNull()) {

            nv = "";

            QTextStream streamNode(&nv);
                e.save( streamNode, 0 );

            if (!(nt = e.firstChildElement("nameTest")).isNull()){
                retHash[ nt.text() ] = base64Encode( nv );
            }
        }
    }

    return  retHash;
}

/*
void RiTApplication::onSourceSelected(){

    lsSource->clear();

    QListWidgetItem *i;

    int idSource = cbSource->currentData().toInt();

    if (idSource == SC_IFILE) {

        QString fileName = QFileDialog::getOpenFileName(
                    pmpDialog,  tr("Open File"),
                    QString(), tr("DNA-Technology realtime (*.xrt *.rt);;XML files (*.xml)"));

        if (QFile::exists( fileName )){
            QHash <QString, QString> tests = importEntries(fileName);

            foreach(QString t, tests.keys()){

                i = new QListWidgetItem( t );
                i->setData( Qt::UserRole, tests[t] );
                i->setData(1+Qt::UserRole,0 );

//                i->setFlags( i->flags() | Qt::ItemIsUserCheckable );
//                i->setCheckState(Qt::Unchecked);

                lsSource->addItem( i );
            }

            lsSource->selectAll();

        }else
            cbSource->setCurrentIndex(0);

        return;
    }


    mysql_query( mysql,
        QString("SELECT idaccache, identry, dataentry FROM accache "
                "WHERE idruser=%1 and typentry='test'" )
                    .arg( idSource ).toStdString().c_str());

    results = mysql_store_result(mysql);

    while((record = mysql_fetch_row(results))){
        i = new QListWidgetItem(  QString(record[1]) );
        i->setData(  Qt::UserRole,QString(record[2]) );
        i->setData(1+Qt::UserRole,QString(record[0]) );

        i->setFlags( i->flags() | Qt::ItemIsUserCheckable );
        i->setCheckState(Qt::Unchecked);

        lsSource->addItem( i );
    }

    mysql_free_result(results);
}
*/

void RiTApplication::onSourceSelected(){

    QTreeWidgetItem* rootItem = lsSource->topLevelItem(0);

    while( rootItem->childCount() )
           rootItem->removeChild( rootItem->child(0) );

    rootItem->setCheckState(0, Qt::Unchecked);

    QTreeWidgetItem *i;

    int idSource = cbSource->currentData().toInt();

    if (idSource == SC_IFILE) {

        QString fileName = QFileDialog::getOpenFileName(
                    pmpDialog,  tr("Open File"),
                    QString(), tr("DNA-Technology realtime (*.xrt *.rt);;XML files (*.xml)"));

        if (QFile::exists( fileName )){
            StringHash tests = importEntries(fileName);

            foreach(QString t, tests.keys()){

                i = new QTreeWidgetItem( QStringList() << t );
                i->setData( 0,  Qt::UserRole, tests[t] );
                i->setData( 0,1+Qt::UserRole, 0 );

//                i->setFlags( i->flags() | Qt::ItemIsUserCheckable );
                i->setCheckState( 0, Qt::Unchecked );

                rootItem->addChild( i );
            }

//            lsSource->selectAll();

        }else
            cbSource->setCurrentIndex(0);

//        lsSource->expandAll();

        return;
    }


    QList< RiTEntryCache* > entries;

    dbReadUserCache( idSource, &entries );

    QFont   bfnt= QFont();  bfnt.setBold(true);
    QFont   ifnt= QFont();  ifnt.setItalic(true);

    foreach( RiTEntryCache* e, entries ){

        if (e->typeEntry() != "test")
            continue;

        i = new QTreeWidgetItem( QStringList() << e->uinIncome() );
//        i->setData( 0, SR_DATA, getEntryData( e ) );
        i->setData( 0, SR_ACID, e->idEntry() );
        i->setData( 0, SR_USR,  e->idOwner() );

//        i->setFlags( i->flags() | Qt::ItemIsUserCheckable );
        i->setCheckState( 0, Qt::Unchecked );

        if ( idSource != e->idOwner() ) i->setData( 0, Qt::FontRole, bfnt );
        else  if ( e->permitionsAll() ) i->setData( 0, Qt::FontRole, ifnt );

        rootItem->addChild( i );
    }

    lsSource->expandAll();

    qDeleteAll( entries );
                entries.clear();

//    QString qry;

//    mysql_query( mysql,
//        (qry = QString(
//                " SELECT idaccache, identry, idruser, dataentry FROM accache "
//                " WHERE typentry='test' and (idruser=%1 or permaskall)"
//                " UNION "
//                " SELECT s.idaccache, e.identry, e.idruser, e.dataentry"
//                " FROM   sharcache s"
//                " inner join accache e ON (s.idruser=%1 and s.idaccache = e.idaccache and s.permask)")
//                    .arg( idSource )).toStdString().c_str());


//    results = mysql_store_result(mysql);

//    QFont   bfnt = QFont();
//            bfnt.setBold(true);

//    while((record = mysql_fetch_row(results))){

//        i = new QTreeWidgetItem( QStringList() << QString(record[1]) );
//        i->setData( 0, SR_DATA, QString(record[3]) );
//        i->setData( 0, SR_ACID, QString(record[0]) );
//        i->setData( 0, SR_USR,  QString(record[2]) );

////        i->setFlags( i->flags() | Qt::ItemIsUserCheckable );
//        i->setCheckState( 0, Qt::Unchecked );

//        if ( idSource != QString(record[2]).toInt() )
//            i->setData( 0, Qt::FontRole, bfnt );

//        lsSource->topLevelItem(0)->addChild( i );
//    }

//    lsSource->expandAll();

//    mysql_free_result(results);
}


void RiTApplication::onTargetSelected(){

    lsTarget->clear();

    int idTarget = cbTarget->currentData().toInt();
//    if (idTarget == -1) {

//        foreach( int c, users.keys() ){
//            QListWidgetItem *i = new QListWidgetItem( users[c] );
//                             i->setData( Qt::UserRole,      c  );
//            lsTarget->addItem( i );
//        }
//        lsTarget->selectAll();
//        return;

//    }else
//    if (idTarget == -2)
//        return;

    switch( idTarget ){
    case TC_MUSERS:
        foreach( int c, users.keys() ){
            QListWidgetItem *i = new QListWidgetItem( users[c] );
                             i->setData( Qt::UserRole,      c  );
            lsTarget->addItem( i );
        }
        lsTarget->selectAll();
        return;
    break;

    case TC_EFILE:
    case TC_REMOVE:
        return;
    }


    mysql_query( mysql,
        QString(" SELECT idaccache, identry, idruser, permaskall FROM accache "
                " WHERE typentry='test' and (idruser=%1 or permaskall)"
                " UNION "
                " SELECT s.idaccache, e.identry, e.idruser, 0"
                " FROM   sharcache s"
                " inner join accache e ON (s.idruser=%1 and s.idaccache = e.idaccache and s.permask)"
                ).arg( idTarget ).toStdString().c_str());



    QFont   bfnt = QFont(); bfnt.setBold(true);
    QFont   ifnt = QFont(); ifnt.setItalic(true);

    if ( results  = mysql_store_result(mysql))
    while((record = mysql_fetch_row(results))){
        QListWidgetItem *i = new QListWidgetItem( QString(record[1]) );
                         i->setData(  Qt::UserRole,QString(record[0]).toInt() );
                         i->setData(1+Qt::UserRole,QString(record[2]).toInt() );

        if ( idTarget != QString(record[2]).toInt() ) i->setData( Qt::FontRole, bfnt );
        else        if ( QString(record[3]).toInt() ) i->setData( Qt::FontRole, ifnt );


        lsTarget->addItem( i );
    }

    mysql_free_result(results);
}


void RiTApplication::onSourceItemChanged(QTreeWidgetItem *si, int){

    lsSource->blockSignals(true);

    if (si == lsSource->topLevelItem(0)){

        for (int c=0; c < si->childCount(); c++)
            si->child(c)->setCheckState(0, si->checkState(0));

    }else{

        bool sAll = treeListCheckedItems(lsSource).count()
                == lsSource->topLevelItem(0)->childCount();

        lsSource->topLevelItem(0)->setCheckState(0,
                    sAll ? Qt::Checked : Qt::Unchecked );
    }

    lsSource->blockSignals(false);
}



bool RiTApplication::isVisible() const{

    return dialog && dialog->isVisible();
}

QTranslator RiTApplication::qtTranslator;
QTranslator RiTApplication::qtBaseTranslator;
QTranslator RiTApplication::ritaTranslator;

int RiTApplication::setLocale(QString appLang) const{

    int retVal = 0;

    QString rtT = "rita_" + appLang;
    QString tDir= //app->applicationDirPath()+"/translations";
                    baseDir + "/translations";

//    if( ritaTranslator->load( rtT, tDir ) ){
//        QTranslator qtTranslator;
//        QTranslator qtBaseTranslator;

    if( ritaTranslator.load( rtT, tDir ) ){

        QString     tnm;
        bool lok = true;

        lok &= qtBaseTranslator.load( tnm = "qtbase_" + appLang, tDir );
        lok &= app->installTranslator(  &qtBaseTranslator );

        lok &= qtTranslator.load( tnm = "qt_" + appLang, tDir );
        lok &= app->installTranslator(  &qtTranslator );

        qDebug() << "Load translators result:" << lok;

        //retVal = app->installTranslator( ritaTranslator );
        retVal = app->installTranslator( &ritaTranslator );
        settings->setValue("main/locale", appLang);

    }else{

        qDebug() << QString("Error while loading translator %1").arg(rtT);
        settings->setValue("main/locale", "en");
    }

    return  retVal;
}


int RiTApplication::setOption(const QString oName, const QString value){

    QStringList defOptions;
    defOptions << "locale"          << "fontcss"        // 0, 1
               << ""                << ""               // 2, 3
               << "clientpid"       << "heartbeatrate"; // 4, 5

    QString name = oName.toLower();

    if (!defOptions.contains(name))
        return -1;

    if (name == defOptions[0])        // locale
        setLocale( value );
    else
    if (name == defOptions[4])        // clientpid
        startHeartBeat( value.toLongLong() );
    else
    if (name == defOptions[5])        // heartbeatrate
        startHeartBeat( pidClient, value.toInt() );
    else
        setProperty( CQSTR(name), value );

    return  1;
}


bool RiTApplication::setRiTProp(int uid, QString p, QString v)
{
    if ( !props.contains(p) ){
        qDebug() << "Cannot find this property"
                 << p << "in properties bag";
        return false;
    }

    QString qry = QString(
                "INSERT INTO accprop (idruser, prop, propval)  "
                "   VALUES (%1, '%2', '%3')"
                "   ON DUPLICATE KEY UPDATE propval = '%3' " );

    mysql_query( mysql,
//        qry.arg( uid ).arg( p ).arg( v )
        CQSTR( qry.arg( uid ).arg( p ).arg( str_escape( v ) ) ) );

//    qDebug() << qry.arg( uid ).arg( p ).arg( str_escape( v ) ) ;

    int afr = mysql_affected_rows( mysql );

    qDebug() << "Affected rows:" << afr;

    return  afr>=0;
}


bool RiTApplication::setRiTPriv( int uid, QString p, bool v){

    if ( !privs.contains(p) ){

        if (p.length())
            qDebug()<< "Cannot find this privilege("
                    << p << ")in privileges bag";

        return  false;
    }

    if (p.startsWith("@"))
        registerPPrivilege( p, v );

    QString qry = QString(
        "INSERT INTO accpriv (idruser, priv, prival)  "
        "   VALUES (%1, '%2', %3)"
        "   ON DUPLICATE KEY UPDATE prival = %3 " );

    mysql_query( mysql,
        CQSTR( qry.arg( uid ).arg( p ).arg( v ) ) );

    return  0 <= mysql_affected_rows( mysql );
}



bool RiTApplication::setEntryPermitions(int e, int p, int u) const {

    QString qry =
          u ? QString(  "INSERT INTO sharcache (permask, idaccache, idruser )  "
                        "   VALUES (%1, %2, %3) "
                        "   ON DUPLICATE KEY UPDATE permask = %1")
            : QString(  "UPDATE accache a "
                        " LEFT OUTER JOIN sharcache s "
                        "   ON (a.idaccache = s.idaccache) "
                        "   SET a.permaskall= %1, s.permask = %1 "
                        " WHERE a.idaccache = %2");
    mysql_query( mysql,
        CQSTR( qry.arg( p ).arg( e ).arg( u ) ) );

    return  true;
}



void RiTApplication::quit(){

    if ( hbTimer ) killTimer( hbTimer );

    users.clear();

    if( dialog )
        dialog->deleteLater();

    dialog = 0;

    if ( !--ritAppCounter ){
        dbDisconnect();
        QTimer::singleShot(0, qApp, SLOT(quit()));
    }
}



//RiTUser* RiTApplication::currentLogin() const{
RiTUser* RiTApplication::currentLogin(){

    if (!isDBConnected || !_currentLogin)
        return  0;

    dbReadUser(  _currentLogin->userId(),
                &_currentLogin );

    return  _currentLogin;

//    return isDBConnected
//            ? ( dialog  ? (dialog->result() ? _currentLogin : 0)
//                        : _currentLogin )
//            : 0;
}


bool RiTApplication::isConnected(){

    return isDBConnected;
}


RiTUser::RiTUser(QObject *p)
: QObject(p){
}


bool RiTUser::setRiTUser(int uid, QString name, bool adm, QString crdate){

    _uid        = uid;    _userName   = name;
    _isadmin    = adm;    _created    = crdate;

    return  (uid>=0) && name.length();
}


RiTUser::~RiTUser(){

    while( _cache.size() )
        delete _cache.take( _cache.keys().last() );
}


RiTApplication *RiTUser::application() const{
    return qobject_cast<RiTApplication*>(parent());
}


QString RiTUser::userName() const{
    return  _userName;
}

bool RiTUser::isAdmin() const{
    return  _isadmin;
}

QString RiTUser::dtCreated() const{
    return  _created;
}

int RiTUser::userId() const{
    return  _uid;
}


QString RiTUser::getAttr(QString p){

    return _prop.contains( p ) ? _prop[p] : "";
}


bool RiTUser::setAttr(QString p, QString v){

    bool    est = _prop.contains( p );
                  _prop[p] = v;

    application()->setRiTProp( userId(), p, v );

    return  est;
}


bool RiTUser::getPriv(QString p){

    return _priv.contains( p ) ? _priv[p] : "";
}



QStringList RiTUser::getCache(QString t) const{

    QStringList retList;

    foreach( RiTEntryCache *e, _cache )
        if ( t.isEmpty() ||(e->typeEntry() == t) )
            retList << QString().number( e->idEntry() );

    return  retList;
}


RiTEntryCache* RiTUser::addCacheEntry(QString eUin, QString eType){

    RiTEntryCache* ex;

    if ((ex = getCacheEntryIndex( eUin, eType, _uid ))
    &&  !delCacheEntry(ex->idEntry()))
        return  0;

    QString qry = QString(
        "INSERT INTO accache (idruser, identry, typentry, created)  "
        "  VALUES (%1, '%2', '%3', now()) "
        "  ON DUPLICATE KEY UPDATE created = now() " );

    mysql_query( mysql,
        CQSTR( qry.arg( userId() ).arg( eUin ).arg( eType ) ) );

//    int afr = mysql_affected_rows( mysql );
//    qDebug() << "Affected rows:" << afr;

    updateCache();

    return  getCacheEntryIndex( eUin, eType, _uid );
}


bool RiTUser::delCacheEntry(int eId){

    QString qry = QString(
        "DELETE FROM accache WHERE idaccache = %1" );

    mysql_query( mysql, CQSTR( qry.arg( eId ) ) );

//    int afr = mysql_affected_rows( mysql );
//    qDebug() << "Affected rows:" << afr;

    updateCache();

    return  0 == getCacheEntry( eId );
}



int RiTUser::updateCache(){

//    int lim = _cache.size() + 10;

    qDeleteAll(_cache);

//    while( !_cache.isEmpty() ){
//        delete _cache.take( _cache.keys().first() );
//        if (!--lim) break;
//    }

    _indexCache.clear();
         _cache.clear();

    RiTUser* thisUser = this;

    application()->dbReadUserCache( &thisUser );

    return  _cache.size();
}



RiTEntryCache *RiTUser::getCacheEntry(int eId){// const{

    return  _cache.contains(eId) ? _cache[eId] : 0;
}

RiTEntryCache *RiTUser::getCacheEntryIndex(QString eId, QString eType, int owId) {

    QString beix = BEIX(eId, eType, owId? owId: _uid);

    return      _indexCache.contains( beix )
            &&  _cache.contains( _indexCache[ beix ] )
                        ?_cache[ _indexCache[ beix ] ]
                        : 0 ;

//    return      _indexCache.contains( eId+eType )
//            &&  _cache.contains( _indexCache[ eId+eType] )
//                        ?_cache[ _indexCache[ eId+eType] ]
//                        : 0 ;
}



QStringList RiTUser::attrNames() const{
    QStringList sl;
    foreach (QString p, _prop.keys()) {sl << p;}
    return  sl;
}

QStringList RiTUser::privNames() const{
    QStringList sl;
    foreach (QString p, _priv.keys()) {sl << p;}

    qSort( sl );
    return sl ;
}


RiTEntryCache::RiTEntryCache(QObject *p)
    : QObject(p){
}

RiTEntryCache::~RiTEntryCache(){
}

RiTUser *RiTEntryCache::user() const{

    return qobject_cast<RiTUser*>(parent());
}

int RiTEntryCache::idOwner() const
{
    return _owner;
}

int RiTEntryCache::idEntry() const{
    return _entry;
}

QString RiTEntryCache::uinIncome() const{
    return _uin;
}

QString RiTEntryCache::typeEntry() const{
    return _type;
}

QString RiTEntryCache::dtCreated() const{
    return _created;
}

int RiTEntryCache::permitions() const{

    return  _permAll | _perm ;
}

int RiTEntryCache::permitionsAll() const{
    return _permAll;
}

bool RiTEntryCache::setRiTEntry(int e, QString u, QString t, QString c, int pa, int p, int ow){

    _entry  = e;    _uin    = u;
    _type   = t;    _created= c;
    _permAll= pa;   _perm   = p;
    _owner  = ow;

    return true;
}

QString RiTEntryCache::getData(){
    return  user()->application()->getEntryData( this );
}

void RiTEntryCache::setData(QString d){
    user()->application()->setEntryData( this, d );
}

bool RiTEntryCache::setPermitionsUser(int p, int u){
    bool pch =  ( user()->isAdmin() || user()->userId() == _owner )
                ? user()->application()->setEntryPermitions( _entry, p, u )
                : false;
    if(!u && pch)_permAll = p;
    return   pch;
}

bool RiTEntryCache::setPermitionsAll(int p){
    return setPermitionsUser(p);
}
