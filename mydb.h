#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

struct DBT {
	void  *data;
	size_t size;
};

struct NODE {
	size_t size;
	size_t leaf;
	size_t numberOfBlock;
	struct DBT ** keys;
	struct DBT ** data;
	size_t * child;
};

struct DB {
	FILE * f;
	void * env;
	size_t blockSize;
	size_t t;
	size_t numberOfBlocks;
	size_t root;
		
	int (*insert)(struct DB *db, struct DBT *key, struct DBT *data);
	int (*nodeInsert)(struct DB *, struct NODE *, struct DBT *, struct DBT *);
	int (*nodeSplit)(struct DB *, struct NODE *, struct NODE *, size_t);
	int (*select)(struct DB *db, struct DBT *key, struct DBT *data);
	int (*nodeSelect)(struct DB *, size_t, struct DBT *, struct DBT *);
	int (*del)(struct DB *db, struct DBT *key);
	int (*close)(struct DB *db);
	int (*nodeReadDisk)(struct DB *, struct NODE *);
	int (*nodeWriteDisk)(struct DB *, struct NODE *);
	int (*allocator)(struct DB *, struct NODE *);
	int (*deAllocator)(struct DB *, struct NODE *);
	int (*cmp)(struct DBT *, struct DBT *);
	int (*nodeDel)(struct DB *, size_t, struct DBT *);
	int (*check)(struct DB *, struct NODE *, struct NODE *, int);
	int (*merge)(struct DB *, struct NODE *, struct NODE *, struct NODE *, int);
	
	int (*debug)(struct DB *, size_t);
};

struct DBC {
    size_t db_size;    /* 256*1024*1024 in tests */
    size_t page_size;  /* 512 */
    size_t cache_size; /* 256*1024 */
};

struct DB *dbcreate(char *, struct DBC *);
int db_insert(struct DB*, void *, size_t, void *, size_t);
int db_select(struct DB*, void *, size_t, void **, size_t *);
int db_delete(struct DB*, void *, size_t);
int db_close (struct DB*);
