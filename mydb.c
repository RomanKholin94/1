#include "mydb.h"

#include <assert.h>
#include <string.h>

int db_insert(struct DB *db, void *key, size_t key_len,
	   void *val, size_t val_len) {
	struct DBT keyt = {
		.data = key,
		.size = key_len
	};
	struct DBT valt = {
		.data = val,
		.size = val_len
	};
	return db->insert(db, &keyt, &valt);
}

int db_select(struct DB *db, void *key, size_t key_len,
	   void **val, size_t *val_len) {
	struct DBT keyt = {
		.data = key,
		.size = key_len
	};
	struct DBT valt = {0, 0};
	int rc = db->select(db, &keyt, &valt);
	*val = valt.data;
	*val_len = valt.size;
	return rc;
}

int db_delete(struct DB *db, void *key, size_t key_len) {
	struct DBT keyt = {
		.data = key,
		.size = key_len
	};
	return db->del(db, &keyt);
}

int db_close(struct DB *db) {
	return db->close(db);
}

int myInsert(struct DB *db, struct DBT *key, struct DBT *data) {
	int i = 0;
	struct NODE *root = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, root);
	root->numberOfBlock = db->root;
	db->nodeReadDisk(db, root);
	for (i = 0; i < root->size; ++i) {
		if (db->cmp(key, root->keys[i]) == 0) {
			//printf("ok1\n");
			root->data[i]->size = data->size;
			free(root->data[i]->data);
			root->data[i]->data = (char *)calloc(data->size + 1, sizeof(char));
			memcpy(root->data[i]->data, data->data, data->size + 1);
			db->nodeWriteDisk(db, root);
			db->deAllocator(db, root);
			return 0;
		}
	}
	if (root->size == 2 * db->t - 1) {
		struct NODE *newRoot = (struct NODE *)calloc(1, sizeof(struct NODE));
		db->allocator(db, newRoot);
		newRoot->size = 0;
		newRoot->leaf = 0;
		newRoot->child[0] = root->numberOfBlock;
		db->numberOfBlocks += 1;
		newRoot->numberOfBlock = db->numberOfBlocks - 1;
		db->root = newRoot->numberOfBlock; 
		db->nodeSplit(db, newRoot, root, 0);
		newRoot->leaf = 0;
		if (db->cmp(key, newRoot->keys[0]) < 0) {
			newRoot->numberOfBlock = newRoot->child[0];
			db->nodeReadDisk(db, newRoot);
			db->nodeInsert(db, newRoot, key, data);
		} else if (db->cmp(key, newRoot->keys[0]) > 0) {
		newRoot->numberOfBlock = newRoot->child[1];
			db->nodeReadDisk(db, newRoot);
			db->nodeInsert(db, newRoot, key, data);
		} else {
			
		}
		db->deAllocator(db, newRoot);
	} else {
		db->nodeInsert(db, root, key, data);
	}
	db->deAllocator(db, root);
	//printf("numberOfBlocks = %zu\n", db->numberOfBlocks);
} 

int myNodeInsert(struct DB *db, struct NODE *node, struct DBT *key, struct DBT *data) {
	int i, j;
	i = node->size - 1;
	//printf("ok\n");
	if (node->leaf == 1) {
		while(i >= 0 && db->cmp(key, node->keys[i]) < 0) {
			node->keys[i + 1]->data = node->keys[i]->data;
			node->keys[i + 1]->size = node->keys[i]->size;
			node->data[i + 1]->data = node->data[i]->data;
			node->data[i + 1]->size = node->data[i]->size;
			--i;
		}
		++i;
		node->keys[i]->size = key->size;
		node->keys[i]->data = (char *)calloc(key->size + 1, sizeof(char));
		memcpy(node->keys[i]->data, key->data, key->size + 1);
		node->data[i]->size = data->size;
		node->data[i]->data = (char *)calloc(data->size + 1, sizeof(char));
		memcpy(node->data[i]->data, data->data, data->size + 1);
		//printf("%s\n", node->keys[i]->data);
		//printf("%d %s - NULL\n", node->keys[0]->size, node->keys[0]->data);
		//printf("%d %s - NULL\n", node->keys[i]->size, node->keys[i]->data);
		//printf("%d %s\n", i, key->data);
		node->size += 1;
		db->nodeWriteDisk(db, node);
	} else {
		while(i >= 0 && db->cmp(key, node->keys[i]) < 0) {
			--i;
		}
		++i;
		struct NODE *child = (struct NODE *)calloc(1, sizeof(struct NODE));
		db->allocator(db, child);
		child->numberOfBlock = node->child[i];
		db->nodeReadDisk(db, child);
		for (j = 0; j < child->size; ++j) {
			if (db->cmp(key, child->keys[j]) == 0) {
				//printf("ok2\n");
				child->data[j]->size = data->size;
				free(child->data[j]->data);
				child->data[j]->data = (char *)calloc(data->size + 1, sizeof(char));
				memcpy(child->data[j]->data, data->data, data->size + 1);
				db->nodeWriteDisk(db, child);
				db->deAllocator(db, child);
				return 0;
			}
		}
		if (child->size == 2 * db->t - 1) {
			db->nodeSplit(db, node, child, i);
			if (db->cmp(key, node->keys[i]) > 0) {
				++i;
			}
		}
		db->deAllocator(db, child);
		child = (struct NODE *)calloc(1, sizeof(struct NODE));
		db->allocator(db, child);
		child->numberOfBlock = node->child[i];
		db->nodeReadDisk(db, child);
		
		db->nodeInsert(db, child, key, data);
		
		db->deAllocator(db, child);
	}
}

