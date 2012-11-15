/*
    Audio Scout - audio content indexing software
    Copyright (C) 2010  D. Grant Starkweather & Evan Klinger
    
    Audio Scout is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    D. Grant Starkweather - dstarkweather@phash.org
    Evan Klinger          - eklinger@phash.org
*/



#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <locale.h>
#include <sndfile.h>
#include <samplerate.h>
#include <zmq.h>
#include "serialize.h"
#include "phash_audio.h"
#include "audiodata.h"

struct globalargs_t {
    char *cmd;        /* build|combine|info|query|queryd */ 
    char *dir_name;   /* directory or file name */
    char *server_addr;/*  meta data db server address - e.g. "tcp://localhost:4000 */ 
    char *index_name; /* name of index to operate on */
    char *dest_index; /* -d destination index to which to add entries from source index */ 
    char *src_index;  /* -s source index to add into the destination index*/ 
    int P;            /* -p number of bits to toggle in creating more candidates to lookup */
    int sr;          
    int blocksize;    /* -b block size for query operation */
    int verbosity;    /* -v */
    int help;         /* -h */
    float nbsecs;     /* -n number seconds of audio to hash from file*/
    float threshold;  /* -t query threshold 0.0-0.10 */ 
    int port;         
}GlobalArgs;


static const char *opt_string = "l:p:t:n:b:d:s:vh?";

static const struct option longOpts[] = {
    { "dbserver", required_argument, NULL, 's'},
    { "toggles", required_argument,   NULL, 'p'},
    { "blocksize", required_argument,  NULL, 'b'},
    { "nbsecs", required_argument,    NULL, 'n'},
    { "threshold", required_argument, NULL, 't'},
    { "verbose", no_argument,         NULL, 'v'},
    { "help", no_argument,            NULL, 'h'},
    { "port", required_argument,      NULL,  0},
    { NULL, no_argument,            NULL,  0}
};


int addtoaudioindex(const char *dir_name, const char *idx_name, const int sr, \
                    const float nbsecs, const unsigned int P){

    const int initial_nbbuckets = 1 << 25;

    char indexfile[FILENAME_MAX];
    snprintf(indexfile, FILENAME_MAX, "%s.idx", idx_name);

    fprintf(stdout, "open index table at %s\n", indexfile);
    AudioIndex index_table = open_audioindex(indexfile, 1, initial_nbbuckets);
    if (index_table == NULL){
	fprintf(stderr,"unable to get table\n");
	return -1;
    }

    void *ctx = zmq_init(1);
    if (ctx == NULL){
	fprintf(stderr,"unable to init zeromq\n");
	return -1;
    }

    AudioDataDB mdatastore = open_audiodata_db(ctx, GlobalArgs.server_addr);
    if (mdatastore == NULL) {
	fprintf(stderr,"unable to connect to metadata server\n");
	return -2;
    }
    fprintf(stdout, "audio data db: %p\n", mdatastore);
    printf("read files from dir\n");
    printf("dir: %s\n", dir_name);
    unsigned int nbfiles;
    char **files = readfilenames(dir_name, &nbfiles);

    if (files == NULL){
	return -3;
    }
    printf("number files %u\n", nbfiles);

    float *sigbuf = (float*)malloc(1<<25);
    unsigned int buflen = (1<<25)/sizeof(float);
    if (sigbuf == NULL){
	return -4;
    }

    AudioMetaData mdata;
    AudioHashStInfo hash_st;
    hash_st.sr = 0;
    char inlinestr[512];
    uint32_t hash_id;
    unsigned int i;
    for (i=0;i<nbfiles;i++){
        fprintf(stdout,"file[%d]: %s\n", i, files[i]);

	unsigned int tmpbuflen = buflen;
	int err;
	float *buf = readaudio(files[i], sr, sigbuf, &tmpbuflen, nbsecs, &mdata, &err);
	if (buf == NULL){
	  fprintf(stderr,"unable to read audio, err = %d\n", err);
	  continue;
	}
	fprintf(stdout,"signal length %u\n", tmpbuflen);

	if (metadata_to_inlinestr(&mdata, inlinestr, 512) < 0){
	    fprintf(stderr, "ERROR: cannot parse metadata struct\n");
	    break;
	}
	fprintf(stdout,"mdata: %s\n", inlinestr);
	if (store_audiodata(mdatastore, inlinestr, &hash_id)<0){
	    fprintf(stderr,"ERROR: cannot store metadata\n");
	    break;
	}
	fprintf(stdout, "uid = %u\n", hash_id);

	uint32_t *phash = NULL;
	unsigned int nbframes;
	if (audiohash(buf, &phash, NULL, NULL, NULL, &nbframes,\
		      NULL, NULL, tmpbuflen,  P, sr, &hash_st) < 0){
  	    fprintf(stderr,"ERROR:  unable to get audio hash\n");
	    continue;
	}
	fprintf(stdout,"nbframes %u\n", nbframes);

	if (insert_into_audioindex(index_table, hash_id, phash, nbframes) < 0){
	    fprintf(stderr,"fatal error: unable to insert %u into hash\n", hash_id);
	}


	if (buf != sigbuf) ph_free(buf);
	ph_free(phash);

	free_mdata(&mdata);
    }

    int nbbkts, nbentries;
    stat_audioindex(index_table, &nbbkts, &nbentries);
    fprintf(stdout,"nb buckets %d\n",nbbkts);
    fprintf(stdout,"nb entries %d\n",nbentries);

    if (flush_audioindex(index_table, indexfile) < 0){
	fprintf(stdout,"error flushing index\n");
    }

    ph_hashst_free(&hash_st);

    free(sigbuf);
    for (i=0;i<nbfiles;i++){
	free(files[i]);
    }
    free(files);
    if (close_audioindex(index_table, 1) < 0){
	fprintf(stdout,"error closing audio index\n");
    }
    close_audiodata_db(mdatastore);

    return 0;
}

