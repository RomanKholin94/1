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
	struct POINTER *root =  (struct POINTER *)calloc(1, sizeof(struct POINTER));
	root->a = (struct NODE *)calloc(1, sizeof(struct NODE));
	//db->allocator(db, root);
	//root->numberOfBlock = db->root;
	db->nodeReadDisk(db, root, db->root);
	//printf("%d\n", root->size);
	for (i = 0; i < root->a->size; ++i) {
		if (db->cmp(key, root->a->keys[i]) == 0) {
			//printf("ok1\n");
			root->a->data[i]->size = data->size;
			free(root->a->data[i]->data);
			root->a->data[i]->data = (char *)calloc(data->size + 1, sizeof(char));
			memcpy(root->a->data[i]->data, data->data, data->size + 1);
			//db->nodeWriteDisk(db, root);
			//printf("1\n");
			//db->deAllocator(db, root);
			return 0;
		}
	}
	if (root->a->size == 2 * db->t - 1) {
		if (db->cashSize == db->numberOfPages) {
			db->removeHash(db);
		}
		struct POINTER *newRoot = (struct POINTER *)calloc(1, sizeof(struct POINTER));
		newRoot->a = (struct NODE *)calloc(1, sizeof(struct NODE));
		db->allocator(db, newRoot);
		newRoot->a->size = 0;
		newRoot->a->leaf = 0;
		newRoot->a->child[0] = root->a->numberOfBlock;
		db->numberOfBlocks += 1;
		newRoot->a->numberOfBlock = db->numberOfBlocks - 1;
		db->root = newRoot->a->numberOfBlock; 
		db->nodeSplit(db, newRoot, root, 0);
		db->addHash(db, newRoot);
		//printf("root = %s\n", newRoot->keys[0]->data);
		newRoot->a->leaf = 0;
		if (db->cmp(key, newRoot->a->keys[0]) < 0) {
			//newRoot->numberOfBlock = newRoot->child[0];
			db->nodeReadDisk(db, newRoot, newRoot->a->child[0]);
			db->nodeInsert(db, newRoot, key, data);
		} else if (db->cmp(key, newRoot->a->keys[0]) > 0) {
			//newRoot->numberOfBlock = newRoot->child[1];
			db->nodeReadDisk(db, newRoot, newRoot->a->child[1]);
			db->nodeInsert(db, newRoot, key, data);
		} else {
			
		}
		//printf("2\n");
		//db->deAllocator(db, newRoot);
	} else {
		//printf("insert\n");
		db->nodeInsert(db, root, key, data);
	}
	//printf("3\n");
	//db->deAllocator(db, root);
	//printf("numberOfBlocks = %zu\n", db->numberOfBlocks);
} 

