/*
 * Table test program.
 *
 * Copyright 2000 by Gray Watson.
 *
 * This file is part of the table package.
 *
 * Permission to use, copy, modify, and distribute this software for
 * any purpose and without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies, and that the name of Gray Watson not be used in advertising
 * or publicity pertaining to distribution of the document or software
 * without specific, written prior permission.
 *
 * Gray Watson makes no representations about the suitability of the
 * software described herein for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * The author may be contacted at http://256/gray/
 *
 * $Id: table_t.c,v 1.12 2000/03/09 03:30:42 gray Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>

#ifdef unix
#include <unistd.h>
#endif

#ifdef win32
#include <windows.h>
#define random rand
#define srandom srand
#endif

#include "table.h"

static	char	*rcs_id =
  "$Id: table_t.c,v 1.12 2000/03/09 03:30:42 gray Exp $";

#define RANDOM_VALUE(x)		((random() % ((x) * 10)) / 10)


#define TABLE_FILE	"table_file.tbl"	/* name of mmap'd test file */


#define SAMPLE_SIZE	1024
#define ITERATIONS	10000
#define MAX_ENTRIES	10240
#define MAX_ORDER_DUMP	10

#define MODE_CLEAR		0		/* then only a 1 in 100 */
#define MODE_INSERT		1		/* store a value into table */
#define MODE_OVERWRITE		2		/* store with overwrite */
#define MODE_RETRIEVE		3		/* retrieve value from table */
#define MODE_DELETE		4		/* delete value */	
#define MODE_DELETE_FIRST	5		/* delete value */	
#define MODE_FIRST		6		/* get first entry in tab */
#define MODE_NEXT		7		/* get next entry in tab */
#define MODE_THIS		8		/* get current entry in tab */
#define MODE_INFO		9		/* info on the table */
#define MODE_ADJUST		10		/* adjust the table */
#define MODE_MAX		11

static	int	mode_weights[] = {
  1,		/* MODE CLEAR */
  10000,	/* MODE INSERT */
  10000,	/* MODE_OVERWRITE */
  10000,	/* MODE_RETRIEVE */
  500,		/* MODE_DELETE */
  500,		/* MODE_DELETE_FIRST */
  3000,		/* MODE_FIRST */
  3000,		/* MODE_NEXT */
  3000,		/* MODE_THIS */
  3000,		/* MODE_INFO */
  10		/* MODE_ADJUST */
};

#define TEST_VALUE		123456789
#define MAX_DATA_SIZE		2157

typedef struct entry_st {
  int			en_free_b;		/* free flag */
  struct entry_st	*en_next_p;		/* next pointer */
  
  void			*en_key;		/* key to store */
  int			en_key_size;		/* size of key */
  void			*en_data;		/* value to store */
  int			en_data_size;		/* size of value */
} entry_t;

/* local vars */
static	int		call_c = 0;		/* number of table calls */
static	char		auto_adjust_b = 0;	/* auto-adjust flag */
static	char		large_b = 0;		/* large table flag */
static	char		verbose_b = 0;		/* verbose messages flag */

/*
 * Fill WHICH data block with random data block and return it.  Pass
 * back its size in SIZE_P.
 */
static	void	*random_block(int *size_p)
{
  char		*buf, *buf_p;
  int		size;
  
  size = RANDOM_VALUE(MAX_DATA_SIZE);
  if (size < 100) {
    size %= sizeof(void *);
  }
  if (size_p != NULL) {
    *size_p = size;
  }
  
  if (size == 0) {
    return NULL;
  }
  
  buf = malloc(size);
  if (buf == NULL) {
    abort();
  }
  
  for (buf_p = buf; buf_p < buf + size; buf_p++) {
    *buf_p = random() % 256;
  }
  
  return buf;
}

/*
 * perform some basic tests
 */