int myNodeSplit(struct DB *db, struct NODE *node, struct NODE *child, size_t i) {
	int t = db->t, j;
	struct NODE *child2 = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, child2);
	node->size += 1;
	for (j = node->size - 1; i < j; --j) {
		node->keys[j]->data = node->keys[j - 1]->data;
		node->keys[j]->size = node->keys[j - 1]->size;
		node->data[j]->data = node->data[j - 1]->data;
		node->data[j]->size = node->data[j - 1]->size;
		node->child[j + 1] = node->child[j];
	}
	db->numberOfBlocks += 1;
	node->child[i + 1] = db->numberOfBlocks - 1;
	child2->numberOfBlock = db->numberOfBlocks - 1;
	node->keys[i]->data = child->keys[t - 1]->data;
	node->keys[i]->size = child->keys[t - 1]->size;
	node->data[i]->data = child->data[t - 1]->data;
	node->data[i]->size = child->data[t - 1]->size;
	child2->size = t - 1;
	for (j = t; j < 2 * t - 1; ++j) {
		child2->keys[j - t]->data = child->keys[j]->data;
		child->keys[j]->data = 0;
		child2->keys[j - t]->size = child->keys[j]->size;
		
		child2->data[j - t]->data = child->data[j]->data;
		child->data[j]->data = 0;
		child2->data[j - t]->size = child->data[j]->size;
		
		child2->child[j - t] = child->child[j];
	}
	child2->child[t - 1] = child->child[2 * t - 1];
	child->size = t - 1;
	child2->leaf = child->leaf;
	//printf("%s %s %s\n", node->keys[0]->data, child->keys[0]->data, child2->keys[0]->data);
	db->nodeWriteDisk(db, node);
	db->nodeWriteDisk(db, child);
	db->nodeWriteDisk(db, child2);
	db->deAllocator(db, child2);
}

int mySelect(struct DB *db, struct DBT *key, struct DBT *data) {
	db->nodeSelect(db, db->root, key, data);
	//printf("ans = %d\n", strlen(data->data));
}

int myNodeSelect(struct DB *db, size_t pointer, struct DBT *key, struct DBT *data) {
	int i = 0;
	struct NODE *node = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, node);
	node->numberOfBlock = pointer;
	db->nodeReadDisk(db, node);
	//printf("select = %s\n", node->keys[0]->data);
	while(i < node->size && db->cmp(key, node->keys[i]) > 0) {
		++i;
	}
	if (i < node->size && db->cmp(key, node->keys[i]) == 0) {
		//printf("ok\n");
		char *ans = (char *) calloc(node->data[i]->size + 1, sizeof(char));
		memcpy(ans, node->data[i]->data, node->data[i]->size + 1);
		data->data = ans;
		data->size = node->data[i]->size;
	} else if (node->leaf == 0) {
		//printf("%d\n", i);
		if (i < 0) {
			printf("aaaaaa\n");
			return 0;
		}
		db->nodeSelect(db, node->child[i], key, data);
	}
	db->deAllocator(db, node);
}

