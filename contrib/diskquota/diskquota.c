/* -------------------------------------------------------------------------
 *
 * auth_delay.c
 *
 * Copyright (c) 2010-2018, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *		contrib/auth_delay/auth_delay.c
 *
 * -------------------------------------------------------------------------
 */
#include "postgres.h"

#include <limits.h>

#include "catalog/pg_class.h"
#include "catalog/pg_namespace.h"
#include "catalog/objectaddress.h"
#include "catalog/objectaccess.h"
#include "commands/schemacmds.h"
#include "commands/tablecmds.h"
#include "port.h"
#include "storage/smgr.h"
#include "utils/elog.h"
#include "utils/guc.h"
#include "utils/lsyscache.h"
#include "utils/rel.h"
#include "utils/timestamp.h"

PG_MODULE_MAGIC;

void		_PG_init(void);
void		_PG_fini(void);

static object_access_hook_type prev_object_access_hook;
static PostDefineRelation_hook_type prev_define_relation;
static PreDropRelations_hook_type prev_drop_relations;
static PreTruncate_hook_type prev_truncate;
static PostAlterTableNS_hook_type prev_alter_schema;
static PostAlterTableOwner_hook_type prev_alter_owner;
static PostCreateSchema_hook_type prev_create_schema;

static void _object_access_hook(ObjectAccessType access, Oid classId, Oid objectId, int subId, void *arg);
static void define_relation(CreateStmt *stmt, Oid ownerId, Oid namespaceId, Oid relid);
static void drop_relations(ObjectAddress *targetObjects, int numrefs);
static void _truncate(Relation relation);
static void alter_schema(AlterObjectSchemaStmt *stmt, Oid relid, Oid oldns, Oid newns);
static void alter_owner(Form_pg_class pg_class, Oid relid, Oid oldOwnerId, Oid newOwnerId);


static void
report_active_table_helper(RelFileNodeBackend *rel)
{
}
static void
report_smgr(SMgrRelation sreln)
{
	RelFileNode *node = &sreln->smgr_rnode.node;
	elog(LOG, "smgr active relation: (%d, %d, %d) %d", node->dbNode, node->spcNode, node->relNode, table_oid);
	report_active_table_helper(&sreln->smgr_rnode);
}
static void
_object_access_hook(ObjectAccessType access, Oid classId, Oid objectId, int subId, void *arg)
{
	elog(NOTICE, "access=%d, classId=%u, oid=%u, subId=%d, arg=%p", (int)access, classId, objectId, subId, arg);
}
static void
define_relation(CreateStmt *stmt, Oid ownerId, Oid namespaceId, Oid relid)
{
	elog(NOTICE, "define relation oid=%u, namespaceId=%u, ownerId=%u", relid, namespaceId, ownerId);
}
static void
drop_relations(ObjectAddress *targetObjects, int numrefs)
{
	int i;
	for (i = 0; i < numrefs; i++)
	{
		ObjectAddress *thisobj = targetObjects + i;
		switch (thisobj->classId)
		{
			case RelationRelationId:
				elog(NOTICE, "drop table %u", thisobj->objectId);
				break;
			case NamespaceRelationId:
				elog(NOTICE, "drop schema %u", thisobj->objectId);
				break;
			default:
				elog(NOTICE, "other drop classId= %u, oid= %u", thisobj->classId, thisobj->objectId);
				break;
		}
	}
}
static void
_truncate(Relation relation)
{
	elog(NOTICE, "truncate relid= %u", relation->rd_id);
}
static void 
alter_schema(AlterObjectSchemaStmt *stmt, Oid relid, Oid oldns, Oid newns)
{
	const char *oldschema;
	oldschema = get_namespace_name(oldns);
	elog(NOTICE, "alter schema: '%s' to '%s' relid= %u, %u => %u", 
			oldschema, stmt->newschema, relid, oldns, newns);
}
static void
alter_owner(Form_pg_class pg_class, Oid relid, Oid oldOwnerId, Oid newOwnerId)
{
	elog(NOTICE, "alter owner: relid = %u, %u => %u", relid, oldOwnerId, newOwnerId);
}
static void 
create_schema(CreateSchemaStmt *stmt, const char *schemaname, Oid schema_oid, Oid owner_uid)
{
	elog(NOTICE, "create schema '%s' id = %u owner= %u", schemaname, schema_oid, owner_uid);
}

/*
 * Module Load Callback
 */
void
_PG_init(void)
{
	/* Install Hooks */
	prev_object_access_hook = object_access_hook;
	object_access_hook = _object_access_hook;

	prev_define_relation = PostDefineRelation_hook;
	PostDefineRelation_hook = define_relation;

	prev_drop_relations = PreDropRelations_hook;
	PreDropRelations_hook = drop_relations;

	prev_truncate = PreTruncate_hook;
	PreTruncate_hook = _truncate;

	prev_alter_schema = PostAlterTableNS_hook;
	PostAlterTableNS_hook = alter_schema;

	prev_alter_owner = PostAlterTableOwner_hook;
	PostAlterTableOwner_hook = alter_owner;

	prev_create_schema = PostCreateSchema_hook;
	PostCreateSchema_hook = create_schema;

	dqs_report_hook = report_smgr;

	elog(NOTICE, "disk_quota_init: hook version");
}
void
_PG_fini(void)
{
	object_access_hook = prev_object_access_hook;
	prev_object_access_hook = NULL;
	PostDefineRelation_hook = prev_define_relation;
	prev_define_relation = NULL;

	PreDropRelations_hook = prev_drop_relations;
	prev_drop_relations = NULL;

	PreTruncate_hook = prev_truncate;
	prev_truncate = NULL;

	PostAlterTableNS_hook = prev_alter_schema;
	prev_alter_schema = NULL;

	PostAlterTableOwner_hook = prev_alter_owner;
	prev_alter_owner = NULL;

	PostCreateSchema_hook = prev_create_schema;
	prev_create_schema = NULL;

	elog(NOTICE, "disk_quota_fini: hook version");
}
