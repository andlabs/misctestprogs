-- Pietro Gagliardi 2 November 2016
m4_changequote(<!,!>)

m4_ifdef(<!MySQL!>,<!
m4_errprint(<!MySQL sucks!>)
m4_m4exit(1)
!>)

m4_ifdef(<!SQLite)!>,<!

m4_dnl TODO add CHECK()s
m4_define(byte,INTEGER NOT NULL DEFAULT 0)
m4_define(int16,INTEGER NOT NULL DEFAULT 0)
m4_define(int32,INTEGER NOT NULL DEFAULT 0)
m4_define(int64,INTEGER NOT NULL DEFAULT 0)

m4_define(float64,REAL NOT NULL DEFAULT 0)

m4_define(string,TEXT NOT NULL DEFAULT '' COLLATE BINARY)
m4_define(cistring,TEXT NOT NULL DEFAULT '' COLLATE NOCASE)
m4_define(bytes,BLOB NOT NULL DEFAULT X'')

m4_define(bool,INTEGER NOT NULL DEFAULT 0)
m4_define(false,0)
m4_define(true,1)

m4_dnl TODO AUTOINCREMENT?
m4_define(primarykey,INTEGER NOT NULL PRIMARY KEY)

PRAGMA encoding = "UTF-8";
m4_define(createDatabase(name),<!
!>)

m4_define(currentTimestamp,strftime('%s', 'now'))

m4_define(defaultCurrentTimestamp,<!DEFAULT (currentTimestamp)!>)

m4_define(commonTriggers,<!
CREATE TRIGGER <!__!>$1<!_commonTriggerUpdateModifiedTime!>
	AFTER UPDATE ON $1
	FOR EACH ROW BEGIN
		UPDATE $1 SET ModifiedAt = currentTimestamp;
	END
!>)

!>)

m4_ifdef(<!PostgreSQL!>,<!
m4_errprint(Not implemented yet)
m4_m4exit(1)
!>)

m4_dnl TODO #error A database must be specified; type make help

m4_dnl use Unix time, in seconds
m4_define(time,int64)
m4_dnl and additional nsec if necessary
m4_define(nsec,int64)

m4_define(commonColumns,<!
	ID primarykey,
	CreatedAt time defaultCurrentTimestamp,
	ModifiedAt time defaultCurrentTimestamp,
	DeletedAt int64 DEFAULT NULL,
!>)
