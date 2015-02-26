#include <string>

#define min(x, y) (x < y ? x : y)

struct DBC {
	size_t db_size;
	size_t chunk_size;
	size_t mem_size;
};

typedef int (*db_put_t)(void *, const char *, size_t, const char *, size_t);
typedef int (*db_get_t)(void *, const char *, size_t, char **, size_t *);
typedef int (*db_del_t)(void *, const char *, size_t);
typedef int (*db_adm_t)(void *);

typedef void *(*db_open_t)(char *);
typedef void *(*db_create_t)(char *, struct DBC *);

class Database {
private:
	std::string so_path;
	std::string db_path;

	void *so_handle;
	void *db_object;

	db_put_t put_function;
	db_get_t get_function;
	db_del_t del_function;

	db_adm_t close_function;
	db_adm_t flush_function;

	db_open_t open_function;
	db_create_t create_function;
public:
	Database(const char *so_path, const char *db_path);
	~Database();
	int put(const std::string &key, const std::string &val);
	int get(const std::string &key, char **val, size_t *val_size);
	int del(const std::string &key);
	int close();
};