int myDel(struct DB *db, struct DBT *key) {
	//printf("ok\n");
	//db->Debug(db, db->root);
	db->nodeDel(db, db->root, key);
	struct NODE *root = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, root);
	root->numberOfBlock = db->root;
	db->nodeReadDisk(db, root);
	if (root->size == 0) {
		db->root = root->child[0];
	}
	return 0;
}

int myNodeDel(struct DB *db, size_t numberOfblock, struct DBT *key) {
	//printf("%d\n", numberOfblock);
	int i, j, k;
	struct NODE *root = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, root);
	root->numberOfBlock = numberOfblock;
	db->nodeReadDisk(db, root);
	if (root->size == 0) {
		db->deAllocator(db, root);
		return 0;
	}
	//printf("%s\n", root->keys[0]->data);
	i = 0;
	while (i < root->size && db->cmp(key, root->keys[i]) > 0) {
		++i;
	}
	/*if (i < root->size) {
		printf("%s %d\n", root->keys[i]->data, db->cmp(key, root->keys[i]));
	}*/
	//printf("%d %d %d\n", i, root->size, root->leaf);
	if ((i == root->size || db->cmp(key, root->keys[i]) != 0) && root->leaf == 1) {
		db->deAllocator(db, root);
		return 0;
	} else if (i < root->size && db->cmp(key, root->keys[i]) == 0) {
		//printf("del key\n");
		if (root->leaf == 1) {
			--root->size;
			//printf("ok leaf\n");
			for (j = i; j < root->size; ++j) {
				root->keys[j]->data = root->keys[j + 1]->data;
				root->keys[j]->size = root->keys[j + 1]->size;
				root->data[j]->data = root->data[j + 1]->data;
				root->data[j]->size = root->data[j + 1]->size;
			}
			db->nodeWriteDisk(db, root);
		} else {
			struct NODE *left = (struct NODE *)calloc(1, sizeof(struct NODE));
			db->allocator(db, left);
			left->numberOfBlock = root->child[i];
			db->nodeReadDisk(db, left);
			struct NODE *right = (struct NODE *)calloc(1, sizeof(struct NODE));
			db->allocator(db, right);
			right->numberOfBlock = root->child[i + 1];
			db->nodeReadDisk(db, right);
			//printf("ok!!! %d %d %s\n", left->size, right->size, root->keys[i]->data);
			if (left->size >= db->t) {
				//printf("ok!!! %d %d\n", left->size, right->size);
				struct NODE *newRoot;
				while (left->leaf == 0) {
					newRoot = left;
					left = (struct NODE *)calloc(1, sizeof(struct NODE));
					db->allocator(db, left);
					left->numberOfBlock = newRoot->child[newRoot->size];
					db->nodeReadDisk(db, left);
					//printf("%s, %s\n", newRoot->keys[0]->data, left->keys[0]->data);
					db->check(db, newRoot, left, newRoot->size);
					//printf("%d, %d\n", newRoot->size, left->size);
				}
				//printf("ok!!! %d %d\n", left->leaf, left->size);
				root->keys[i]->data = left->keys[left->size - 1]->data;
				root->keys[i]->size = left->keys[left->size - 1]->size;
				root->data[i]->data = left->data[left->size - 1]->data;
				root->data[i]->size = left->data[left->size - 1]->size;
				//printf("ok!!!\n");
				
				left->size -= 1;
				
				db->nodeWriteDisk(db, root);
				db->nodeWriteDisk(db, left);
			} else if (right->size >= db->t) {
				struct NODE *newRoot;
				while (right->leaf == 0) {
					newRoot = right;
					right = (struct NODE *)calloc(1, sizeof(struct NODE));
					db->allocator(db, right);
					right->numberOfBlock = newRoot->child[0];
					db->nodeReadDisk(db, right);
					/*for (j = 0; j <= newRoot->size; ++j) {
						printf("%zu ", newRoot->child[j]);
					}
					printf("\n");*/
					//printf("%s, %d, %s, %d\n", newRoot->keys[0]->data, newRoot->size, right->keys[0]->data, right->size);
					db->check(db, newRoot, right, 0);
					//printf("%d, %d\n", newRoot->size, right->size);
				}
				root->keys[i]->data = right->keys[0]->data;
				root->keys[i]->size = right->keys[0]->size;
				root->data[i]->data = right->data[0]->data;
				root->data[i]->size = right->data[0]->size;
				
				right->size -= 1;
				for (j = 0; j < right->size; ++j) {
					right->keys[j]->data = right->keys[j + 1]->data;
					right->keys[j]->size = right->keys[j + 1]->size;
					right->data[j]->data = right->data[j + 1]->data;
					right->data[j]->size = right->data[j + 1]->size;
					right->child[j] = right->child[j + 1];
				}
				right->child[j] = right->child[j + 1];
				
				db->nodeWriteDisk(db, root);
				db->nodeWriteDisk(db, right);
			} else {
				//printf("ok\n");
				db->merge(db, root, left, right, i);
				db->nodeDel(db, root->child[i], key);
			}
			db->deAllocator(db, left);
			db->deAllocator(db, right);
		}
	} else {
		struct NODE *node = (struct NODE *)calloc(1, sizeof(struct NODE));
		db->allocator(db, node);
		node->numberOfBlock = root->child[i];
		db->nodeReadDisk(db, node);
		//printf("i = %d\n", i);
		db->check(db, root, node, i);
		//printf("i = %d\n", i);
		--i;
		if (i < 0 || (i < root->size && db->cmp(key, root->keys[i]) > 0)) {
			++i;
		}
		//printf("aaaa %d\n", i);
		db->nodeDel(db, root->child[i], key);
		db->deAllocator(db, node);
	}
	db->deAllocator(db, root);
}