static	void	basic(table_t *tab_p)
{
  long	key;
  int	ret;
  void	*data_p;
  
  (void)printf("Performing basic tests:\n");
  (void)fflush(stdout);
  
  key = TEST_VALUE;
  ret = table_insert(tab_p, &key, sizeof(key), NULL, 0, &data_p, 0);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr,
		  "could not store null pointer of size 0 in table: %s\n",
		 table_strerror(ret));
    exit(1);
  }
  
  /* try the replace */
  ret = table_insert(tab_p, &key, sizeof(key), NULL, 0, &data_p, 1);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr,
		  "could not store null pointer of size 0 in table: %s\n",
		 table_strerror(ret));
    exit(1);
  }
  
  /* try to insert without replace */
  ret = table_insert(tab_p, &key, sizeof(key), NULL, 0, &data_p, 0);
  if (ret != TABLE_ERROR_OVERWRITE) {
    (void)printf("replaced a key in the table with no-overwrite flag: %s\n",
		 table_strerror(ret));
    exit(1);
  }
  
  /* delete the key */
  ret = table_delete(tab_p, &key, sizeof(key), NULL, NULL);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "could not delete test key %ld: %s\n",
		  key, table_strerror(ret));
    exit(1);
  }
  
  /* test to make sure we can insert a NULL with size > 0 */
  ret = table_insert(tab_p, &key, sizeof(key), NULL, 100, &data_p, 0);
  if (ret != TABLE_ERROR_NONE) {
    (void)printf("could not add NULL with size > 0: %s\n",
		 table_strerror(ret));
    exit(1);
  }
  ret = table_delete(tab_p, &key, sizeof(key), NULL, NULL);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "could not delete test key %ld: %s\n",
		  key, table_strerror(ret));
    exit(1);
  }
}

#if 0
/*
 * dump the table to stdout
 */
static	void	dump_table(table_t *tab_p)
{
  int	ret, entry_c;
  long	*data_p, *key_p;
  
  for (ret = table_first(tab_p, (void **)&key_p, NULL, (void **)&data_p, NULL),
	 entry_c = 0;
       ret == TABLE_ERROR_NONE;
       ret = table_next(tab_p, (void **)&key_p, NULL, (void **)&data_p, NULL),
	 entry_c++) {
    (void)printf("%d: key %ld, data %ld\n", entry_c, *key_p, *data_p);
  }
  
  if (ret != TABLE_ERROR_NOT_FOUND) {
    (void)fprintf(stderr, "ERROR: first or next key in table: %s\n",
		  table_strerror(ret));
  }
}
#endif

/*
 * try ITERN random program iterations.
 */
