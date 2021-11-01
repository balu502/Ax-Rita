ALTER TABLE `accache` DROP COLUMN `permaskall`;
ALTER TABLE `accache` ADD COLUMN `permaskall` int(11) NOT NULL DEFAULT 0 AFTER `created`;