int myCheck(struct DB *db, struct NODE *root, struct NODE *node, int i) {
	int j, k;
	k = i;
	struct NODE *left = (struct NODE *)calloc(1, sizeof(struct NODE));	
	struct NODE *right = (struct NODE *)calloc(1, sizeof(struct NODE));	
	//printf("t = %d, size = %d\n", db->t, node->size);
	if (node->size >= db->t) {
		//printf("OK111\n");
		return 0;
	}
	if (i > 0) {
		db->allocator(db, left);
		left->numberOfBlock = root->child[i - 1];
		db->nodeReadDisk(db, left);
		//printf("left = %s\n", left->keys[0]->data);
	}
	if (i < root->size) {
		db->allocator(db, right);
		right->numberOfBlock = root->child[i + 1];
		db->nodeReadDisk(db, right);
		//printf("right = %s\n", right->keys[0]->data);
	} 
	if (i > 0 && left->size >= db->t) {
		node->size += 1;
		
		for (j = node->size - 1; j >= 0; --j) {
			node->keys[j + 1]->data = node->keys[j]->data;
			node->keys[j + 1]->size = node->keys[j]->size;
			node->data[j + 1]->data = node->data[j]->data;
			node->data[j + 1]->size = node->data[j]->size;
			node->child[j + 2] = node->child[j + 1];
		}
		node->child[j + 2] = node->child[j + 1];
		
		node->keys[0]->data = root->keys[i - 1]->data;
		node->keys[0]->size = root->keys[i - 1]->size;
		node->data[0]->data = root->data[i - 1]->data;
		node->data[0]->size = root->data[i - 1]->size;
		node->child[0] = left->child[left->size];
		
		root->keys[i - 1]->data = left->keys[left->size - 1]->data;
		root->keys[i - 1]->size = left->keys[left->size - 1]->size;
		root->data[i - 1]->data = left->data[left->size - 1]->data;
		root->data[i - 1]->size = left->data[left->size - 1]->size;
	
		left->size -= 1;
		--i;
		db->nodeWriteDisk(db, left);
	} else if (i < root->size && right->size >= db->t) {
		node->size += 1;
		node->keys[node->size - 1]->data = root->keys[i]->data;
		node->keys[node->size - 1]->size = root->keys[i]->size;
		node->data[node->size - 1]->data = root->data[i]->data;
		node->data[node->size - 1]->size = root->data[i]->size;
		node->child[node->size] = right->child[0];
		
		root->keys[i]->data = right->keys[0]->data;
		root->keys[i]->size = right->keys[0]->size;
		root->data[i]->data = right->data[0]->data;
		root->data[i]->size = right->data[0]->size;
		
		right->size -= 1;
		for (j = 0; j < right->size; ++j) {
			right->keys[j]->data = right->keys[j + 1]->data;
			right->keys[j]->size = right->keys[j + 1]->size;
			right->data[j]->data = right->data[j + 1]->data;
			right->data[j]->size = right->data[j + 1]->size;
			right->child[j] = right->child[j + 1];
		}				
		right->child[j] = right->child[j + 1];
		db->nodeWriteDisk(db, right);
	} else if (i > 0) {
		//printf("check ok1 %d %d\n", left->size, node->size);
		db->merge(db, root, left, node, i - 1);
		//printf("check ok2 %d %d\n", left->size, node->size);
		node->size = left->size;
		node->leaf = left->leaf;
		node->numberOfBlock = left->numberOfBlock;
		node->keys = left->keys;
		node->data = left->data;
		node->child = left->child;
		//printf("check ok2 %d %d\n", left->size, node->size);
		--i;
	} else if (i < root->size) {
		db->merge(db, root, node, right, i);
	} else {
		printf("truble\n");
	}
			
	db->nodeWriteDisk(db, root);
	db->nodeWriteDisk(db, node);
	//printf("ok\n");
	//db->deAllocator(db, left);
	//db->deAllocator(db, right);
}