static	void	stress(table_t *tab_p, const int iter_n, const int mmaping_b)
{
  void		*data, *key;
  int		which = 0, mode, weight_total;
  int		iter_c, pnt_c, free_c, ret, ksize, dsize;
  entry_t	*grid, *free_p, *grid_p, *last_p;
  int		linear_b = 0, linear_eof_b = 0;
  
  (void)printf("Performing stress tests with %d iterations:\n", iter_n);
  (void)fflush(stdout);
  
  grid = malloc(sizeof(entry_t) * MAX_ENTRIES);
  if (grid == NULL) {
    (void)printf("problems allocating space for %d entries.\n",
		 MAX_ENTRIES);
    exit(1);
  }
  
  /* initialize free list */
  free_p = grid;
  for (grid_p = grid; grid_p < grid + MAX_ENTRIES; grid_p++) {
    grid_p->en_free_b = 1;
    grid_p->en_key = NULL;
    grid_p->en_key_size = 0;
    grid_p->en_data = NULL;
    grid_p->en_data_size = 0;
    grid_p->en_next_p = grid_p + 1;
  }
  /* redo the last next pointer */
  (grid_p - 1)->en_next_p = NULL;
  free_c = MAX_ENTRIES;
  
#if 0
  /* load the list */
  if (mmaping_b) {
    for (ret = table_first(tab_p, (void **)&key_p, NULL, (void **)&data_p,
			   NULL);
	 ret == TABLE_ERROR_NONE;
	 ret = table_next(tab_p, (void **)&key_p, NULL, (void **)&data_p,
			  NULL)) {
    }
  }
#endif
  
  /* total the weights */
  weight_total = 0;
  for (mode = 0; mode < MODE_MAX; mode++) {
    weight_total += mode_weights[mode];
  }
  
  for (iter_c = 0; iter_c < iter_n;) {
    int		weight;
    
    /* decide what to do */
    weight = RANDOM_VALUE(weight_total) + 1;
    for (mode = 0; mode < MODE_MAX; mode++) {
      weight -= mode_weights[mode];
      if (weight <= 0) {
	break;
      }
    }
    
    /* out of bounds */
    if (mode >= MODE_MAX) {
      continue;
    }
    
    switch (mode) {
      
    case MODE_CLEAR:
      if (mmaping_b || large_b) {
	continue;
      }
      
      call_c++;
      table_clear(tab_p);
      
      /* re-init free list */
      free_p = grid;
      for (grid_p = grid; grid_p < grid + MAX_ENTRIES; grid_p++) {
	if (! grid_p->en_free_b) {
	  if (grid_p->en_key != NULL) {
	    free(grid_p->en_key);
	  }
	  if (grid_p->en_data != NULL) {
	    free(grid_p->en_data);
	  }
	}
	grid_p->en_free_b = 1;
	grid_p->en_next_p = grid_p + 1;
      }
      /* redo the last next pointer */
      (grid_p - 1)->en_next_p = NULL;
      free_c = MAX_ENTRIES;
      linear_b = 0;
      linear_eof_b = 0;
      iter_c++;
      if (verbose_b) {
	(void)printf("table cleared.\n");
	fflush(stdout);
      }
      break;
      
    case MODE_INSERT:
      if (mmaping_b) {
	continue;
      }
      if (free_c > 0) {
	which = RANDOM_VALUE(free_c);
	last_p = NULL;
	grid_p = free_p;
	for (pnt_c = 0; pnt_c < which && grid_p != NULL; pnt_c++) {
	  last_p = grid_p;
	  grid_p = grid_p->en_next_p;
	}
	if (grid_p == NULL) {
	  (void)printf("reached end of free list prematurely\n");
	  exit(1);
	}
	
	do {
	  key = random_block(&ksize);
	} while (key == NULL);
	data = random_block(&dsize);
	
	call_c++;
	ret = table_insert(tab_p, key, ksize, data, dsize, NULL, 0);
	if (ret == TABLE_ERROR_NONE) {
	  if (verbose_b) {
	    (void)printf("stored in pos %d: %d, %d bytes of key, data\n",
			 grid_p - grid, ksize, dsize);
	    fflush(stdout);
	  }
	  
	  grid_p->en_free_b = 0;
	  grid_p->en_key = key;
	  grid_p->en_key_size = ksize;
	  grid_p->en_data = data;
	  grid_p->en_data_size = dsize;
	  
	  /* shift free list */
	  if (last_p == NULL) {
	    free_p = grid_p->en_next_p;
	  }
	  else {
	    last_p->en_next_p = grid_p->en_next_p;
	  }
	  grid_p->en_next_p = NULL;
	  free_c--;
	  iter_c++;
	}
	else {
	  for (grid_p = grid; grid_p < grid + MAX_ENTRIES; grid_p++) {
	    if (grid_p->en_free_b) {
	      continue;
	    }
	    if (grid_p->en_key_size == ksize
		&& memcmp(grid_p->en_key, key, ksize) == 0) {
	      break;
	    }
	  }
	  
	  /* if we did not store it then error */
	  if (grid_p >= grid + MAX_ENTRIES) {
	    (void)fprintf(stderr, "ERROR storing #%d: %s\n",
			  which, table_strerror(ret));
	  }
	  if (key != NULL) {
	    free(key);
	  }
	  if (data != NULL) {
	    free(data);
	  }
	}
      }
      break;
      
    case MODE_OVERWRITE:
      if (mmaping_b) {
	continue;
      }
      if (free_c < MAX_ENTRIES) {
	which = RANDOM_VALUE(MAX_ENTRIES);
	
	if (grid[which].en_free_b) {
	  continue;
	}
	
	data = random_block(&dsize);
	
	call_c++;
	ret = table_insert(tab_p, grid[which].en_key, grid[which].en_key_size,
			   data, dsize, NULL, 1);
	if (ret == TABLE_ERROR_NONE) {
	  if (verbose_b) {
	    (void)printf("overwrite pos %d with data of %d bytes\n",
			 which, dsize);
	    fflush(stdout);
	  }
	  grid[which].en_free_b = 0;
	  if (grid[which].en_data != NULL) {
	    free(grid[which].en_data);
	  }
	  grid[which].en_data = data;
	  grid[which].en_data_size = dsize;
	  grid[which].en_next_p = NULL;
	  free_c--;
	  iter_c++;
	}
	else {
	  (void)fprintf(stderr, "ERROR overwriting #%d: %s\n",
			which, table_strerror(ret));
	  free(data);
	}
      }
      break;
      
    case MODE_RETRIEVE:
      if (free_c < MAX_ENTRIES) {
	which = RANDOM_VALUE(MAX_ENTRIES);
	
	if (grid[which].en_free_b) {
	  continue;
	}
	
	call_c++;
	ret = table_retrieve(tab_p, grid[which].en_key, grid[which].en_key_size,
			     (void **)&data, &dsize);
	if (ret == TABLE_ERROR_NONE) {
	  if (grid[which].en_data_size == dsize
	      && memcmp(grid[which].en_data, data, dsize) == 0) {
	    if (verbose_b) {
	      (void)printf("retrieved key #%d, got data of %d bytes\n",
			   which, dsize);
	      fflush(stdout);
	    }
	  }
	  else {
	    (void)fprintf(stderr,
			  "ERROR: retrieve key #%d: data (%d bytes) didn't "
			  "match table (%d bytes)\n",
			  which, grid[which].en_data_size, dsize);
	  }
	  iter_c++;
	}
	else {
	  (void)fprintf(stderr, "error retrieving key #%d: %s\n",
			which, table_strerror(ret));
	}
      }
      break;
      
    case MODE_DELETE:
      if (mmaping_b) {
	continue;
      }
      if (free_c >= MAX_ENTRIES) {
	continue;
      }
      
      which = RANDOM_VALUE(MAX_ENTRIES);
      
      if (grid[which].en_free_b) {
	continue;
      }
      
      call_c++;
      ret = table_delete(tab_p, grid[which].en_key, grid[which].en_key_size,
			 (void **)&data, &dsize);
      if (ret == TABLE_ERROR_NONE) {
	if (grid[which].en_data_size == dsize
	    && memcmp(grid[which].en_data, data, dsize) == 0) {
	  if (verbose_b) {
	    (void)printf("deleted key #%d, got data of %d bytes\n",
			 which, dsize);
	    fflush(stdout);
	  }
	}
	else {
	  (void)fprintf(stderr,
			"ERROR deleting key #%d: data didn't match table\n",
			which);
	}
	grid[which].en_free_b = 1;
	if (grid[which].en_key != NULL) {
	  free(grid[which].en_key);
	}
	if (grid[which].en_data != NULL) {
	  free(grid[which].en_data);
	}
	grid[which].en_next_p = free_p;
	free_p = grid + which;
	free_c++;
	if (free_c == MAX_ENTRIES) {
	  linear_b = 0;
	  linear_eof_b = 0;
	}
	iter_c++;
	if (data != NULL) {
	  free(data);
	}
      }
      else {
	(void)fprintf(stderr, "ERROR deleting key %d: %s\n",
		      which, table_strerror(ret));
      }
      break;
      
    case MODE_DELETE_FIRST:
      /*
       * We have a problem here.  This is the only action routine
       * which modifies the table and is not key based.  We don't have
       * a way of looking up the key in our local data structure.
       */
      break;
      
    case MODE_FIRST:
      call_c++;
      ret = table_first(tab_p, (void **)&key, &ksize, (void **)&data, &dsize);
      if (ret == TABLE_ERROR_NONE) {
	linear_b = 1;
	linear_eof_b = 0;
	if (verbose_b) {
	  (void)printf("first entry has key, data of %d, %d bytes\n",
		       ksize, dsize);
	  fflush(stdout);
	}
	iter_c++;
      }
      else if (free_c == MAX_ENTRIES) {
	if (verbose_b) {
	  (void)printf("no first in table\n");
	  fflush(stdout);
	}
      }
      else {
	(void)fprintf(stderr, "ERROR: first in table: %s\n",
		      table_strerror(ret));
      }
      break;
      
    case MODE_NEXT:
      call_c++;
      ret = table_next(tab_p, (void **)&key, &ksize, (void **)&data, &dsize);
      if (ret == TABLE_ERROR_NONE) {
	if (verbose_b) {
	  (void)printf("next entry has key, data of %d, %d\n",
		       ksize, dsize);
	  fflush(stdout);
	}
	iter_c++;
      }
      else if (ret == TABLE_ERROR_LINEAR && (! linear_b)) {
	if (verbose_b) {
	  (void)printf("no first command run yet\n");
	  fflush(stdout);
	}
      }
      else if (ret == TABLE_ERROR_NOT_FOUND) {
	if (verbose_b) {
	  (void)printf("reached EOF with next in table: %s\n",
		       table_strerror(ret));
	  fflush(stdout);
	}
	linear_b = 0;
	linear_eof_b = 1;
      }
      else {
	(void)fprintf(stderr, "ERROR: table_next reports: %s\n",
		      table_strerror(ret));
	linear_b = 0;
	linear_eof_b = 0;
      }
      break;
      
    case MODE_THIS:
      call_c++;
      ret = table_this(tab_p, (void **)&key, &ksize, (void **)&data, &dsize);
      if (ret == TABLE_ERROR_NONE) {
	if (verbose_b) {
	  (void)printf("this entry has key,data of %d, %d bytes\n",
		       ksize, dsize);
	  fflush(stdout);
	}
	iter_c++;
      }
      else if (ret == TABLE_ERROR_LINEAR && (! linear_b)) {
	if (verbose_b) {
	  (void)printf("no first command run yet\n");
	  fflush(stdout);
	}
      }
      else if (ret == TABLE_ERROR_NOT_FOUND || linear_eof_b) {
	if (verbose_b) {
	  (void)printf("table linear already reached EOF\n");
	  fflush(stdout);
	}
      }
      else {
	(void)fprintf(stderr, "ERROR: this table: %s\n", table_strerror(ret));
	linear_b = 0;
	linear_eof_b = 0;
      }
      break;
      
    case MODE_INFO:
      {
	int	buckets, entries;
	
	call_c++;
	ret = table_info(tab_p, &buckets, &entries);
	if (ret == TABLE_ERROR_NONE) {
	  if (verbose_b) {
	    (void)printf("table has %d buckets, %d entries\n",
			 buckets, entries);
	    fflush(stdout);
	  }
	  iter_c++;
	}
	else {
	  (void)fprintf(stderr, "ERROR: table info: %s\n",
			table_strerror(ret));
	}
      }
    break;
    
    case MODE_ADJUST:
      {
	int	buckets, entries;
	
	if (mmaping_b || auto_adjust_b || large_b) {
	  continue;
	}
	
	call_c++;
	ret = table_info(tab_p, &buckets, &entries);
	if (ret == TABLE_ERROR_NONE) {
	  if (entries == 0) {
	    if (verbose_b) {
	      (void)printf("cannot adjusted table, %d entries\n", entries);
	      fflush(stdout);
	    }
	  }
	  else if (buckets == entries) {
	    if (verbose_b) {
	      (void)printf("no need to adjust table, %d buckets and entries\n",
			   buckets);
	      fflush(stdout);
	    }
	  }
	  else {
	    ret = table_adjust(tab_p, entries);
	    if (ret == TABLE_ERROR_NONE) {
	      (void)printf("adjusted table from %d to %d buckets\n",
			   buckets, entries);
	      iter_c++;
	    }
	    else {
	      (void)printf("ERROR: table adjust to %d buckets: %s\n",
			   entries, table_strerror(ret));
	    }
	  }
	}
	else {
	  (void)fprintf(stderr, "ERROR: table info: %s\n",
			table_strerror(ret));
	}
      }
      break;
      
    default:
      (void)printf("unknown mode %d\n", which);
      break;
    }
  }
  
  /* run through the grid and free the entries */
  for (grid_p = grid; grid_p < grid + MAX_ENTRIES; grid_p++) {
    if (! grid_p->en_free_b) {
      if (grid_p->en_key != NULL) {
	free(grid_p->en_key);
      }
      if (grid_p->en_data != NULL) {
	free(grid_p->en_data);
      }
    }
  }
  
  /* free used pointers */
  free(grid);
}