int myNodeInsert(struct DB *db, struct POINTER *node, struct DBT *key, struct DBT *data) {
	int i, j;
	i = node->a->size - 1;
	//printf("ok\n");
	//printf("aaaa %d\n", node->a->numberOfBlock);
	if (node->a->leaf == 1) {
		while(i >= 0 && db->cmp(key, node->a->keys[i]) < 0) {
			node->a->keys[i + 1]->data = node->a->keys[i]->data;
			node->a->keys[i + 1]->size = node->a->keys[i]->size;
			node->a->data[i + 1]->data = node->a->data[i]->data;
			node->a->data[i + 1]->size = node->a->data[i]->size;
			--i;
		}
		++i;
		node->a->keys[i]->size = key->size;
		node->a->keys[i]->data = (char *)calloc(key->size + 1, sizeof(char));
		memcpy(node->a->keys[i]->data, key->data, key->size + 1);
		node->a->data[i]->size = data->size;
		node->a->data[i]->data = (char *)calloc(data->size + 1, sizeof(char));
		memcpy(node->a->data[i]->data, data->data, data->size + 1);
		//printf("%s\n", node->keys[i]->data);
		//printf("%d %s - NULL\n", node->keys[0]->size, node->keys[0]->data);
		//printf("%d %s - NULL\n", node->keys[i]->size, node->keys[i]->data);
		//printf("%d %s\n", i, key->data);
		node->a->size += 1;
		//printf("%d\n", node->a->numberOfBlock);
		//db->nodeWriteDisk(db, node);
	} else {
		while(i >= 0 && db->cmp(key, node->a->keys[i]) < 0) {
			--i;
		}
		++i;
		struct POINTER *child = (struct POINTER *)calloc(1, sizeof(struct POINTER));
		child->a = (struct NODE *)calloc(1, sizeof(struct NODE)); 
		//db->allocator(db, child);
		//child->numberOfBlock = node->child[i];
		db->nodeReadDisk(db, child, node->a->child[i]);
		for (j = 0; j < child->a->size; ++j) {
			if (db->cmp(key, child->a->keys[j]) == 0) {
				//printf("ok2\n");
				child->a->data[j]->size = data->size;
				free(child->a->data[j]->data);
				child->a->data[j]->data = (char *)calloc(data->size + 1, sizeof(char));
				memcpy(child->a->data[j]->data, data->data, data->size + 1);
				//db->nodeWriteDisk(db, child);
				//printf("4\n");
				//db->deAllocator(db, child);
				return 0;
			}
		}
		if (child->a->size == 2 * db->t - 1) {
			db->nodeSplit(db, node, child, i);
			if (db->cmp(key, node->a->keys[i]) > 0) {
				++i;
			}
		}
		//printf("5\n");
		//db->deAllocator(db, child);
		child = (struct POINTER *)calloc(1, sizeof(struct POINTER));
		child->a = (struct NODE *)calloc(1, sizeof(struct NODE));
		//db->allocator(db, child);
		//child->numberOfBlock = node->child[i];
		db->nodeReadDisk(db, child, node->a->child[i]);
		
		db->nodeInsert(db, child, key, data);
		
		//printf("6\n");
		//db->deAllocator(db, child);
	}
}

int myNodeSplit(struct DB *db, struct POINTER *node, struct POINTER *child, size_t i) {
	int t = db->t, j;
	if (db->cashSize == db->numberOfPages) {
		db->removeHash(db);
	}
	struct POINTER *child2 = (struct POINTER *)calloc(1, sizeof(struct POINTER));
	child2->a = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, child2);
	node->a->size += 1;
	for (j = node->a->size - 1; i < j; --j) {
		node->a->keys[j]->data = node->a->keys[j - 1]->data;
		node->a->keys[j]->size = node->a->keys[j - 1]->size;
		node->a->data[j]->data = node->a->data[j - 1]->data;
		node->a->data[j]->size = node->a->data[j - 1]->size;
		node->a->child[j + 1] = node->a->child[j];
	}
	db->numberOfBlocks += 1;
	node->a->child[i + 1] = db->numberOfBlocks - 1;
	child2->a->numberOfBlock = db->numberOfBlocks - 1;
	node->a->keys[i]->data = child->a->keys[t - 1]->data;
	node->a->keys[i]->size = child->a->keys[t - 1]->size;
	node->a->data[i]->data = child->a->data[t - 1]->data;
	node->a->data[i]->size = child->a->data[t - 1]->size;
	//printf("t = %d\n", t);
	child2->a->size = t - 1;
	for (j = t; j < 2 * t - 1; ++j) {
		child2->a->keys[j - t]->data = child->a->keys[j]->data;
		child->a->keys[j]->data = 0;
		child2->a->keys[j - t]->size = child->a->keys[j]->size;
		
		child2->a->data[j - t]->data = child->a->data[j]->data;
		child->a->data[j]->data = 0;
		child2->a->data[j - t]->size = child->a->data[j]->size;
		
		child2->a->child[j - t] = child->a->child[j];
	}
	child2->a->child[t - 1] = child->a->child[2 * t - 1];
	child->a->size = t - 1;
	//printf("t = %d\n", t);
	child2->a->leaf = child->a->leaf;
	//printf("%s %s %s\n", node->keys[0]->data, child->keys[0]->data, child2->keys[0]->data);
	//db->nodeWriteDisk(db, node);
	//db->nodeWriteDisk(db, child);
	//db->nodeWriteDisk(db, child2);
	db->addHash(db, child2);
	//printf("7\n");
	//db->deAllocator(db, child2);
}