int myMerge(struct DB *db, struct NODE *root, struct NODE *left, struct NODE *right, int i) {
	int j, t;
	//printf("%d %d %d, i = %d\n", left->size, root->size, right->size, i);
	t = db->t;
	left->keys[t - 1]->data = root->keys[i]->data;
	left->keys[t - 1]->size = root->keys[i]->size;
	left->data[t - 1]->data = root->data[i]->data;
	left->data[t - 1]->size = root->data[i]->size;
	root->size -= 1;
	//printf("ok\n");
	for (j = i; j < root->size; ++j) {
		root->keys[j]->data = root->keys[j + 1]->data;
		root->keys[j]->size = root->keys[j + 1]->size;
		root->data[j]->data = root->data[j + 1]->data;
		root->data[j]->size = root->data[j + 1]->size;
		root->child[j + 1] = root->child[j + 2];
	}
	left->size = 2 * t - 1;
	//printf("ok\n");
	for (j = 0; j < right->size; ++j) {
		left->keys[t + j]->data = right->keys[j]->data;
		left->keys[t + j]->size = right->keys[j]->size;
		left->data[t + j]->data = right->data[j]->data;
		left->data[t + j]->size = right->data[j]->size;
		left->child[t + j] = right->child[j]; 
	}
	left->child[left->size] = right->child[j];
	right->size = 0;
	//printf("ok\n");
	db->nodeWriteDisk(db, root);
	db->nodeWriteDisk(db, left);
	db->nodeWriteDisk(db, right);
}

int myDebug(struct DB *db, size_t numberOfBlock, FILE * f) {
	int i = 0;
	struct NODE *node = (struct NODE *)calloc(1, sizeof(struct NODE));
	node->numberOfBlock = numberOfBlock;
	db->allocator(db, node);
	db->nodeReadDisk(db, node);
	//fprintf(f, "%d\n", node->size);
	for (i = 0; i < node->size; ++i) {
		if (node->leaf == 0) {
		//	db->debug(db, node->child[i], f);
		}
		fprintf(f, "%s %s\n", node->keys[i]->data, node->data[i]->data);
	}
	if (node->leaf == 0) {
	//	db->debug(db, node->child[i], f);
	}
	fprintf(f, "\n");
	for (i = 0; i <= node->size; ++i) {
		if (node->leaf == 0) {
			db->debug(db, node->child[i], f);
		}
	}
} 

int myClose(struct DB *db) {
	FILE *f;
	f = fopen("debug.txt", "w");
	//printf("ok\n");
	//db->debug(db, db->root, f);
	fclose(f);
}

int myNodeReadDisk(struct DB *db, struct NODE *node) {
	int i;
	fseek(db->f, db->blockSize * node->numberOfBlock, SEEK_SET);
	fscanf(db->f, "%zu", &(node->size));
	fscanf(db->f, "%zu", &(node->leaf));
	for (i = 0; i < node->size; ++i) {
		fscanf(db->f, "%zu", &(node->keys[i]->size));
		//printf("%zu\n", node->keys[i]->size);
		node->keys[i]->data = (char *) calloc(node->keys[i]->size + 1,  sizeof(char));
		fread (node->keys[i]->data, 1, 1, db->f);
		fread (node->keys[i]->data, 1, node->keys[i]->size, db->f);
		//printf("!!! %s\n", node->keys[i]->data);
		
		fscanf(db->f, "%zu", &(node->data[i]->size));
		//printf("%zu", node->data[i]->size);
		node->data[i]->data = (char *) calloc(node->data[i]->size + 1,  sizeof(char));
		fread (node->data[i]->data, 1, 1, db->f);
		fread (node->data[i]->data, 1, node->data[i]->size, db->f);
		//printf("!!! %s\n", node->data[i]->data);
	}
	for(i = 0; i <= node->size; ++i) {
		fscanf(db->f, "%zu", &node->child[i]);
	}
}