/*
 * compare the keys in two tables.  returns 1 if equal else 0
 */
static	int	test_eq(table_t *tab1_p, table_t *tab2_p, const int verb_b)
{
  int	ret, eq = 1, key_size, data1_size, data2_size;
  void	*key_p, *data1_p, *data2_p;
  
  /* test the table entries */
  for (ret = table_first(tab1_p, (void **)&key_p, &key_size,
			 (void **)&data1_p, &data1_size);
       ret == TABLE_ERROR_NONE;
       ret = table_next(tab1_p, (void **)&key_p, &key_size,
			(void **)&data1_p, &data1_size)) {
    ret = table_retrieve(tab2_p, key_p, key_size,
			 (void **)&data2_p, &data2_size);
    if (ret != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "could not find key of %d bytes: %s\n",
		    key_size, table_strerror(ret));
      eq = 0;
    }
    else if (data1_size == data2_size
	     && memcmp(data1_p, data2_p, data1_size) == 0) {
      if (verb_b) {
	(void)printf("key of %d bytes, data of %d bytes\n",
		     key_size, data1_size);
	fflush(stdout);
      }
    }
    else {
      (void)fprintf(stderr,
		    "ERROR: key of %d bytes: data (size %d) != other "
		    "(size %d)\n",
		    key_size, data1_size, data2_size);
      eq = 0;
    }
  }
  
  if (ret != TABLE_ERROR_NOT_FOUND) {
    eq = 0;
  }
  return eq;
}

