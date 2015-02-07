#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <err.h>

#include <dynamic.h>
#include <unordered_map>
#include <sparsehash/dense_hash_map>
#include <ulib/hash_func.h>
#include <ulib/hash_open.h>

using namespace ulib;

#if STANDARD_INT
#define VARIANT_DECLARE std::unordered_map <int, int> *m;
#define VARIANT_NEW     m = new std::unordered_map<int, int>(); m->max_load_factor(0.5);
#define VARIANT_INSERT  m->insert(std::make_pair(data[j], j));
#define VARIANT_LOOKUP  sum += (*m)[data[j]];
#define VARIANT_DELETE  delete m;
#elif DYNAMIC_INT
#define VARIANT_DECLARE map_int *m;   
#define VARIANT_NEW     m = map_int_new(sizeof i, -1, -2);
#define VARIANT_INSERT  map_int_insert(m, data[j], &j);
#define VARIANT_LOOKUP  sum += *(int *) map_int_at(m, data[j]);
#define VARIANT_DELETE  map_int_free(m);
#elif DENSEHASH_INT
#define VARIANT_DECLARE google::dense_hash_map<int, int> *m;
#define VARIANT_NEW     m = new google::dense_hash_map<int, int>(); m->set_empty_key(-1); m->max_load_factor(0.5);
#define VARIANT_INSERT  m->insert(std::make_pair(data[j], j));
#define VARIANT_LOOKUP  sum += (*m)[data[j]];
#define VARIANT_DELETE  delete m;
#elif ULIB_INT
#define VARIANT_DECLARE open_hash_map<int, int> *m;
#define VARIANT_NEW     m = new open_hash_map<int, int>();
#define VARIANT_INSERT  (*m)[data[j]] = j;
#define VARIANT_LOOKUP  sum += (*m)[data[j]];
#define VARIANT_DELETE  delete m;
#else
#error "Please define a variant"
#endif 

void usage()
{
  extern char *__progname;

  (void) fprintf(stderr, "usage: %s <size> <rounds>\n", __progname);
  exit(1);
}

vector *create_random_dataset(int size)
{
  vector *dataset;
  int i, n;
  
  dataset = vector_new(sizeof(int));
  if (!dataset)
    return NULL;

  for (i = 0; i < size; i ++)
    {
      n = rand();
      vector_push_back(dataset, &n);
    }

  return dataset;
}

void shuffle(int *data, int size)
{
  int tmp, i, r;

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

int main(int argc, char **argv)
{
  long sum, i, j, k, n;
  uint64_t t1, t2;
  double t_lookup, t_insert;
  vector *dataset;
  int *data;

  VARIANT_DECLARE;

  if (argc != 3)
    usage();
  
  n = strtol(argv[1], NULL, 0);
  k = strtol(argv[2], NULL, 0);
  if (n <= 0 || k <= -1)
    usage();

  t_insert = 0;
  t_lookup = 0;
  for (i = 0; i < k; i ++)
    {
      sum = 0;
      dataset = create_random_dataset(n);
      if (!dataset)
	err(1, "create_random_dataset");

      data = (int *) vector_data(dataset);

      VARIANT_NEW
      if (!m)
	err(1, "map_str_new");
      
      t1 = ntime();
      for (j = 0; j < n; j ++)
	VARIANT_INSERT
      t2 = ntime();
      t_insert += t2 - t1;
      
      shuffle(data, n);
      t1 = ntime();
      for (j = 0; j < n; j ++)
	VARIANT_LOOKUP
      t2 = ntime();
      t_lookup += t2 - t1;

      VARIANT_DELETE
      vector_free(dataset);
    }

  t_insert /= 1000000000.;
  t_lookup /= 1000000000.;
  
  (void) fprintf(stdout, "%.2f %.2f\n", k * n / t_insert, k * n / t_lookup);
}

