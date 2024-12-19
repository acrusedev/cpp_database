# SQL Database with custom parser for DQL (Data Query Language), DDL (Data Definition Language) and DML (Data Manipulation Language).

## Principles
This database engine focuses on simplicity, yet providing atomicity and implementing custom sql parser
- Data is stored in a _data.db_ file in the root folder.
- Atomicity is ensured by logging user actions into a db.log file before executing command.
- 1. If, for any reason, __QUERY__ command failes to execute, all operations will be repeated upon next program run,