/*
 * perform some basic tests
 */
static	void	io_test(table_t *tab_p)
{
  int		ret, bucket_n, entry_n;
  table_t	*tab2_p;
  
  (void)printf("Performing I/O tests:\n");
  (void)fflush(stdout);
  
#if 0
  {
    long	key, data;
    (void)table_clear(tab_p);
    key = 1;
    data = 2;
    (void)table_insert(tab_p, &key, sizeof(key), &data, sizeof(data), NULL, 0);
    key = 3;
    data = 4;
    (void)table_insert(tab_p, &key, sizeof(key), &data, sizeof(data), NULL, 0);
    key = 5;
    data = 6;
    (void)table_insert(tab_p, &key, sizeof(key), &data, sizeof(data), NULL, 0);
    (void)table_adjust(tab_p, 0);
    dump_table(tab_p);
  }
#endif
  
  ret = table_info(tab_p, &bucket_n, &entry_n);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "could not get info of table: %s\n",
		  table_strerror(ret));
    exit(1);
  }
  (void)printf("Table we are writing has %d buckets and %d entries\n",
	       bucket_n, entry_n);
  
  /*
   * dump the table to disk
   */
  int pmode = 0640;
#ifdef win32
  pmode = _S_IREAD | _S_IWRITE;
#endif
  (void)unlink(TABLE_FILE);
  ret = table_write(tab_p, TABLE_FILE, pmode);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "could not write table to '%s': %s\n",
		  TABLE_FILE, table_strerror(ret));
    exit(1);
  }
  
