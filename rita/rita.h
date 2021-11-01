#ifndef RITA_H
#define RITA_H

#include <QWidget>
#include <QHash>
#include <QDialog>
#include <QCoreApplication>
#include <QTranslator>
#include <QVariant>
#include <QDateTime>

#define UP_CHPWD    "UP_CHPWD"

class RiTUser;
class RiTApplication;
class RiTEntryCache;

class QLabel;
class QComboBox;
class QLineEdit;
class QListWidget;
class QTreeWidget;
class QCheckBox;
class QStatusBar;
class QPushButton;
class QSettings;
class QListWidgetItem;
class QTreeWidgetItem;


typedef QList< QStringList >        ListRecords;
typedef QList< RiTEntryCache* >     RiTCaches;
typedef QHash< QString, RiTCaches > RiTUserTests;


class RiTEntryCache : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("ClassID",      "{1D217A8E-4081-41a7-8F6E-03F585C17DF2}")
    Q_CLASSINFO("InterfaceID",  "{04B254A8-8971-4e90-AB8E-2EB6EEA5A6DA}")

    Q_PROPERTY(RiTUser* user        READ user       )

    Q_PROPERTY(int      idEntry     READ idEntry    )
    Q_PROPERTY(int      idOwner     READ idOwner    )
    Q_PROPERTY(QString  uinIncome   READ uinIncome  )
    Q_PROPERTY(QString  typeEntry   READ typeEntry  )
    Q_PROPERTY(QString  dtCreated   READ dtCreated  )
    Q_PROPERTY(QString  dataEntry   READ getData        WRITE setData)
    Q_PROPERTY(int  permitionsAll   READ permitionsAll  WRITE setPermitionsAll)
    Q_PROPERTY(int     permitions   READ permitions)

    int       _entry;
    int       _owner;
    QString     _uin;
    QString    _type;
    QString _created;
    int     _permAll;
    int     _perm;

public:
    RiTEntryCache(QObject*);
    ~RiTEntryCache();

    RiTUser*     user()  const;
    int       idOwner()  const;
    int       idEntry()  const;
    QString uinIncome()  const;
    QString typeEntry()  const;
    QString dtCreated()  const;
    int     permitions() const;
    int     permitionsAll() const;

    bool    setRiTEntry(int, QString, QString, QString, int, int, int);

public slots:
    QString getData();
    void    setData(QString);

    bool    setPermitionsUser(int p, int u=0);
    bool    setPermitionsAll( int p );
};


//typedef QList<int> QIntList;
//typedef QList<int> SAFEARRAY(int);


class RiTUser : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("ClassID",      "{9B30F1F5-2082-406d-8557-20B888488824}")
    Q_CLASSINFO("InterfaceID",  "{D62C073C-011E-485f-8ED3-F8F6DB81E71D}")

    Q_PROPERTY(RiTApplication*  application READ application)

    Q_PROPERTY(int              userId      READ userId)
    Q_PROPERTY(bool             isAdmin     READ isAdmin)
    Q_PROPERTY(QString          userName    READ userName)
    Q_PROPERTY(QString          dtCreated   READ dtCreated)
    Q_PROPERTY(QStringList      attrNames   READ attrNames)
    Q_PROPERTY(QStringList      privNames   READ privNames)
//    Q_PROPERTY(QList<QVariant>  cache  READ cache)


    QString                     _userName;
    int                         _uid;
    bool                        _isadmin;
    QString                     _created;
    QHash <QString, bool>       _priv;
    QHash <QString, QString>    _prop;
    QHash <int, RiTEntryCache*> _cache;
    QHash <QString, int>        _indexCache;

public:

     RiTUser(QObject*);
    ~RiTUser();

    RiTApplication *application()   const;

    bool setRiTPriv(QString, bool);
    bool setRiTProp(QString, QString);
    bool setRiTEntry(int, QString, QString, QString, int, int, int);
    bool setRiTUser(int, QString, bool, QString);

    QString     userName()      const;
    int         userId()        const;
    bool        isAdmin()       const;
    QString     dtCreated()     const;
    QStringList attrNames()     const;
    QStringList privNames()     const;

public slots:
    QString     getAttr(QString);
    bool        setAttr(QString, QString);
    bool        getPriv(QString);

    QStringList getCache(QString t=QString()) const;

    RiTEntryCache*  getCacheEntry(int eId);//  const;
    RiTEntryCache*  getCacheEntryIndex(QString eId, QString eType, int owId=0);
    RiTEntryCache*  addCacheEntry(QString eUin, QString eType);
    bool            delCacheEntry(int eId);
    int             updateCache();
};


struct PrivFields {
    int     idgroup;
    QString group;
    bool    enabled;
};

PrivFields makePrivFields( int g, QString n, bool v );

class RiTApplication : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("ClassID",      "{14CB79BB-D11C-4e98-8CB8-450379A0B008}")
    Q_CLASSINFO("InterfaceID",  "{ED0BA98F-5410-4315-91CA-EA171C4642E2}")
    Q_CLASSINFO("EventsID",     "{A83C63EF-E970-40e8-85AB-C50458142031}")
    Q_CLASSINFO("RegisterObject", "yes")

    Q_PROPERTY(QString  id              READ id)
    Q_PROPERTY(RiTUser *currentLogin    READ currentLogin)
    Q_PROPERTY(QList<QVariant> usersIndex    READ usersIndex  )


    bool dbConnect();
    bool dbDisconnect();

    bool dbChkExists();
    bool dbBuild();

    int         hbTimer;
    qlonglong   pidClient;
    QDateTime   lastClentTrigger;

    bool        appIsAxServer;
    QCoreApplication     *app;
    QSettings   *settings;
    //QTranslator *ritaTranslator;
    static QTranslator ritaTranslator;
    static QTranslator qtTranslator;
    static QTranslator qtBaseTranslator;


    QHash <int, QString>        users;
    QHash <QString, QString>    props;
    QHash <QString, PrivFields> privs;

    static QHash <QString, QString>     trPriv;
    static QHash <QString, PrivFields>  ctmPriv;

    static QHash <QString, QString>     trProp;

    RiTUser* _currentLogin;
    RiTUser* _modifiedLogin;

    QFont       font;

    QDialog     *dialog;
    QDialog     *pmpDialog;
    QDialog     *muDialog;

    QPushButton *btEdit;
    QLabel      *cpName;
    QAction     *chxIsAdm;
