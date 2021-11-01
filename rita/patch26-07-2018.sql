ALTER TABLE `accache` ADD COLUMN `permaskall` int(11) NOT NULL DEFAULT 0 AFTER `created`;

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
