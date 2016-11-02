-- Pietro Gagliardi 2 November 2016

#if defined(MySQL)
#error MySQL sucks
#elif defined(SQLite)

-- TODO add CHECK()s
#define byte INTEGER NOT NULL DEFAULT 0
#define int16 INTEGER NOT NULL DEFAULT 0
#define int32 INTEGER NOT NULL DEFAULT 0
#define int64 INTEGER NOT NULL DEFAULT 0

#define float64 REAL NOT NULL DEFAULT 0

#define string TEXT NOT NULL DEFAULT '' COLLATE BINARY
#define cistring TEXT NOT NULL DEFAULT '' COLLATE NOCASE
#define bytes BLOB NOT NULL DEFAULT X''

#define bool INTEGER NOT NULL DEFAULT 0
#define false 0
#define true 1

-- TODO AUTOINCREMENT?
#define primarykey INTEGER NOT NULL PRIMARY KEY

PRAGMA encoding = "UTF-8";
#define createDatabase(name) TODO

#define currentTimestamp strftime('%s', 'now')

#define defaultCurrentTimestamp DEFAULT (currentTimestamp)

#define commonTriggers(table) \
	CREATE TRIGGER __##table##_commonTriggerUpdateModifiedTime \
		AFTER UPDATE ON table \
		FOR EACH ROW BEGIN \
			UPDATE table SET ModifiedAt = currentTimestamp; \
		END

#elif defined(PostgreSQL)
#error Not implemented yet
#else
#error A database must be specified; type make help
#endif

-- use Unix time, in seconds
#define time int64
-- and additional nsec if necessary
#define nsec int32

#define commonColumns() \
	ID primarykey, \
	CreatedAt time defaultCurrentTimestamp, \
	ModifiedAt time defaultCurrentTimestamp, \
	DeletedAt int64 DEFAULT NULL,
