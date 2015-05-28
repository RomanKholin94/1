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

struct POINTER {
	struct NODE * a;
};

struct NL {
	struct NODE * page;
	struct NL * link;
	struct NL * next;
	struct NL * prev;
};

struct LIST {
	struct NL * first;
	struct NL * last;
};

struct HASH {
	size_t size;
	struct LIST ** data;
};

struct DB {
	FILE * f;
	void * env;
	size_t blockSize;
	size_t t;
	size_t numberOfBlocks;
	size_t root;
	size_t numberOfPages;
	size_t cashSize;
	struct LIST * L;
	struct HASH * H;
		
	int (*insert)(struct DB *db, struct DBT *key, struct DBT *data);
	int (*nodeInsert)(struct DB *, struct POINTER *, struct DBT *, struct DBT *);
	int (*nodeSplit)(struct DB *, struct POINTER *, struct POINTER *, size_t);
	int (*select)(struct DB *db, struct DBT *key, struct DBT *data);
	int (*nodeSelect)(struct DB *, size_t, struct DBT *, struct DBT *);
	int (*del)(struct DB *db, struct DBT *key);
	int (*close)(struct DB *db);
	int (*nodeReadDisk)(struct DB *, struct POINTER *,  size_t);
	int (*nodeWriteDisk)(struct DB *, struct NODE *);
	int (*allocator)(struct DB *, struct POINTER *);
	int (*deAllocator)(struct DB *, struct NODE *);
	int (*cmp)(struct DBT *, struct DBT *);
	int (*nodeDel)(struct DB *, size_t, struct DBT *);
	int (*check)(struct DB *, struct POINTER *, struct POINTER *, int);
	int (*merge)(struct DB *, struct POINTER *, struct POINTER *, struct POINTER *, int);
	int (*searchHash)(struct DB *, size_t, struct POINTER *);
	int (*removeHash)(struct DB *);
	int (*addHash)(struct DB *, struct POINTER *);
	int (*remove)(struct LIST *, struct NL *);
	int (*add)(struct LIST *, struct NL *, struct NL *);
	
	int (*debug)(struct DB *, size_t, FILE * f);
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
