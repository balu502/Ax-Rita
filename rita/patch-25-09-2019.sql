delete from accpriv where priv='SINGLE_LOGIN';
delete from rupriv  where priv='SINGLE_LOGIN';
insert into rupriv (priv, valdefault) VALUES('@SINGLE_LOGIN', 0);
