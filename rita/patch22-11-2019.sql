DROP TABLE IF EXISTS `groupriv`;
CREATE TABLE `groupriv` (
  `idgroupriv`  int(11)     NOT NULL,
  `grname`      char(255)   NOT NULL,
  PRIMARY KEY (`idgroupriv`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*!40000 ALTER TABLE `groupriv` DISABLE KEYS */;
INSERT INTO `groupriv` (`idgroupriv`, `grname`) VALUES
  (1, 'Common')
 ,(2, 'Modify protocol')
 ,(3, 'Execution')
 ;
 /*!40000 ALTER TABLE `groupriv` ENABLE KEYS */;


/*!40000 ALTER TABLE `rupriv` DISABLE KEYS */;

ALTER TABLE `rupriv` ADD COLUMN `idgroupriv` int(11)  NOT NULL AFTER `priv`;

UPDATE `rupriv` SET `idgroupriv`=1 WHERE `priv` = '@SINGLE_LOGIN';
UPDATE `rupriv` SET `idgroupriv`=2 WHERE `priv` IN ( 'CHANGE_APP_PREF', 'CHANGE_ASYS_PREF',
    'CHANGE_DEVICE_PREF', 'EDIT_PROTOCOL', 'EDIT_TEST', 'MASK_DATA', 'COPY_BLOCK_TEST');
UPDATE `rupriv` SET `idgroupriv`=3 WHERE `priv` IN ( 'CONTROL_RUN', 'CHANGE_DEVICE_PREF');

ALTER TABLE `rupriv` ADD KEY `fk_rupriv_groupriv` (`idgroupriv`);
ALTER TABLE `rupriv` ADD CONSTRAINT `fk_rupriv_groupriv` FOREIGN KEY (`idgroupriv`) REFERENCES `groupriv` (`idgroupriv`) ON DELETE CASCADE ON UPDATE NO ACTION;

/*!40000 ALTER TABLE `rupriv` ENABLE KEYS */;