int mySelect(struct DB *db, struct DBT *key, struct DBT *data) {
	db->nodeSelect(db, db->root, key, data);
	//printf("ans = %d\n", strlen(data->data));
}

int myNodeSelect(struct DB *db, size_t pointer, struct DBT *key, struct DBT *data) {
	int i = 0;
	struct POINTER *node = (struct POINTER *)calloc(1, sizeof(struct POINTER));
	node->a = (struct NODE *)calloc(1, sizeof(struct NODE));
	//db->allocator(db, node);
	//node->numberOfBlock = pointer;
	db->nodeReadDisk(db, node, pointer);
	//printf("%d %d\n", node->a->size, pointer);
	//printf("select = %s\n", node->keys[0]->data);
	while(i < node->a->size && db->cmp(key, node->a->keys[i]) > 0) {
		++i;
	}
	if (i < node->a->size && db->cmp(key, node->a->keys[i]) == 0) {
		//printf("ok\n");
		char *ans = (char *) calloc(node->a->data[i]->size + 1, sizeof(char));
		memcpy(ans, node->a->data[i]->data, node->a->data[i]->size + 1);
		data->data = ans;
		data->size = node->a->data[i]->size;
	} else if (node->a->leaf == 0) {
		//printf("%d\n", i);
		if (i < 0) {
			printf("aaaaaa\n");
			return 0;
		}
		db->nodeSelect(db, node->a->child[i], key, data);
	}
	//printf("8\n");
	//db->deAllocator(db, node);
}

int myDel(struct DB *db, struct DBT *key) {
	//printf("ok\n");
	//db->Debug(db, db->root);
	db->nodeDel(db, db->root, key);
	struct POINTER *root = (struct POINTER *)calloc(1, sizeof(struct POINTER));
	root->a = (struct NODE *)calloc(1, sizeof(struct NODE));
	//db->allocator(db, root);
	//root->numberOfBlock = db->root;
	db->nodeReadDisk(db, root, db->root);
	if (root->a->size == 0) {
		db->root = root->a->child[0];
	}
	return 0;
}

