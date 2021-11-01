-- MySQL Administrator dump 1.4
--
-- ------------------------------------------------------
-- Server version	5.5.47


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


--
-- Create schema rita
--

CREATE DATABASE IF NOT EXISTS rita;
USE rita;

--
-- Definition of table `accpriv`
--

DROP TABLE IF EXISTS `accpriv`;
CREATE TABLE `accpriv` (
  `idaccpriv` int(11) NOT NULL AUTO_INCREMENT,
  `idruser` int(11) NOT NULL,
  `priv` char(255) NOT NULL,
  `prival` tinyint(1) DEFAULT NULL,
  PRIMARY KEY (`idaccpriv`),
  UNIQUE KEY `uk_user_priv` (`idruser`,`priv`),
  KEY `fk_acc_priv` (`priv`),
  KEY `fk_acc_user` (`idruser`),
  CONSTRAINT `fk_acc_priv` FOREIGN KEY (`priv`) REFERENCES `rupriv` (`priv`) ON DELETE CASCADE ON UPDATE NO ACTION,
  CONSTRAINT `fk_acc_user` FOREIGN KEY (`idruser`) REFERENCES `ruser` (`idruser`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;




--
-- Definition of table `accprop`
--

DROP TABLE IF EXISTS `accprop`;
CREATE TABLE `accprop` (
  `idaccprop` int(11) NOT NULL AUTO_INCREMENT,
  `idruser` int(11) NOT NULL,
  `prop` char(20) NOT NULL,
  `propval` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`idaccprop`),
  UNIQUE KEY `uk_user_prop` (`idruser`,`prop`),
  KEY `fk_accprop_prop` (`prop`),
  KEY `fk_accprop_user` (`idruser`),
  CONSTRAINT `fk_accprop_prop` FOREIGN KEY (`prop`) REFERENCES `ruprop` (`prop`) ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT `fk_accprop_user` FOREIGN KEY (`idruser`) REFERENCES `ruser` (`idruser`) ON DELETE NO ACTION ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;



--
-- Definition of table `accache`
--

DROP TABLE IF EXISTS `accache`;
CREATE TABLE `accache` (
  `idaccache` int(11)  NOT NULL AUTO_INCREMENT,
  `idruser`   int(11)  NOT NULL,
  `identry`   char(64) NOT NULL,
  `typentry`  char(32) DEFAULT '',
  `created` datetime   DEFAULT NULL,
  `permaskall` int(11) NOT NULL DEFAULT 0,
  `dataentry` longtext,
  PRIMARY KEY (`idaccache`),
  KEY `fk_accache_idruser` (`idruser`),
  UNIQUE KEY `type_entry_UNIQUE` (`idruser`, `identry`, `typentry`),
  CONSTRAINT `fk_accache_idruser` FOREIGN KEY (`idruser`) REFERENCES `ruser` (`idruser`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;



--
-- Definition of table `sharcache`
--

DROP TABLE IF EXISTS `sharcache`;
CREATE TABLE `sharcache` (
  `idsharcache` int(11)  NOT NULL AUTO_INCREMENT,
  `idaccache`   int(11)  NOT NULL,
  `idruser`     int(11)  NOT NULL,
  `permask`     int(11)  NOT NULL,
  PRIMARY KEY (`idsharcache`),
  KEY `fk_sharcache_idaccache` (`idaccache`),
  KEY `fk_sharcache_idruser`   (`idruser`),
  UNIQUE KEY `accache_ruser_UNIQUE` (`idaccache`, `idruser`),
  CONSTRAINT `fk_sharcache_idaccache` FOREIGN KEY (`idaccache`) REFERENCES `accache` (`idaccache`) ON DELETE CASCADE ON UPDATE NO ACTION,
  CONSTRAINT `fk_sharcache_idruser`   FOREIGN KEY (`idruser`)   REFERENCES `ruser`   (`idruser`)   ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;



--
-- Definition of table `groupriv`
--

DROP TABLE IF EXISTS `groupriv`;
CREATE TABLE `groupriv` (
  `idgroupriv`  int(11)     NOT NULL,
  `grname`      char(255)   NOT NULL,
  PRIMARY KEY (`idgroupriv`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


--
-- Definition of table `rupriv`
--

DROP TABLE IF EXISTS `rupriv`;
CREATE TABLE `rupriv` (
  `priv`        char(255) NOT NULL,
  `idgroupriv`  int(11)  NOT NULL,
  `valdefault`  tinyint(1) NOT NULL,
  PRIMARY KEY (`priv`),
  KEY `fk_rupriv_groupriv` (`idgroupriv`),
  CONSTRAINT `fk_rupriv_groupriv` FOREIGN KEY (`idgroupriv`) REFERENCES `groupriv` (`idgroupriv`) ON DELETE CASCADE ON UPDATE NO ACTION
) ENGINE=InnoDB DEFAULT CHARSET=utf8;



--
-- Definition of table `ruprop`
--

DROP TABLE IF EXISTS `ruprop`;
CREATE TABLE `ruprop` (
  `prop` char(20) NOT NULL,
  `valdefault` varchar(255) DEFAULT NULL,
  PRIMARY KEY (`prop`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


--
-- Definition of table `ruser`
--

DROP TABLE IF EXISTS `ruser`;
CREATE TABLE `ruser` (
  `idruser` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(45) NOT NULL,
  `pwd` varchar(45) DEFAULT NULL,
  `created` datetime DEFAULT NULL,
  `admin` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`idruser`),
  UNIQUE KEY `name_UNIQUE` (`name`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


--
-- Dumping data for table `ruprop`
--

/*!40000 ALTER TABLE `ruprop` DISABLE KEYS */;
INSERT INTO `ruprop` (`prop`,`valdefault`) VALUES
  ('dir:PR_HOME','%ProgramData%/DTmaster/Users/%RITUSER%')
 ,('str:PR_EMAIL','');
/*!40000 ALTER TABLE `ruprop` ENABLE KEYS */;


--
-- Dumping data for table `groupriv`
--
/*!40000 ALTER TABLE `groupriv` DISABLE KEYS */;
INSERT INTO `groupriv` (`idgroupriv`, `grname`) VALUES
  (1, 'Common')
 /*,(2, 'Modify protocol')*/
 ,(2, 'Execution')
 /*,(3, 'Execution')*/
 ,(3, 'Interface')
 ,(4, 'Confirmations')
 ;
 /*!40000 ALTER TABLE `groupriv` ENABLE KEYS */;

/*
    update groupriv set grname='Execution' where idgroupriv = 2;
    update groupriv set grname='Interface' where idgroupriv = 3;
    insert groupriv (idgroupriv, grname) values (4, 'Confirmations');
*/

--
-- Dumping data for table `rupriv`
--
/*!40000 ALTER TABLE `rupriv` DISABLE KEYS */;
INSERT INTO `rupriv` (`priv`, `idgroupriv`, `valdefault`) VALUES
  ('IGNORE_EXPOSURE',   4, false)
 ,('IGNORE_PROGRAM',    4, false)
 ,('IGNORE_VOLUME',     4, false)
 ,('IGNORE_MIN_LEVEL',  4, false)

 ,('ENABLE_SELECT_FN',  3, false)
 ,('ENABLE_ADD_ANALYSIS',3,false)
 ,('ENABLE_PAGE_RUN',   3, true )
 ,('ENABLE_PAGE_SETUP', 3, true )
 ,('ENABLE_CROSSTABLE', 3, false)
 ,('COPY_ONLINE',       3, false)

 ,('CHANGE_APP_PREF',   2, false)
 ,('CHANGE_ASYS_PREF',  2, false)
 ,('CHANGE_DEVICE_PREF', 2, false)
 ,('CONTROL_RUN',       2, true )
 ,('EDIT_PROTOCOL',     2, false)
 ,('EDIT_TEST',         2, false)
 ,('MASK_DATA',         2, false)
 ,('COPY_BLOCK_TEST',   2, false)
 ,('ENABLE_CMD',        2, false)
 ,('SAVE_LOCATION_WIN', 2, false)
 ,('@SINGLE_LOGIN',     1, false)
 ;
 /*!40000 ALTER TABLE `rupriv` ENABLE KEYS */;

/*
    update rupriv set idgroupriv=2 where priv in(
        'CHANGE_DEVICE_PREF', 'CONTROL_RUN', 'SAVE_LOCATION_WIN' );

    delete from rupriv where priv not in (
        'IGNORE_EXPOSURE',      'IGNORE_PROGRAM',
        'IGNORE_VOLUME',        'IGNORE_MIN_LEVEL',
        'ENABLE_ADD_ANALYSIS'   'ENABLE_PAGE_RUN', 'ENABLE_SELECT_FN',
        'ENABLE_PAGE_SETUP',    'ENABLE_CROSSTABLE', 'COPY_ONLINE',
        'CHANGE_APP_PREF',       'CHANGE_ASYS_PREF',
        'CHANGE_DEVICE_PREF',    'CONTROL_RUN',
        'EDIT_PROTOCOL',         'EDIT_TEST',
        'MASK_DATA',             'COPY_BLOCK_TEST',
        'ENABLE_CMD',            'SAVE_LOCATION_WIN',
        '@SINGLE_LOGIN' )

    INSERT INTO `rupriv` (`priv`, `idgroupriv`, `valdefault`) VALUES
    ('IGNORE_EXPOSURE',   4, false)
   ,('IGNORE_PROGRAM',    4, false)
   ,('IGNORE_VOLUME',     4, false)
   ,('IGNORE_MIN_LEVEL',  4, false)
   ,('ENABLE_ADD_ANALYSIS',3,false), ('ENABLE_SELECT_FN',3,false)
   ,('ENABLE_PAGE_RUN',   3, false), ('COPY_ONLINE',3,false)
   ,('ENABLE_PAGE_SETUP', 3, false)
   ,('ENABLE_CROSSTABLE', 3, false)
   ,('ENABLE_CMD',        2, false)
   ,('SAVE_LOCATION_WIN', 2, true )
   ON DUPLICATE KEY UPDATE valdefault = valdefault
   ;


*/

--
-- Dumping data for table `ruser`
--

/*!40000 ALTER TABLE `ruser` DISABLE KEYS */;
INSERT INTO `ruser` (`name`,`pwd`,`created`,`admin`) VALUES
 ('admin1','*23AE809DDACAF96AF0FD78ED04B6A265E05AA257','2016-12-14 16:40:00',1)
,('guest','','2016-12-14 16:41:00',0);


/*!40000 ALTER TABLE `ruser` ENABLE KEYS */;



--
-- Dumping data for table `accprop`
--

/*!40000 ALTER TABLE `accprop` DISABLE KEYS */;
INSERT INTO `accprop` (`idruser`,`prop`,`propval`) VALUES
 (1,'dir:PR_HOME','');
/*!40000 ALTER TABLE `accprop` ENABLE KEYS */;


--
-- Dumping data for table `accpriv`
--

/*!40000 ALTER TABLE `accpriv` DISABLE KEYS */;
INSERT INTO `accpriv` (`idruser`,`priv`,`prival`) VALUES
 (1, 'CHANGE_APP_PREF',     true)
,(1, 'CHANGE_ASYS_PREF',    true)
,(1, 'CHANGE_DEVICE_PREF',  true)
,(1, 'CONTROL_RUN',         true)
,(1, 'EDIT_PROTOCOL',       true)
,(1, 'EDIT_TEST',           true)
,(1, 'MASK_DATA',           true)
,(1, 'COPY_BLOCK_TEST',     true)

,(1, 'IGNORE_EXPOSURE',    false)
,(1, 'IGNORE_PROGRAM',     false)
,(1, 'IGNORE_VOLUME',      false)
,(1, 'IGNORE_MIN_LEVEL',   false)

,(1, 'ENABLE_SELECT_FN',  false)
,(1, 'ENABLE_ADD_ANALYSIS',true)
,(1, 'ENABLE_PAGE_RUN',    true)
,(1, 'ENABLE_PAGE_SETUP',  true)
,(1, 'ENABLE_CROSSTABLE',  true)
,(1, 'COPY_ONLINE',       false)


,(1, 'ENABLE_CMD',         true)
,(1, 'SAVE_LOCATION_WIN',  true)

,(1, '@SINGLE_LOGIN',      false);
/*!40000 ALTER TABLE `accpriv` ENABLE KEYS */;


/*
INSERT INTO `accpriv` (`idruser`,`priv`,`prival`)
select idruser, pn, pd
from ruser as u
left outer join (
          select 'IGNORE_EXPOSURE' as pn,   false as pd
    union select 'IGNORE_PROGRAM',          false
    union select 'IGNORE_VOLUME',           false
    union select 'ENABLE_SELECT_FN',        false
    union select 'IGNORE_MIN_LEVEL',        false
    union select 'ENABLE_ADD_ANALYSIS',     true
    union select 'ENABLE_PAGE_RUN',         true
    union select 'ENABLE_PAGE_SETUP',       true
    union select 'ENABLE_CROSSTABLE',       true
    union select 'COPY_ONLINE',       		true
    union select 'ENABLE_CMD',              true
    union select 'SAVE_LOCATION_WIN',       true
)as p on u.idruser=u.idruser
where pn not in ( select priv from accpriv )
*/


/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
