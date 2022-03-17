====================
	RITA README
====================

RITA (RealTime Authentification)

1. Перед использованием производится регистрация COM сервера в системе путем запуска
	rita-register.bat
	или запуском rita.exe -regserver
	Регистрация проходит тихо, информационные сообщения выводятся 
	только при возникновении ошибок.
	
2. Проверка приложения. Просто запускается rita.exe, появляется окно интерфейса.
	Изначально есть, как минимум, два пользователя - администратор admin с паролем 123,
	простой пользователь user1 с паролем 111.
	Возможности администратора: 
		* может создавать новых пользователей,
		* может удалять пользователей,
		* может наделять правами администрирования других пользователей и отзывать их,
		* может изменять свой пароль,
		* не может снять с себя право администрирования (только через другого админа)
		* может изменять параметры других пользователей.
	Простой пользователь, возможности:
		* может изменять свой пароль.
		
3. Программный интерфейс.
	
	#include <QAxObject>
	//...
	//
	// Инициализация COM системы, выполняется 1 раз в начале программы
	//
	if ( FAILED( CoInitialize( NULL ))){
      qDebug() << "Unable to initialize COM";
      return -1;
    }
	
	//  Инициализация объекта по идентификатору
	//
	QAxObject 
		axRita;
		axRita.setControl( "{14CB79BB-D11C-4e98-8CB8-450379A0B008}" );   // class rita		
		
    if (axRita.control().isNull()){
        qDebug() << "Unable to set COM control";
        return -2;
    }
	

        //      Настройка локали (en|ru)
        //
        axRita.dynamicCall( "setLocale(QString)", "ru" );



	//	Вызов модального окна диалога
	//
	axRita.dynamicCall( "execute()" );
	
	// Если вошел админ, то можно открыть диалог импорта-обмена данными
	//
	axRita.dynamicCall( "dataPump()" );
	


        //  Если нужно объявить привилегию, которая отсутствует в системе:
        //
        //  - trPrivilege определяет перевод (интернационализация) привилеги по названию, с кодовым именем CUSTOM_PRIV
        //  - registerPrivilege регистрирует в системе эту привилегию CUSTOM_PRIV и значения по умолчанию
        //
        //  axRita.dynamicCall( "trPrivilege(QString,QString)",    "CUSTOM_PRIV", tr("My new privilege") );
        //  axRita.dynamicCall( "registerPrivilege(QString,bool)", "CUSTOM_PRIV", false );

        // Регистрация привилегии ALLOW_TEST1 (default=false) и группы Grp1 (индивидуальный код группы=4)
        //
        // axRita.dynamicCall("registerPrivilege(QString,bool,int,QString)", "ALLOW_TEST1", false, 4, "Grp1");


	//	Если был осуществлен удачный вход пользователя, то в 
	//	объекте *axUser можно получить о нем информацию
	//
    QAxObject*  axUser = axRita.querySubObject( "currentLogin" ) ;
	
    if ( !axUser ) return;
	
	// Чтение свойств объекта axUser
	//
        qDebug() << axUser->property( "userId" );		//	int		идентификатор пользователя
	qDebug() << axUser->property( "isAdmin" );		//	QString		признак администратора
	qDebug() << axUser->property( "userName" );		//	QString		логин пользователя
	qDebug() << axUser->property( "dtCreated" );	//	QString		дата регистрации
	qDebug() << axUser->property( "attrNames" );	//	QStringList	список названий атрибутов
	qDebug() << axUser->property( "privNames" );	//	QStringList	список названий привелегий


	// Чтение и запись атрибута PR_HOME у объекта axUser
	//
	qDebug() << axUser->dynamicCall( "getAttr(QString)", 			"PR_HOME" );
	qDebug() << axUser->dynamicCall( "setAttr(QString,QString)", 	"PR_HOME", "results/admin1" );
	
        qDebug() << axUser->dynamicCall( "getPriv(QString)",	"CHANGE_APP_PREF");	// привилегия изменения настроек приложения
        qDebug() << axUser->dynamicCall( "getPriv(QString)",	"CHANGE_ASYS_PREF");	// привилегия изменения настроек анализа
	qDebug() << axUser->dynamicCall( "getPriv(QString)",	"CHANGE_DEVICE_PREF");	// привилегия изменения настроек устройства
        qDebug() << axUser->dynamicCall( "getPriv(QString)",	"CONTROL_RUN");		// привилегия контроля запуска исполнения протокола на устройстве
        qDebug() << axUser->dynamicCall( "getPriv(QString)",	"EDIT_PROTOCOL");	// привилегия редактирования протокола
        qDebug() << axUser->dynamicCall( "getPriv(QString)",	"EDIT_TEST");		// привилегия редактирования теста
        qDebug() << axUser->dynamicCall( "getPriv(QString)",	"MASK_DATA");		// привилегия (reopen) наложения данных на протокол
        qDebug() << axUser->dynamicCall( "getPriv(QString)",	"SINGLE_LOGIN");	// включение однопользовательского режима работы приложения

	
	//	Получить кэш тестов, имеющийся у пользователя axUser. 
	//	Список содержит идентификаторы в строковом виде (из-за ограничений передачи списков в Qt)
	//
        QStringList	cache = axUser->dynamicCall("getCache(QString)", "test").toStringList();
	qDebug() << cache;
	
	//	Получение свойств первой записи из кэша.
	//
	if ( cache.size() ){
		
		QAxObject*  axItemCache = axUser->querySubObject( "getCacheEntry(int)", cache[0].toInt() );
		
		qDebug() << axItemCache->property( "idEntry" );		//	int			идентификатор записи
		qDebug() << axItemCache->property( "uinIncome" );	//	QString		внешний код-идентификатор (теста/протокола)
		qDebug() << axItemCache->property( "typeEntry" );	//	QString		тип записи (test|protocol|etc..)
		qDebug() << axItemCache->property( "dtCreated" );	//	QString		дата создания
		qDebug() << axItemCache->property( "dataEntry" );	//	QString		данные (xml-строка, или просто строка)

		// Изменение или ввод данных
		//
		axItemCache->setProperty( "dataEntry", "<xml..>...</xml>" );
		
		//	Если надо удалить запись кэша
		//
		axUser->dynamicCall("delCacheEntry(int)", cache[0].toInt());		
	}

	//	Запись из кэша можно найти по внешнему идентификатору и типу
	// 
	//	QAxObject*  axItemCache = axUser->querySubObject( "getCacheEntryIndex(QString,QString)", 
        //                                                          "some-ext-id", "test" );
	//	if( axItemCache ) ... чтение свойств и т.п.
	
	
	
	// Создание новой записи кэша - сначала указывается 
	// внешний код-идентификатор и тип записи, потом прикрепляются данные
	//
	QAxObject*  axItemCache = axUser->querySubObject( "addCacheEntry(QString,QString)",
                                                        "new-cache-id", "test" );														
	if( axItemCache )
		axItemCache->setProperty( "dataEntry", "<xml..>...</xml>" );
	
	
	//...
	//	Отключение COM системы в конце программы
	//
	CoUninitialize();


3.1 Работа с правами доступа к записи кэша

    ...
    QList<QVariant> uIx= axgp.property( "usersIndex" ).toList();  // получить идентификаторы пользователей

    QAxObject*  axUser = axgp.querySubObject( "userObject(int)", uIx[ 0 ].toInt() ) ;
                        //
                        // или, если интересует текущий пользователь
                        // = axgp.querySubObject( "currentLogin" ) ;

                        //  Получить список тестов (идентификаторы) у пользователя
                        //
    QStringList cIx = axUser->dynamicCall("getCache(QString)", "test");

                        // Получить 0-й объект из списка тестов
                        //
    QAxObject*  axCe = axlg->querySubObject( "getCacheEntry(int)", cIx[ 0 ].toInt() );


                        // Предоставить общий доступ к тесту (1-всем, 0-никому)
                        //
    qDebug() << axCe->dynamicCall( "setPermitionsAll(int)", 1 );

                        // Признак общего доступа
                        //
    qDebug() << axCe->property( "permitionsAll" );

                        // Идентификатор владельца,
                        // сравнивается с axUser->property( "userId" )
                        //
    qDebug() << axCe->property( "idOwner" );