void print_audioindex_info(const char *idx_name){

    char indexfile[FILENAME_MAX];
    snprintf(indexfile, FILENAME_MAX, "%s.idx", idx_name);

    AudioIndex index_table = open_audioindex(indexfile, 1, 0);
    
    int nbbkts, nbentries;
    stat_audioindex(index_table, &nbbkts, &nbentries);
    
    double load = (double)nbbkts/(double)nbentries;
    fprintf(stdout,"buckets %d, entries %d, load %f\n", nbbkts,nbentries, load);

    close_audioindex(index_table, 1);
}

int queryaudioindex(const char *dir_name, const char *idx_name, const int sr,\
                    const int block_size, const float nbsecs, const float confidence_lvl,\
                    const unsigned int P){

    unsigned int nbfiles;
    char **files = readfilenames(dir_name, &nbfiles);
    if (files == NULL){
        files = (char**)malloc(sizeof(char*));
	files[0] = strdup(dir_name);
	nbfiles = 1;
    }
    fprintf(stdout,"nb files %d\n\n", nbfiles);

    char indexfile[FILENAME_MAX];
    snprintf(indexfile, FILENAME_MAX, "%s.idx", idx_name);

    AudioIndex audio_index = open_audioindex(indexfile, 0, 0);
    if (audio_index < 0){
      fprintf(stderr,"unable to open audio index\n");
      return -1;
    }
    void *ctx = zmq_init(1);
    if (ctx == NULL){
	fprintf(stderr, "unable to init zeromq\n");
	return -1;
    }

    AudioDataDB mdatastore = open_audiodata_db(ctx, GlobalArgs.server_addr);
    if (mdatastore < 0) {
      fprintf(stderr,"unable to open metadata store\n");
      return -1;
    }

    float *sigbuf = (float*)malloc(1<<25);
    if (sigbuf == NULL){
	fprintf(stdout,"mem alloc error\n");
	return -2;
    }
    const unsigned int buflen = (1<<25)/sizeof(float);

    char *inlinestr;
    AudioHashStInfo hash_st;
    hash_st.sr = 0;
    int i,j;
    for (i=0;i<nbfiles;i++){
	fprintf(stdout,"query[%3d] =  %s\n", i, files[i]);

	unsigned int tmpbuflen = buflen;
	int err;
	float *buf = readaudio(files[i], sr, sigbuf, &tmpbuflen, nbsecs, NULL, &err);
	if (buf == NULL){
	    fprintf(stdout, "could not get audio, err = %d\n", err);
	    continue;
	}

	uint32_t *phash=NULL;
	double **coeffs = NULL;
	uint8_t **toggles = NULL;
	unsigned int nbframes, nbcoeffs;
	double minbark,maxbark;

	fprintf(stdout,"calculating hash for query ... \n");

	if (audiohash(buf, &phash, &coeffs, &toggles, &nbcoeffs, &nbframes,\
		      &minbark, &maxbark, tmpbuflen, P, sr, &hash_st) < 0){
	    fprintf(stdout, "unable to get audio hash\n");
            continue;
	}
	fprintf(stdout,"number hash frames %d\n\n", nbframes);

	uint32_t result_id = 0;
	float cs = 0;

	fprintf(stdout,"do lookup ...\n");

	lookupaudiohash(audio_index, phash, toggles, nbframes,\
                        P, block_size, confidence_lvl, &result_id, &cs);

	fprintf(stdout, "found id %u\n", result_id);
        inlinestr = retrieve_audiodata(mdatastore, result_id);
	if (inlinestr == NULL){
	    fprintf(stderr, "unable to retrieve %u\n", result_id);
	}

	if (cs >= 0 && inlinestr){
	    fprintf(stdout,"FOUND ==> \"%s\" cs%f\n\n", inlinestr,cs);
	} else {
	    fprintf(stdout,"NONE FOUND\n\n");
	}

	if (buf != sigbuf) free(buf);
	for (j=0;j<nbframes;j++){
	    free(coeffs[j]);
	    if (toggles) free(toggles[j]);
	}
	if (toggles) free(toggles);
	free(coeffs);
	free(phash);
    }
    ph_hashst_free(&hash_st);
    close_audioindex(audio_index, 0);
    close_audiodata_db(mdatastore);

    return 0;
}

