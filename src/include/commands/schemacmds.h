/*-------------------------------------------------------------------------
 *
 * schemacmds.h
 *	  prototypes for schemacmds.c.
 *
 *
 * Portions Copyright (c) 1996-2018, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/commands/schemacmds.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef SCHEMACMDS_H
#define SCHEMACMDS_H

#include "catalog/objectaddress.h"
#include "nodes/parsenodes.h"

typedef void (*PostCreateSchema_hook_type)(CreateSchemaStmt *stmt, const char *schemaname,
		Oid schema_oid, Oid owner_uid);
extern PostCreateSchema_hook_type PostCreateSchema_hook;

extern Oid CreateSchemaCommand(CreateSchemaStmt *parsetree,
					const char *queryString,
					int stmt_location, int stmt_len);

extern void RemoveSchemaById(Oid schemaOid);

extern ObjectAddress RenameSchema(const char *oldname, const char *newname);
extern ObjectAddress AlterSchemaOwner(const char *name, Oid newOwnerId);
extern void AlterSchemaOwner_oid(Oid schemaOid, Oid newOwnerId);

#endif							/* SCHEMACMDS_H */