int myNodeDel(struct DB *db, size_t numberOfblock, struct DBT *key) {
	//printf("%d\n", numberOfblock);
	int i, j, k;
	struct POINTER *root = (struct POINTER *)calloc(1, sizeof(struct POINTER));
	root->a = (struct NODE *)calloc(1, sizeof(struct NODE));
	//db->allocator(db, root);
	//root->numberOfBlock = numberOfblock;
	db->nodeReadDisk(db, root, numberOfblock);
	if (root->a->size == 0) {
		//printf("9\n");
		//db->deAllocator(db, root);
		return 0;
	}
	//printf("%s\n", root->keys[0]->data);
	i = 0;
	while (i < root->a->size && db->cmp(key, root->a->keys[i]) > 0) {
		++i;
	}
	/*if (i < root->size) {
		printf("%s %d\n", root->keys[i]->data, db->cmp(key, root->keys[i]));
	}*/
	//printf("%d %d %d\n", i, root->size, root->leaf);
	if ((i == root->a->size || db->cmp(key, root->a->keys[i]) != 0) && root->a->leaf == 1) {
		//printf("10\n");
		//db->deAllocator(db, root);
		return 0;
	} else if (i < root->a->size && db->cmp(key, root->a->keys[i]) == 0) {
		//printf("del key\n");
		if (root->a->leaf == 1) {
			--root->a->size;
			//printf("ok leaf\n");
			for (j = i; j < root->a->size; ++j) {
				root->a->keys[j]->data = root->a->keys[j + 1]->data;
				root->a->keys[j]->size = root->a->keys[j + 1]->size;
				root->a->data[j]->data = root->a->data[j + 1]->data;
				root->a->data[j]->size = root->a->data[j + 1]->size;
			}
			//db->nodeWriteDisk(db, root);
		} else {
			struct POINTER *left = (struct POINTER *)calloc(1, sizeof(struct POINTER));
			left->a = (struct NODE *)calloc(1, sizeof(struct NODE));
			//db->allocator(db, left);
			//left->numberOfBlock = root->child[i];
			db->nodeReadDisk(db, left, root->a->child[i]);
			struct POINTER *right = (struct POINTER *)calloc(1, sizeof(struct POINTER));
			right->a = (struct NODE *)calloc(1, sizeof(struct NODE));
			//db->allocator(db, right);
			//right->numberOfBlock = root->child[i + 1];
			db->nodeReadDisk(db, right, root->a->child[i + 1]);
			//printf("ok!!! %d %d %s\n", left->size, right->size, root->keys[i]->data);
			if (left->a->size >= db->t) {
				//printf("ok!!! %d %d\n", left->size, right->size);
				struct POINTER *newRoot;
				while (left->a->leaf == 0) {
					newRoot = left;
					left = (struct POINTER *)calloc(1, sizeof(struct POINTER));
					left->a = (struct NODE *)calloc(1, sizeof(struct NODE));
					//db->allocator(db, left);
					//left->numberOfBlock = newRoot->child[newRoot->size];
					db->nodeReadDisk(db, left, newRoot->a->child[newRoot->a->size]);
					//printf("%s, %s\n", newRoot->keys[0]->data, left->keys[0]->data);
					db->check(db, newRoot, left, newRoot->a->size);
					//printf("%d, %d\n", newRoot->size, left->size);
				}
				//printf("ok!!! %d %d\n", left->leaf, left->size);
				root->a->keys[i]->data = left->a->keys[left->a->size - 1]->data;
				root->a->keys[i]->size = left->a->keys[left->a->size - 1]->size;
				root->a->data[i]->data = left->a->data[left->a->size - 1]->data;
				root->a->data[i]->size = left->a->data[left->a->size - 1]->size;
				//printf("ok!!!\n");
				
				left->a->size -= 1;
				
				//db->nodeWriteDisk(db, root);
				//db->nodeWriteDisk(db, left);
			} else if (right->a->size >= db->t) {
				struct POINTER *newRoot;
				while (right->a->leaf == 0) {
					newRoot = right;
					right = (struct POINTER *)calloc(1, sizeof(struct POINTER));
					right->a = (struct NODE *)calloc(1, sizeof(struct NODE));
					//db->allocator(db, right);
					//right->numberOfBlock = newRoot->child[0];
					db->nodeReadDisk(db, right, newRoot->a->child[0]);
					/*for (j = 0; j <= newRoot->size; ++j) {
						printf("%zu ", newRoot->child[j]);
					}
					printf("\n");*/
					//printf("%s, %d, %s, %d\n", newRoot->keys[0]->data, newRoot->size, right->keys[0]->data, right->size);
					db->check(db, newRoot, right, 0);
					//printf("%d, %d\n", newRoot->size, right->size);
				}
				root->a->keys[i]->data = right->a->keys[0]->data;
				root->a->keys[i]->size = right->a->keys[0]->size;
				root->a->data[i]->data = right->a->data[0]->data;
				root->a->data[i]->size = right->a->data[0]->size;
				
				right->a->size -= 1;
				for (j = 0; j < right->a->size; ++j) {
					right->a->keys[j]->data = right->a->keys[j + 1]->data;
					right->a->keys[j]->size = right->a->keys[j + 1]->size;
					right->a->data[j]->data = right->a->data[j + 1]->data;
					right->a->data[j]->size = right->a->data[j + 1]->size;
					right->a->child[j] = right->a->child[j + 1];
				}
				right->a->child[j] = right->a->child[j + 1];
				
				//db->nodeWriteDisk(db, root);
				//db->nodeWriteDisk(db, right);
			} else {
				//printf("ok\n");
				db->merge(db, root, left, right, i);
				db->nodeDel(db, root->a->child[i], key);
			}
			//printf("11\n");
			//db->deAllocator(db, left);
			//printf("12\n");
			//db->deAllocator(db, right);
		}
	} else {
		struct POINTER *node = (struct POINTER *)calloc(1, sizeof(struct POINTER));
		node->a = (struct NODE *)calloc(1, sizeof(struct NODE));
		//db->allocator(db, node);
		//node->numberOfBlock = root->child[i];
		db->nodeReadDisk(db, node, root->a->child[i]);
		//printf("i = %d\n", i);
		db->check(db, root, node, i);
		//printf("i = %d\n", i);
		--i;
		if (i < 0 || (i < root->a->size && db->cmp(key, root->a->keys[i]) > 0)) {
			++i;
		}
		//printf("aaaa %d\n", i);
		db->nodeDel(db, root->a->child[i], key);
		//printf("13\n");
		//db->deAllocator(db, node);
	}
	//printf("14\n");
	//db->deAllocator(db, root);
}

