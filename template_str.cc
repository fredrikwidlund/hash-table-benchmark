#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <err.h>

#include <dynamic.h>
#include <farmhash.h>
#include <unordered_map>
#include <sparsehash/dense_hash_map>
#include <ulib/hash_func.h>
#include <ulib/hash_open.h>
#include <tommyds/tommyhashdyn.h>

using namespace ulib;

/* tommyds support routines */

struct object {
  tommy_node node;
  char *key;
  int value;
};

int compare(const void *arg, const void *obj)
{
  return strcmp(*(char **) arg, ((struct object *) obj)->key);
}

/* ulib support routines */

struct str {
  const char *c_str;

  str(const char *s = 0) : c_str(s) { }

  operator size_t() const
  {
    return util::Hash64(c_str, strlen(c_str));
  }

  bool operator==(const str &other) const
  {
    return strcmp(c_str, other.c_str) == 0;
  }
};

#if STANDARD_STR
#define VARIANT_DECLARE std::unordered_map <const char *, int, struct hash, struct equal_to> *m;
#define VARIANT_NEW     m = new std::unordered_map<const char *, int, struct hash, struct equal_to>(); m->max_load_factor(0.5);
#define VARIANT_INSERT  m->insert(std::make_pair(data[j], j));
#define VARIANT_LOOKUP  sum += (*m)[data[j]];
#define VARIANT_DELETE  delete m;
#elif DYNAMIC_STR
#define VARIANT_DECLARE map_str *m;   
#define VARIANT_NEW     m = map_str_new(sizeof i);
#define VARIANT_INSERT  map_str_insert(m, data[j], &j);
#define VARIANT_LOOKUP  sum += *(int *) map_str_at(m, data[j]);
#define VARIANT_DELETE  map_str_free(m);
#elif DENSEHASH_STR
#define VARIANT_DECLARE google::dense_hash_map<const char *, uint32_t, struct hash, struct equal_to> *m;
#define VARIANT_NEW     m = new google::dense_hash_map<const char *, uint32_t, struct hash, struct equal_to>(); m->set_empty_key(""); m->max_load_factor(0.5);
#define VARIANT_INSERT  m->insert(std::make_pair(data[j], j));
#define VARIANT_LOOKUP  sum += (*m)[data[j]];
#define VARIANT_DELETE  delete m;
#elif ULIB_STR
#define VARIANT_DECLARE open_hash_map<str, int> *m;
#define VARIANT_NEW     m = new open_hash_map<str, int>();
#define VARIANT_INSERT  (*m)[data[j]] = j;
#define VARIANT_LOOKUP  sum += (*m)[data[j]];
#define VARIANT_DELETE  delete m;
#elif TOMMYDS_STR
#define VARIANT_DECLARE tommy_hashdyn *m;
#define VARIANT_NEW     m = (tommy_hashdyn *) malloc(sizeof *m); tommy_hashdyn_init(m);
#define VARIANT_INSERT  {struct object *object = (struct object *) malloc(sizeof(*object)); object->key = data[j]; object->value = j; tommy_hashdyn_insert(m, &object->node, object, util::Hash64(data[j], strlen(data[j])));}
#define VARIANT_LOOKUP  {struct object* object = (struct object *) tommy_hashdyn_search(m, compare, &data[j], util::Hash64(data[j], strlen(data[j]))); sum += object->value;}
#define VARIANT_DELETE  tommy_hashdyn_done(m); free(m); /* NOTE: we need to delete all objects to avoid leaks */
#else
#error "Please define a variant"
#endif 

void usage()
{
  extern char *__progname;

  (void) fprintf(stderr, "usage: %s <size> <rounds>\n", __progname);
  exit(1);
}

vector *load(char *path)
{
  string *data;
  vector *lines;
  
  data = io_string_load(path);
  if (!data)
    return NULL;
  
  lines = string_split(data, (char *) "\n");
  string_free(data);

  return lines;
}

void dataset_release(void *object)
{
  free(*(char **) object);
}

vector *create_random_dataset(vector *dict, int size, int words)
{
  vector *dataset;
  string *s;
  int i, j;
  char *entry;
  
  dataset = vector_new(sizeof(char *));
  if (!dataset)
    return NULL;

  vector_release(dataset, dataset_release);

  for (i = 0; i < size; i ++)
    {
      s = string_new((char *) "");
      for (j = 0; j < words; j ++)
	{
	  string_append(s, (char *) "/");
	  string_append(s, string_data(*(string **) vector_at(dict, rand() % vector_size(dict))));
	}
      entry = string_deconstruct(s);

      vector_push_back(dataset, &entry);
    }

  return dataset;
}

void shuffle(char **data, int size)
{
  char *tmp;
  int i, r;

  for (i = 0; i < size; i ++)
    {
      tmp = data[i];
      r = i + rand() % (size - i);
      data[i] = data[r];
      data[r] = tmp;       
    }
}

uint64_t ntime()
{
  struct timespec ts;

  (void) clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
  return ((uint64_t) ts.tv_sec * 1000000000) + ((uint64_t) ts.tv_nsec);
}

struct hash
{
  size_t operator() (const char *key) const {
    return util::Hash64(key, strlen(key));
  }  
};

struct equal_to
{
  size_t operator() (const char *a, const char *b) const {
    return strcmp(a, b) == 0;
  }
};

int t_compare(const void *a, const void *b)
{
  return *(double *) b - *(double *) a;
}

int main(int argc, char **argv)
{
  long sum, i, j, k, n;
  uint64_t t1, t2;
  vector *dataset;
  vector *dict;
  char **data;

  VARIANT_DECLARE;

  if (argc != 3)
    usage();
  
  n = strtol(argv[1], NULL, 0);
  k = strtol(argv[2], NULL, 0);
  if (n <= 0 || k <= -1)
    usage();

  dict = load((char *) "propernames");
  if (!dict)
    err(1, "load");

  double t_lookup[k], t_insert[k];
  for (i = 0; i < k; i ++)
    {
      sum = 0;
      dataset = create_random_dataset(dict, n, 4);
      if (!dataset)
	err(1, "create_random_dataset");

      data = (char **) vector_data(dataset);

      VARIANT_NEW
      if (!m)
	err(1, "map_str_new");
      
      t1 = ntime();
      for (j = 0; j < n; j ++)
	VARIANT_INSERT
      t2 = ntime();
      t_insert[i] = t2 - t1;
      
      shuffle(data, n);
      t1 = ntime();
      for (j = 0; j < n; j ++)
	VARIANT_LOOKUP
      t2 = ntime();
      t_lookup[i] = t2 - t1;

      VARIANT_DELETE
      vector_free(dataset);

      if (sum != (n / 2) * (n - 1))
	errx(1, "invalid checksum");
    }

  qsort(t_insert, k, sizeof t_insert[0], t_compare);
  qsort(t_lookup, k, sizeof t_lookup[0], t_compare);
  
  (void) fprintf(stdout, "%f %f\n", 1000000000 * n / t_insert[k / 2], 1000000000 * n / t_lookup[k / 2]);

  vector_free(dict);
}