#if 0
  dump_table(tab_p);
#endif
  
  /*
   * now read back in the table
   */
  tab2_p = table_read(TABLE_FILE, &ret);
  if (tab2_p == NULL) {
    (void)fprintf(stderr, "could not read in file '%s': %s\n",
		  TABLE_FILE, table_strerror(ret));
    exit(1);
  }
  
  (void)printf("Testing table-read...\n");
  if (test_eq(tab_p, tab2_p, 0)) {
    (void)printf("  equal.\n");
  }
  else {
    (void)printf("  NOT equal.\n");
  }
  
  ret = table_free(tab2_p);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "could not free read table: %s\n",
		  table_strerror(ret));
    exit(1);
  }
  
  /*
   * mmap in the table
   */
  tab2_p = table_mmap(TABLE_FILE, &ret);
  if (tab2_p == NULL) {
    (void)fprintf(stderr, "could not mmap file '%s': %s\n",
		  TABLE_FILE, table_strerror(ret));
    exit(1);
  }
  
  (void)printf("Testing table-mmap...\n");
  if (test_eq(tab2_p, tab_p, 0)) {
    (void)printf("  equal.\n");
  }
  else {
    (void)printf("  NOT equal.\n");
  }
  
  ret = table_munmap(tab2_p);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "could not munmap file '%s': %s\n",
		  TABLE_FILE, table_strerror(ret));
    exit(1);
  }
}