int myCheck(struct DB *db, struct POINTER *root, struct POINTER *node, int i) {
	int j, k;
	k = i;
	struct POINTER *left = (struct POINTER *)calloc(1, sizeof(struct POINTER));	
	left->a = (struct NODE *)calloc(1, sizeof(struct NODE));
	struct POINTER *right = (struct POINTER *)calloc(1, sizeof(struct POINTER));	
	right->a = (struct NODE *)calloc(1, sizeof(struct NODE));
	//printf("t = %d, size = %d\n", db->t, node->size);
	if (node->a->size >= db->t) {
		//printf("OK111\n");
		return 0;
	}
	if (i > 0) {
		//db->allocator(db, left);
		//left->numberOfBlock = root->child[i - 1];
		db->nodeReadDisk(db, left, root->a->child[i - 1]);
		//printf("left = %s\n", left->keys[0]->data);
	}
	if (i < root->a->size) {
		//db->allocator(db, right);
		//right->numberOfBlock = root->child[i + 1];
		db->nodeReadDisk(db, right, root->a->child[i + 1]);
		//printf("right = %s\n", right->keys[0]->data);
	} 
	if (i > 0 && left->a->size >= db->t) {
		node->a->size += 1;
		
		for (j = node->a->size - 1; j >= 0; --j) {
			node->a->keys[j + 1]->data = node->a->keys[j]->data;
			node->a->keys[j + 1]->size = node->a->keys[j]->size;
			node->a->data[j + 1]->data = node->a->data[j]->data;
			node->a->data[j + 1]->size = node->a->data[j]->size;
			node->a->child[j + 2] = node->a->child[j + 1];
		}
		node->a->child[j + 2] = node->a->child[j + 1];
		
		node->a->keys[0]->data = root->a->keys[i - 1]->data;
		node->a->keys[0]->size = root->a->keys[i - 1]->size;
		node->a->data[0]->data = root->a->data[i - 1]->data;
		node->a->data[0]->size = root->a->data[i - 1]->size;
		node->a->child[0] = left->a->child[left->a->size];
		
		root->a->keys[i - 1]->data = left->a->keys[left->a->size - 1]->data;
		root->a->keys[i - 1]->size = left->a->keys[left->a->size - 1]->size;
		root->a->data[i - 1]->data = left->a->data[left->a->size - 1]->data;
		root->a->data[i - 1]->size = left->a->data[left->a->size - 1]->size;
	
		left->a->size -= 1;
		--i;
		//db->nodeWriteDisk(db, left);
	} else if (i < root->a->size && right->a->size >= db->t) {
		node->a->size += 1;
		node->a->keys[node->a->size - 1]->data = root->a->keys[i]->data;
		node->a->keys[node->a->size - 1]->size = root->a->keys[i]->size;
		node->a->data[node->a->size - 1]->data = root->a->data[i]->data;
		node->a->data[node->a->size - 1]->size = root->a->data[i]->size;
		node->a->child[node->a->size] = right->a->child[0];
		
		root->a->keys[i]->data = right->a->keys[0]->data;
		root->a->keys[i]->size = right->a->keys[0]->size;
		root->a->data[i]->data = right->a->data[0]->data;
		root->a->data[i]->size = right->a->data[0]->size;
		
		right->a->size -= 1;
		for (j = 0; j < right->a->size; ++j) {
			right->a->keys[j]->data = right->a->keys[j + 1]->data;
			right->a->keys[j]->size = right->a->keys[j + 1]->size;
			right->a->data[j]->data = right->a->data[j + 1]->data;
			right->a->data[j]->size = right->a->data[j + 1]->size;
			right->a->child[j] = right->a->child[j + 1];
		}				
		right->a->child[j] = right->a->child[j + 1];
		//db->nodeWriteDisk(db, right);
	} else if (i > 0) {
		//printf("check ok1 %d %d\n", left->size, node->size);
		db->merge(db, root, left, node, i - 1);
		//printf("check ok2 %d %d\n", left->size, node->size);
		node->a->size = left->a->size;
		node->a->leaf = left->a->leaf;
		node->a->numberOfBlock = left->a->numberOfBlock;
		node->a->keys = left->a->keys;
		node->a->data = left->a->data;
		node->a->child = left->a->child;
		//printf("check ok2 %d %d\n", left->size, node->size);
		--i;
	} else if (i < root->a->size) {
		db->merge(db, root, node, right, i);
	} else {
		printf("truble\n");
	}
			
	//db->nodeWriteDisk(db, root);
	//db->nodeWriteDisk(db, node);
	//printf("ok\n");
	//db->deAllocator(db, left);
	//printf("15\n");
	//db->deAllocator(db, right);
}

