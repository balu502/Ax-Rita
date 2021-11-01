update groupriv set grname='Execution' where idgroupriv = 2;
update groupriv set grname='Interface' where idgroupriv = 3;
insert groupriv (idgroupriv, grname) values (4, 'Confirmations');

update rupriv set idgroupriv=2 where priv in(
	'CHANGE_DEVICE_PREF', 'CONTROL_RUN', 'SAVE_LOCATION_WIN' );

delete from rupriv where priv not in (
	'IGNORE_EXPOSURE',      'IGNORE_PROGRAM',
	'IGNORE_VOLUME',        'IGNORE_MIN_LEVEL',
	'ENABLE_ADD_ANALYSIS'   'ENABLE_PAGE_RUN',
	'ENABLE_PAGE_SETUP',    'ENABLE_CROSSTABLE',
	'CHANGE_APP_PREF',       'CHANGE_ASYS_PREF',
	'CHANGE_DEVICE_PREF',    'CONTROL_RUN',
	'EDIT_PROTOCOL',         'EDIT_TEST',
	'MASK_DATA',             'COPY_BLOCK_TEST',
	'ENABLE_CMD',            'SAVE_LOCATION_WIN',
	'@SINGLE_LOGIN' );

INSERT INTO `rupriv` (`priv`, `idgroupriv`, `valdefault`) VALUES
('IGNORE_EXPOSURE',   4, false)
,('IGNORE_PROGRAM',    4, false)
,('IGNORE_VOLUME',     4, false)
,('IGNORE_MIN_LEVEL',  4, false)
,('ENABLE_ADD_ANALYSIS',3,false)
,('ENABLE_PAGE_RUN',   3, false)
,('ENABLE_PAGE_SETUP', 3, false)
,('ENABLE_CROSSTABLE', 3, false)
,('ENABLE_CMD',        2, false)
,('SAVE_LOCATION_WIN', 2, true )
ON DUPLICATE KEY UPDATE valdefault = valdefault;

INSERT INTO `accpriv` (`idruser`,`priv`,`prival`)
select idruser, pn, pd
from ruser as u
left outer join (
          select 'IGNORE_EXPOSURE' as pn,   false as pd
    union select 'IGNORE_PROGRAM',          false
    union select 'IGNORE_VOLUME',           false
    union select 'IGNORE_MIN_LEVEL',        false
    union select 'ENABLE_ADD_ANALYSIS',     true
    union select 'ENABLE_PAGE_RUN',         true
    union select 'ENABLE_PAGE_SETUP',       true
    union select 'ENABLE_CROSSTABLE',       true
    union select 'ENABLE_CMD',              true
    union select 'SAVE_LOCATION_WIN',       true
)as p on u.idruser=u.idruser
where pn not in ( select priv from accpriv );