/*
 * perform some basic tests
 */
static	void	order_test(table_t *tab_p)
{
  table_entry_t		**entries, **entries_p;
  table_linear_t	*linears, *linears_p;
  void			*key_p, *last_key;
  void			*key, *data, *key2, *data2;
  int			error, entry_n, last_ksize, size, cmp;
  int			ret, ksize, dsize, ksize2, dsize2;
  
  (void)printf("Performing ordering tests:\n");
  (void)fflush(stdout);
  
  /* order the table */
  entries = table_order(tab_p, NULL, &entry_n, &error);
  if (entries == NULL) {
    (void)fprintf(stderr, "could not order table: %s\n",
		  table_strerror(error));
    exit(1);
  }
  
  /* order the table with the pos method */
  linears = table_order_pos(tab_p, NULL, &entry_n, &error);
  if (linears == NULL) {
    (void)fprintf(stderr, "could not order-pos table: %s\n",
		  table_strerror(error));
    exit(1);
  }
  
  last_key = NULL;
  last_ksize = 0;
  
  for (entries_p = entries, linears_p = linears;
       entries_p < entries + entry_n;
       entries_p++, linears_p++) {
    
    /* get the entry */
    error = table_entry(tab_p, *entries_p, &key, &ksize, &data, &dsize);
    if (error != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "could not get-entry from table: %s\n",
		    table_strerror(error));
      exit(1);
    }
    
    if (verbose_b) {
      for (key_p = key;
	   (char *)key_p < (char *)key + ksize
	     && (char *)key_p < (char *)key + MAX_ORDER_DUMP;
	   key_p = (char *)key_p + 1) {
	(void)printf("%03d ", *(unsigned char *)key_p);
      }
      if (ksize > MAX_ORDER_DUMP) {
	(void)fputs("... ", stdout);
      }
      (void)printf("(%d)\n", ksize);
    }
    
    /* get the pos entry */
    error = table_entry_pos(tab_p, linears_p, &key2, &ksize2, &data2, &dsize2);
    if (error != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "could not get-entry-pos from table: %s\n",
		    table_strerror(error));
      exit(1);
    }
    
    /* make sure we get the same data */
    if (ksize != ksize2
	|| memcmp(key, key2, ksize) != 0
	|| dsize != dsize2
	|| memcmp(data, data2, dsize) != 0) {
      (void)fprintf(stderr,
		    "ERROR: entry and entry-pos output does not match\n");
      exit(1);
    }
    
    if (entries_p == entries) {
      continue;
    }
    
    /* compare with the last key */
    size = last_ksize;
    if (ksize < size) {
      size = ksize;
    }
    cmp = memcmp(last_key, key, size);
    if (cmp > 0	|| (cmp == 0 && last_ksize > ksize)) {
      (void)fprintf(stderr, "ERROR: entries not in order!!\n");
      exit(1);
    }
  }
  
  if (entries_p > entries + 1) {
    size = last_ksize;
    if (ksize < size) {
      size = ksize;
    }
    cmp = memcmp(last_key, key, size);
    if (cmp > 0	|| (cmp == 0 && last_ksize > ksize)) {
      (void)fprintf(stderr, "ERROR: entries not in order!!\n");
      exit(1);
    }
  }
  
  /* free the ordered arrays */
  ret = table_order_free(tab_p, entries, entry_n);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "could not free table order array: %s\n",
		  table_strerror(ret));
  }
  ret = table_order_pos_free(tab_p, linears, entry_n);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "could not free table order pos array: %s\n",
		  table_strerror(ret));
  }
}