void display_usage(){
    fprintf(stdout,"usage: audio_index <command> <options> <index> <dir>\n\n");
    fprintf(stdout,"\n");
    fprintf(stdout,"commands:\n");
    fprintf(stdout,"help  or ?                               print usage information\n");
    fprintf(stdout,"build <index> <dir|file>                 build or add to index\n");
    fprintf(stdout,"stat  <index>                            print number bins and entries\n");
    fprintf(stdout,"query -p|t|n|b  <index> <dir|file>       query index for files in dir\n");
    fprintf(stdout,"\n");
    fprintf(stdout,"options:\n");
    fprintf(stdout,"  -s --dbserver <address>                address of metadata server\n");
    fprintf(stdout,"                                             e.g. \"tcp:localhost:4000\" \n");
    fprintf(stdout,"  -p --toggles <integer>                 number toggle bits\n");
    fprintf(stdout,"  -t --threshold <real>                  threshold in query\n");
    fprintf(stdout,"  -n --nbsecs <real>                     secs to hash from signal\n");
    fprintf(stdout,"  -b --blocksize <integer>               block size\n");
    fprintf(stdout,"\n\n\n");
}

void init_options(){
    GlobalArgs.cmd = NULL;
    GlobalArgs.dir_name = NULL;
    GlobalArgs.index_name = NULL;
    GlobalArgs.server_addr = NULL;
    GlobalArgs.dest_index = NULL;
    GlobalArgs.src_index = NULL;
    GlobalArgs.P = 0;
    GlobalArgs.sr = 6000;
    GlobalArgs.blocksize = 256;
    GlobalArgs.verbosity = 0;
    GlobalArgs.help = 0;
    GlobalArgs.nbsecs = 0.0f;
    GlobalArgs.threshold = 0.015;
}