//    QCheckBox   *chxIsAdm;
    QComboBox   *cbxUsers;
    QLineEdit   *edPasswd;
    QComboBox   *cxName;
    QLineEdit   *edPwd1;
    QLineEdit   *edPwd2;
    QListWidget *lsPerm;
    QListWidget *lsProp;
    QStatusBar  *sbInfo;

    QTreeWidget *lsSource;      QComboBox   *cbSource;
    QListWidget *lsTarget;      QComboBox   *cbTarget;

    int     dbSelectUsers();
    int     dbSelectProps();
    int     dbSelectPrivs();
    bool    translatePrivs();

    QHash <QString, QString>
            importEntries( QString filePath );

    bool    setCurrentLogin(RiTUser*, QString passwd);
    int     userCbxIndex(int userId);

    QList< QTreeWidgetItem *>
            treeListCheckedItems(QTreeWidget *tw);

    QList<QVariant>
            usersIndex() const;

    void acceptAppInput(WId);
    void initSettings();
    void initStyles();
//    void initFont();

public:

    static void setAppNameDomain(){
        QCoreApplication::setApplicationName    ("RITA");
        QCoreApplication::setOrganizationName   ("DNA-Technology");
        QCoreApplication::setOrganizationDomain ("dna-technology.ru");
    }


    RiTApplication(QObject *parent = 0);
    ~RiTApplication();

    QString      id() const { return objectName();  }
    QWidget* window() const { return dialog;        }

    bool    isConnected();

    bool    setRiTProp(int, QString, QString );
    bool    setRiTPriv(int, QString, bool);

    //RiTUser* currentLogin() const;
    RiTUser* currentLogin();

    int     dbReadUserCache(RiTUser**);
    int     dbReadUserCache(int u, QList<RiTEntryCache *> *entries);

    QString getEntryData( int );
    QString getEntryData( RiTEntryCache* );
    void    setEntryData( RiTEntryCache*, QString );
    bool    isDBConnected;
    QString baseDir;

    bool    clientProcessExists();

signals:
    void    afterDbConnected(bool);
    void    afterLogIn(bool);
    void    afterAdminLogIn(bool);
    void    usersSelected();

public slots:
    void    quit();    
    void    execute(bool isAx=true);
    bool    setupUser();

    bool      startHeartBeat(qlonglong pidC, int rate = 3000);
    void    triggerHeartBeat();

    void    logIn(bool isAx=true);
    void    logOut(bool isAx=true);

    void    dataPump();

    bool    isVisible() const;

    int     setLocale(QString) const;
    int     setOption(const QString name, const QString value);

//    int     setLogin(QString n, QString p=QString());

    bool    registerFPrivilege(
                    const QString name, PrivFields p );

    bool    registerPPrivilege(
                    const QString name, bool valDefault) const;

    bool    registerPrivilege(
                    const QString name, bool valDefault,
                             int igp,   const QString gname ) const;

    bool    trPrivilege(const QString name, const QString trans ) const;    
    bool    trProperty (const QString name, const QString trans ) const;

    RiTUser*    userObject(int);
    bool        setEntryPermitions(int e, int p, int u=0) const;


protected:
    void timerEvent(QTimerEvent *event);

private slots:
    void    buildLayout();
    void    editUserDialog();

    bool    viewProps();
    bool    viewPerms();

    bool    dbReadUser(int, RiTUser**);
    bool    saveRiTUser();
    bool    addRiTUser(QString uname=QString());
    bool    delRiTUser();

    bool    openPresetRiTUser();
    bool    savePresetRiTUser();

    bool    modifPerm(QListWidgetItem*);
    bool    modifProp();

    bool    tryAutoLogIn();
    bool    tryLogIn(bool autoLogin=false);

    void    onSourceSelected();
    void    onTargetSelected();

    void    onSourceItemChanged(QTreeWidgetItem*,int);

    void    onDataPump();
    void    onDataAccess();
    void    onDataRemove();

    void    onUsersSelected();
    
    void    openConsoleSQL();
    void    runSqlCommand();
    bool    createBackup(QString toolp, QString sp, QString dp);

    QString     searchRealTimePCR();
    QStringList loadRealTimePCREntries();
    void        importRiTUsers();
    void        dialog7ItemImportCompleted(QTreeWidgetItem *wi);
};


class RiSplashInfo : public QDialog
{
    Q_OBJECT

    QLabel  *label;
    int     tClose;
public:
        RiSplashInfo( QWidget * parent = 0
                    , QString msg=QString()
                    , int timeoutClose = 0);

        ~RiSplashInfo(){}

        void appendLabel(QString s);
        void setWindowFlag(Qt::WindowType, bool);

public slots:
        void        show();
        virtual int exec();
        void        hide();
};

#endif // RITA_H