/*
 * print the usage message
 */
static	void	usage(void)
{
  (void)fprintf(stderr,
		"Usage: table_t [-alnv] [-A alignment] [-i iter-num] "
		"[-s random-seed]\n");
  (void)fprintf(stderr,	"  -a              turn auto-adjusting on\n");
  (void)fprintf(stderr,	"  -A align        set data alignment for table\n");
  (void)fprintf(stderr,	"  -i iter-num     set the number of iterations\n");
  (void)fprintf(stderr,	"  -l              build a large table in memory\n");
  (void)fprintf(stderr,	"  -s              set the random seed value\n");
  (void)fprintf(stderr,	"  -v              verbose_b messages\n");
  exit(1);      
}

int	main(int argc, char ** argv)
{
  table_t	*tab;
  int		ret, iter_n = ITERATIONS, alignment = 0;
  long		seed;
  
  argc--, argv++;
  
  /* set default seed */
  seed = time(NULL);
  
  /* process the args */
  for (; *argv != NULL; argv++, argc--) {
    if (**argv != '-') {
      continue;
    }
    
    switch (*(*argv + 1)) {
      
    case 'a':
      auto_adjust_b = 1;
      break;
      
    case 'A':
      if (argc > 0) {
	argv++, argc--;
	if (argc == 0) {
	  usage();
	}
	alignment = atoi(*argv);
      }
      break;
      
    case 'i':
      if (argc > 0) {
	argv++, argc--;
	if (argc == 0) {
	  usage();
	}
	iter_n = atoi(*argv);
      }
      break;
      
    case 'l':
      large_b = 1;
      break;
      
    case 's':
      if (argc > 0) {
	argv++, argc--;
	if (argc == 0) {
	  usage();
	}
	seed = atol(*argv);
      }
      break;
      
    case 'v':
      verbose_b = 1;
      break;
      
    default:
      usage();
      break;
    }
  }
  
  if (argc > 0) {
    usage();
  }
  
  (void)srandom(seed);
  (void)printf("Seed for random is %ld\n", seed);
  
  /* alloc the test table */
  call_c++;
  tab = table_alloc(0, &ret);
  if (tab == NULL) {
    (void)printf("table_alloc returned: %s\n", table_strerror(ret));
    exit(1);
  }
  
  if (auto_adjust_b) {
    ret = table_attr(tab, TABLE_FLAG_AUTO_ADJUST);
    if (ret != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "ERROR: could not set table for auto-adjust: %s\n",
		    table_strerror(ret));
      exit(1);
    }
  }
  
  if (alignment > 0) {
    ret = table_set_data_alignment(tab, alignment);
    if (ret != TABLE_ERROR_NONE) {
      (void)fprintf(stderr, "ERROR: could not set data alignment to %d: %s\n",
		    alignment, table_strerror(ret));
      exit(1);
    }
  }
  
  basic(tab);
  stress(tab, iter_n, 0);
  io_test(tab);
  order_test(tab);
  
  call_c++;
  ret = table_free(tab);
  if (ret != TABLE_ERROR_NONE) {
    (void)fprintf(stderr, "ERROR in table_free: %s\n", table_strerror(ret));
  }
  
  (void)fputc('\n', stdout);
  (void)printf("Test program performed %d table calls\n", call_c);
  
  exit(0);
}