int myMerge(struct DB *db, struct POINTER *root, struct POINTER *left, struct POINTER *right, int i) {
	int j, t;
	//printf("%d %d %d, i = %d\n", left->size, root->size, right->size, i);
	t = db->t;
	left->a->keys[t - 1]->data = root->a->keys[i]->data;
	left->a->keys[t - 1]->size = root->a->keys[i]->size;
	left->a->data[t - 1]->data = root->a->data[i]->data;
	left->a->data[t - 1]->size = root->a->data[i]->size;
	root->a->size -= 1;
	//printf("ok\n");
	for (j = i; j < root->a->size; ++j) {
		root->a->keys[j]->data = root->a->keys[j + 1]->data;
		root->a->keys[j]->size = root->a->keys[j + 1]->size;
		root->a->data[j]->data = root->a->data[j + 1]->data;
		root->a->data[j]->size = root->a->data[j + 1]->size;
		root->a->child[j + 1] = root->a->child[j + 2];
	}
	left->a->size = 2 * t - 1;
	//printf("ok\n");
	for (j = 0; j < right->a->size; ++j) {
		left->a->keys[t + j]->data = right->a->keys[j]->data;
		left->a->keys[t + j]->size = right->a->keys[j]->size;
		left->a->data[t + j]->data = right->a->data[j]->data;
		left->a->data[t + j]->size = right->a->data[j]->size;
		left->a->child[t + j] = right->a->child[j]; 
	}
	left->a->child[left->a->size] = right->a->child[j];
	right->a->size = 0;
	//printf("ok\n");
	//db->nodeWriteDisk(db, root);
	//db->nodeWriteDisk(db, left);
	//db->nodeWriteDisk(db, right);
}