void parse_options(int argc, char **argv){

    if (argc >= 2) {
	GlobalArgs.cmd = argv[1];
    } else {
	GlobalArgs.cmd = "none";
	GlobalArgs.help = 1;
	return;
    }
    
    int longIndex;
    char opt = getopt_long(argc-1, argv+1, opt_string, longOpts, &longIndex);
    while (opt != -1){
	switch (opt){
	case 'p':
	    GlobalArgs.P = atoi(optarg);
	    break;
	case 't':
	    GlobalArgs.threshold = atof(optarg);
	    break;
	case 'n':
	    GlobalArgs.nbsecs = atof(optarg);
	    break;
	case 'b':
	    GlobalArgs.blocksize = atoi(optarg);
	    break;
	case 'd':
	    GlobalArgs.dest_index = optarg;
	    break;
	case 's':
	    GlobalArgs.server_addr = optarg;
	    break;
	case 'v':
	    GlobalArgs.verbosity = 1;
	case 'h':
	case '?':
	    GlobalArgs.help = 1;
	    break;
        case 0:
	default:
	    break;
  
	}
	opt = getopt_long(argc-1, argv+1, opt_string, longOpts, &longIndex);
    }

    /* get remaining args */ 
    char **filenames = (char**)(argv + 1 + optind);
    if (!strcmp(GlobalArgs.cmd, "queryd")){
	GlobalArgs.index_name = filenames[0];
    } else {
	GlobalArgs.index_name = filenames[0];
	GlobalArgs.dir_name = filenames[1];
    } 

}

int main(int argc, char **argv){

  if (setlocale(LC_ALL,"") == NULL){
    fprintf(stderr,"locale unknown\n");
    return -1;
  }

    init_options();
    parse_options(argc, argv);

    if (GlobalArgs.help){
	display_usage();
    }
    if (GlobalArgs.cmd == NULL){
	display_usage();
	exit(1);
    }
    
    fprintf(stdout,"p = %d\n", GlobalArgs.P);
    fprintf(stdout,"t = %f\n", GlobalArgs.threshold);
    fprintf(stdout,"n = %f\n", GlobalArgs.nbsecs);

    if (!strcmp(GlobalArgs.cmd, "build")){
      if (GlobalArgs.dir_name == NULL || GlobalArgs.index_name == NULL){
	fprintf(stderr,"not enough input args\n");
	exit(1);
      }

	fprintf(stdout,"add files in %s dir to index %s\n",\
		GlobalArgs.dir_name, GlobalArgs.index_name);
	if (addtoaudioindex(GlobalArgs.dir_name,GlobalArgs.index_name,GlobalArgs.sr,\
			    GlobalArgs.nbsecs,GlobalArgs.P) < 0){
	    fprintf(stdout,"unable to complete command\n");
	}

    } else if (!strcmp(GlobalArgs.cmd, "combine")){

	fprintf(stdout,"not yet implemented\n");

    } else if (!strcmp(GlobalArgs.cmd, "stat")){
      if (GlobalArgs.index_name == NULL){
	fprintf(stderr,"no index name given\n");
	exit(1);
      }
	fprintf(stdout,"information on table");
	print_audioindex_info(GlobalArgs.index_name);

    } else if (!strcmp(GlobalArgs.cmd, "query")){
      if (GlobalArgs.dir_name == NULL || GlobalArgs.index_name == NULL){
	fprintf(stderr,"not enough inptu args\n");
	exit(1);
      }
	fprintf(stdout,"query files in %s against index %s\n",\
                GlobalArgs.dir_name, GlobalArgs.index_name);
	if (queryaudioindex(GlobalArgs.dir_name,GlobalArgs.index_name,GlobalArgs.sr,\
             GlobalArgs.blocksize, GlobalArgs.nbsecs,GlobalArgs.threshold,GlobalArgs.P) < 0){
	    fprintf(stdout,"unable to complete query\n");
	}


    } else if (!strcmp(GlobalArgs.cmd, "start")){

	fprintf(stdout," %s not yet implemented\n",GlobalArgs.cmd);

    } else if (!strcmp(GlobalArgs.cmd, "stop")){

	fprintf(stdout," %s not yet implemented\n", GlobalArgs.cmd);
    } else {
	fprintf(stdout," %s is unknown command\n", GlobalArgs.cmd);
	display_usage();
    }


    
    return 0;

}

#ifdef DMALLOC
#include "dmalloc.h"
#endif