int myNodeWriteDisk(struct DB *db, struct NODE *node) {
	int i;
	fseek(db->f, db->blockSize * (node->numberOfBlock), SEEK_SET);
	fprintf(db->f, "%zu\n", node->size);
	fprintf(db->f, "%zu\n", node->leaf);
	//printf("%zu\n", node->size);
	//printf("%zu\n", node->leaf);
	for (i = 0; i < node->size; ++i) {
		fprintf(db->f, "%zu\n", node->keys[i]->size);
		//printf("%zu\n", node->keys[i]->size);
		fwrite(node->keys[i]->data, sizeof(char), node->keys[i]->size, db->f);
		//printf("!!! %s\n", node->keys[i]->data);
		//write(node->keys[i]->data, sizeof(char), node->keys[i]->size);
		fprintf(db->f, "%zu\n", node->data[i]->size);
		//printf("%zu\n", node->data[i]->size);
		fwrite (node->data[i]->data, sizeof(char), node->data[i]->size, db->f);
	}
	for (i = 0; i <= node->size; ++i) {
		fprintf(db->f, "%zu\n", node->child[i]);
	}
}

int myAllocator(struct DB *db, struct NODE *node) {
	int i;
	node->keys = (struct DBT **) calloc((db->t * 2 - 1),  sizeof(struct DBT *));		
	node->data = (struct DBT **) calloc((db->t * 2 - 1),  sizeof(struct DBT *));
	node->child = (size_t *) calloc((db->t * 2),  sizeof(size_t));
	for (i = 0; i < db->t * 2 - 1; ++i) {	
		node->keys[i] = (struct DBT *) calloc(1,  sizeof(struct DBT));
		node->data[i] = (struct DBT *) calloc(1,  sizeof(struct DBT));
	}
}

int myDeAllocator(struct DB *db, struct NODE *node) {
	/*int i;
	//printf("%zu\n", node->size);
	//printf("i = %d %s ok\n", node->keys[0]->size, node->keys[0]->data);
	for (i = 0; i < db->t * 2 - 1; ++i) {
		//printf("i = %d %s ok\n", node->keys[i]->size, node->keys[i]->data);
		if (node->keys[i]->data != 0) {
			free(node->keys[i]->data);
		}
		free(node->keys[i]);
		if (node->data[i]->data != 0) {
			free(node->data[i]->data);
		}
		free(node->data[i]);
	}
	//printf("end i\n");
	free(node->keys);
	free(node->data);
	free(node->child);
	free(node);*/
}

int myCmp(struct DBT * a, struct DBT * b) {
	int ans, min;
	if (a->size < b->size) {
		min = a->size;
	} else {
		min = b->size;
	}
	ans = memcmp(a->data, b->data, min);
	if (ans != 0) {
		return ans;
	} else if (a->size < b->size) {
		return -1;
	} else if (a->size > b->size) {
		return 1;
	} else {
		return 0;
	}
}

struct DB *dbcreate(char *path, struct DBC *conf) {
	struct DB *db = (struct DB *)calloc(1, sizeof(struct DB));
	db->f = fopen("mytempfile.txt", "w+");
	db->blockSize = conf->page_size;
	db->t = 3;
	db->numberOfBlocks = 1;
	db->root = 0;
		
	db->insert = myInsert;
	db->nodeInsert = myNodeInsert;
	db->select = mySelect;
	db->nodeSelect = myNodeSelect;
	db->del = myDel;
	db->close = myClose;
	db->nodeSplit = myNodeSplit;
	db->nodeReadDisk = myNodeReadDisk;
	db->nodeWriteDisk = myNodeWriteDisk;
	db->allocator = myAllocator;
	db->deAllocator = myDeAllocator;
	db->cmp = myCmp;
	db->nodeDel = myNodeDel;
	db->check = myCheck;
	db->merge = myMerge;
	
	db->debug = myDebug;
	
	struct NODE *root = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, root);
	root->size = 0;
	root->leaf = 1;
	root->numberOfBlock = 0;
	db->nodeWriteDisk(db, root);
	db->deAllocator(db, root);
	
	return db;
}