int myDebug(struct DB *db, size_t numberOfBlock, FILE * f) {
	/*int i = 0;
	struct POINTER *node = (struct POINTER *)calloc(1, sizeof(struct POINTER));
	node->numberOfBlock = numberOfBlock;
	db->allocator(db, node);
	db->nodeReadDisk(db, node, numberOfBlock);
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
	}*/
} 

int myClose(struct DB *db) {
	FILE *f;
	f = fopen("debug.txt", "w");
	//printf("ok\n");
	//db->debug(db, db->root, f);
	fclose(f);
}

int myNodeReadDisk(struct DB *db, struct POINTER *node, size_t numberOfBlock) {
	if (db->searchHash(db, numberOfBlock, node) == 1) {
		return 0;
	}
	if (db->cashSize == db->numberOfPages) {
		db->removeHash(db);
	}
	//node = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, node);
	node->a->numberOfBlock = numberOfBlock;
	int i;
	fseek(db->f, db->blockSize * node->a->numberOfBlock, SEEK_SET);
	fscanf(db->f, "%zu", &(node->a->size));
	//printf("%zu\n", node->size);
	fscanf(db->f, "%zu", &(node->a->leaf));
	for (i = 0; i < node->a->size; ++i) {
		fscanf(db->f, "%zu", &(node->a->keys[i]->size));
		//printf("%zu\n", node->keys[i]->size);
		node->a->keys[i]->data = (char *) calloc(node->a->keys[i]->size + 1,  sizeof(char));
		fread (node->a->keys[i]->data, 1, 1, db->f);
		fread (node->a->keys[i]->data, 1, node->a->keys[i]->size, db->f);
		//printf("!!! %s\n", node->keys[i]->data);
		
		fscanf(db->f, "%zu", &(node->a->data[i]->size));
		//printf("%zu", node->data[i]->size);
		node->a->data[i]->data = (char *) calloc(node->a->data[i]->size + 1,  sizeof(char));
		fread (node->a->data[i]->data, 1, 1, db->f);
		fread (node->a->data[i]->data, 1, node->a->data[i]->size, db->f);
		//printf("!!! %s\n", node->data[i]->data);
	}
	for(i = 0; i <= node->a->size; ++i) {
		fscanf(db->f, "%zu", &node->a->child[i]);
	}
	db->addHash(db, node);
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

int mySearchHash(struct DB *db, size_t x, struct POINTER *node) {
	int i;
	struct NL * j;
	i = x % db->H->size;
	if (db->H->data[i] == 0) {
		return 0;
	}
	j = db->H->data[i]->first;
	//printf("x = %d, i =  %d, j = %d\n", x, i, db->H->data[i]->first);
	while(j != 0) {
		//printf("%d %d\n", j, j->page->numberOfBlock);
		if (j->page->numberOfBlock == x) {
			
			node->a = j->page;
			
			db->remove(db->L, j->link);
			db->add(db->L, db->L->last, j);
			return 1;
		}
		//printf("%d\n", );
		j = j->next;
	}
	
	return 0;
}

int myRemoveHash(struct DB *db) {
	int i;
	struct NL * j;
	struct NL * k;
	
	i = db->L->first->page->numberOfBlock % db->numberOfPages;
	j = db->L->first;
	k = j->link;
	db->remove(db->L, j);
	db->remove(db->H->data[i], k);
		
	db->nodeWriteDisk(db, j->page);
	db->deAllocator(db, j->page);
	db->cashSize -= 1;
}

int myAddHash(struct DB *db, struct POINTER *node) {
	 struct NL * nl;
	 struct NL * nh;
	 nl = (struct NL *) calloc(1,  sizeof(struct NL));
	 nh = (struct NL *) calloc(1,  sizeof(struct NL));
	 nl->link = nh;
	 nh->link = nl;
	 nh->page = node->a;
	 nl->page = node->a;
	 db->add(db->L, db->L->last, nl);
	 db->add(db->H->data[node->a->numberOfBlock % db->H->size], db->H->data[node->a->numberOfBlock % db->H->size]->last, nh);
	 db->cashSize += 1;
}

int myRemove(struct LIST * l, struct NL * n) {
	if (n->next == 0 && n->prev == 0) {
		l->last = 0;
		l->first = 0;
		return 0;
	}
	if (n->next != 0) {
		n->next->prev = n->prev;
	} else {
		l->last = n->prev;
	}
	
	if (n->prev != 0) {
		n->prev->next = n->next;
	} else {
		l->first = n->next;
	}
}

int myAdd(struct LIST * l, struct NL * a, struct NL * b) {
	if (a == 0) {
		l->first = b;
		l->last = b;
	} else {
		b->prev = a;
		b->next = a->next;
		a->next = b;
	}
	//printf("add = %d\n", l->last);
}

int myAllocator(struct DB *db, struct POINTER *node) {
	int i;
	//printf("Alloc %d\n", db->t);
	node->a->keys = (struct DBT **) calloc((db->t * 2 - 1),  sizeof(struct DBT *));		
	node->a->data = (struct DBT **) calloc((db->t * 2 - 1),  sizeof(struct DBT *));
	node->a->child = (size_t *) calloc((db->t * 2),  sizeof(size_t));
	for (i = 0; i < db->t * 2 - 1; ++i) {	
		node->a->keys[i] = (struct DBT *) calloc(1,  sizeof(struct DBT));
		node->a->data[i] = (struct DBT *) calloc(1,  sizeof(struct DBT));
	}
}

int myDeAllocator(struct DB *db, struct NODE *node) {
	/*int i;
	if (db == 0) {
		return 0;
	}
	//printf("node size = %zu\n", node->size);
	//printf("%d %s ok\n", node->keys[0]->size, node->keys[0]->data);
	for (i = 0; i < node->size; ++i) {
		//printf("i = %d %s ok\n", node->keys[i]->size, node->keys[i]->data);
		if (node->keys[i]->data != 0) {
			free(node->keys[i]->data);
		}
		if (node->data[i]->data != 0) {
			free(node->data[i]->data);
		}
	}
	for (i = 0; i < db->t * 2 - 1; ++i) {
		free(node->keys[i]);
		free(node->data[i]);
	}
	//printf("end i\n");
	free(node->keys);
	free(node->data);
	free(node->child);*/
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
	int i;
	
	struct DB *db = (struct DB *)calloc(1, sizeof(struct DB));
	db->f = fopen("mytempfile.txt", "w+");
	db->blockSize = conf->page_size;
	db->t = 6;
	db->numberOfBlocks = 1;
	db->root = 0;
	db->numberOfPages = conf->cache_size / conf->page_size;
	db->cashSize = 0;
	db->L = (struct LIST *) calloc(1,  sizeof(struct LIST));
	db->L->first = 0;
	db->H = (struct HASH *) calloc(1,  sizeof(struct HASH));
	db->H->size = db->numberOfPages * 3;
	db->H->data = (struct LIST **) calloc((db->H->size),  sizeof(struct LIST *));
	for (i = 0; i < db->H->size; ++i) {
		db->H->data[i] = (struct LIST *) calloc(1,  sizeof(struct LIST));
		db->H->data[i]->first = 0;
		db->H->data[i]->last = 0;
	}
		
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
	db->searchHash = mySearchHash;
	db->removeHash = myRemoveHash;
	db->addHash = myAddHash;
	db->remove = myRemove;
	db->add = myAdd;
	
	db->debug = myDebug;
	
	struct POINTER *root = (struct POINTER *)calloc(1, sizeof(struct POINTER));
	root->a = (struct NODE *)calloc(1, sizeof(struct NODE));
	db->allocator(db, root);
	root->a->size = 0;
	root->a->leaf = 1;
	root->a->numberOfBlock = 0;
	db->nodeWriteDisk(db, root->a);
	db->deAllocator(db, root->a);
	
	return db;
}
