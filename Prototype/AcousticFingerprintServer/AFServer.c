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

	Modified by Soleo Shao 2011-4-13
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <stdarg.h>
#include <getopt.h>
#include <syslog.h>
#include <time.h>
#include <zmq.h>
#include <assert.h>
#include "phash_audio.h"
#include "serialize.h"
#include "zmqhelper.h"



#define WORKING_DIR "/tmp"
#define LOG_IDENT "AFServer"
#define LOCKFILE "AFServer.lock"
#define MAX_INDEX_FILE_SIZE 65
#define NB_BUCKETS_TABLE_SIZE (1<<25)
#define NB_BUCKETS_TMP_TABLE_SIZE (1<<20)

static const char *opt_string = "w:l:s:i:p:b:t:n:vh?";
static const char *init_str = "INIT";
static const char *kill_str = "KILL";
const int g_debug = 1;
static int term_sig = 0;
static uint32_t g_lastid = 0;
#define LOG(log) \
	if(g_debug) { \
		printf(log); \
		printf("\n"); \
	}
static uint32_t g_process_amount = 0;
static const struct option longOpts[] = {
    { "wd", required_argument, NULL, 'w'          },
    { "level", required_argument, NULL, 'l'       },
    { "index", required_argument, NULL, 'i'       },
    { "server", required_argument, NULL, 's'      },
    { "port", required_argument, NULL, 'p'        },
    { "blocksize", required_argument, NULL, 'b'   },
    { "threshold", required_argument, NULL, 't'   },
    { "threads", required_argument, NULL, 'n'     },
    { "verbose", no_argument, NULL, 'v'           },
    { "help", no_argument, NULL, 'h'              },
    { NULL, no_argument, NULL, 0                  }
};


struct globalargs_t {
    char *wd;              /* working directory          */
    char *server_address;  /* name of server address     */
    char *index_name;
    int level;
    int port;              /* starting port range on server */
    int blocksize;         /* block size for lookup on hash */
    float threshold;       /* threshold for look up comparisons */
    int nbthreads;         /* number worker threads to service incoming queries */
    int verboseflag;
    int helpflag;
} GlobalArgs;

void init_options(){
    GlobalArgs.wd = NULL;
    GlobalArgs.level = LOG_UPTO(LOG_ERR);
    GlobalArgs.index_name = NULL;
    GlobalArgs.port = 4005;
    GlobalArgs.blocksize = 256;
    GlobalArgs.threshold = 0.015;
    GlobalArgs.nbthreads = 60;
    GlobalArgs.verboseflag = 0;
    GlobalArgs.helpflag = 0;
}

void parse_options(int argc, char **argv)
{
    int longIndex;
    char opt = getopt_long(argc, argv,opt_string, longOpts, &longIndex);
    while (opt != -1)
	{
		switch(opt)
		{
			case 'w':
				GlobalArgs.wd = optarg;
				break;
			case 'l':
				GlobalArgs.level = LOG_UPTO(atoi(optarg));
				break;
			case 'i':
				GlobalArgs.index_name = optarg;
				break;
			case 'p':
				GlobalArgs.port = atoi(optarg);
				break;
			case 'b':
				GlobalArgs.blocksize = atoi(optarg);
				break;
			case 't':
				GlobalArgs.threshold = atof(optarg);
				break;
			case 'v':
				GlobalArgs.verboseflag = 1;
				break;
			case 'n':
				GlobalArgs.nbthreads = atoi(optarg);
				break;
			case 'h' :
				GlobalArgs.helpflag = 1;
				break;
			case '?':
				GlobalArgs.helpflag = 1;
				break;
			default:
				break;
		}
		opt = getopt_long(argc, argv, opt_string, longOpts, &longIndex);
    }
}

void AFServer_usage()
{
    fprintf(stdout,"afserverd [options]\n\n");
    fprintf(stdout,"options:\n");
    fprintf(stdout," -p <portnumber>         start of port range  - e.g. 5000");
    fprintf(stdout,"                         mandatory\n");
    fprintf(stdout," -w <working dir>        working dir to run tableserver default \"tmp\"\n"); 
    fprintf(stdout," -l <log level>          log level 0 and 1  to 7(0,LOG_EMERG...LOG_DEBUG\n");
    fprintf(stdout,"                         log levels correspond to those in syslog.h\n");
    fprintf(stdout," -b <block size>         blocksize for performing lookup,  default 256\n");
    fprintf(stdout," -t <threshold>          threshold for performing lookup, default 0.015\n");
    fprintf(stdout," -n <threads>            number of worker threads, default is 60\n");
    fprintf(stdout," -i <index name>         path and name of index file - mandatory\n");
}  

typedef struct phashbox{
    uint8_t   cmd;         // 1 for query, 2 for submission
    uint8_t   earlyendflag;
    uint8_t   amount;      // amount of phash times
    uint8_t   id;          // order of phash
    uint32_t  nbframes;    // length of phash 
    uint32_t* phash;
} phashbox_t;

enum {
    ONLYPHASH      = 0x01 << 0,   // 0000 0001
    PHASHANDSPHASH = 0x01 << 1,   // 0000 0010
    LOOKUP         = 0x01 << 2,   // 0000 0100
    INSERT         = 0x01 << 3,   // 0000 1000
    NOCALCHASH     = 0x01 << 4    // 0001 0000
  };

static uint8_t table_number = 0;

static char indexfile[FILENAME_MAX];
static char tmpindexfile[FILENAME_MAX];

/* Number currently accessing the index. */
/* Access this variable through its associated mutex. */
/* Do not access directly, but through the wait_for/post functions. */
/* When changing the value, signal on associated condition */
static int nb_index_access = 0;
static pthread_mutex_t  access_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t   access_cond = PTHREAD_COND_INITIALIZER;
 
/* control access to the main index. */
/* use wait_for() and post() around access to the main index */
/* this allows a set nb of lookups to occur in parallel */
static AudioIndex audioindex = NULL;


/* call when thread is done accessing index */
void post_main_index()
{
    pthread_mutex_lock(&access_mutex);
    nb_index_access--;
    pthread_cond_signal(&access_cond);
    pthread_mutex_unlock(&access_mutex);
}

/* call when thread about to access index */
/* waits until the value of 'nb_index_access' variable is below the parameter, nb */
void waitfor_main_index(int nb)
{
  pthread_mutex_lock(&access_mutex);
  while (nb_index_access > nb) 
    pthread_cond_wait(&access_cond, &access_mutex);
  nb_index_access++;
  pthread_mutex_unlock(&access_mutex);
}


/* control access to tmp index */
/* wrap all access to tmp index with waitfor_index()/pos_index() */
/* native semaphore implementation to allow unlimited access, but the */
/* ability to unplug the index during updates by waiting for the count to go to zero */
static int nb_tmp_index_access = 0;
static pthread_mutex_t tmpaccess_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  tmpaccess_cond  = PTHREAD_COND_INITIALIZER;

/* tmp index for new additions */
/* sandwich all access with waitfor_tmp()/post_tmp() function calls*/
static AudioIndex audioindex_tmp = NULL;

/* control access to tmp index */
/* with post_tmp_index()/waitfor_tmp_index() */
void post_tmp_index()
{
    pthread_mutex_lock(&tmpaccess_mutex);
    nb_tmp_index_access--;
    pthread_cond_signal(&tmpaccess_cond);
    pthread_mutex_unlock(&tmpaccess_mutex);
}

uint32_t GeneratePosid()
{
  return ++g_lastid;
}

void waitfor_tmp_index(int nb, int flag, uint32_t* id)
{
  pthread_mutex_lock(&tmpaccess_mutex);
  while (nb_tmp_index_access > nb)
    pthread_cond_wait(&tmpaccess_cond, &tmpaccess_mutex);
  nb_tmp_index_access++;
  if (flag)
  {  
	  *id = GeneratePosid();
	  g_process_amount++;
  }
  pthread_mutex_unlock(&tmpaccess_mutex);
}


/* DO NOT USE  - only saved here in order to shut down zmq messaging properly on term signal */
static void *main_ctx = NULL;
static void *main_clients = NULL;
static void *main_workers = NULL;
void init_process();
void kill_process();
int init_server();
int kill_server();
int init_index();
int kill_index();

void handle_signal(int sig)
{
    switch (sig)
	{
		case SIGUSR1:
		case SIGUSR2:
		 LOG("SIGUSR signal received");
		 syslog(LOG_DEBUG, "recieved SIGUSR %d", sig);
		  if (kill_server() < 0) 
		  { 
			syslog(LOG_CRIT,"SIGHANDLER: unable to kill server");
			break;
		  }
		  if (kill_index() < 0)
		  { 
			syslog(LOG_CRIT,"SIGHANDLER: unable to kill index");
			break;
		  }
		  if (init_index() < 0)
		  {
			syslog(LOG_CRIT,"SIGHANDLER: unable to init index - index STILL down");
			break;
		  }
    	  if (init_server(main_ctx, main_clients, main_workers) < 0)
		  {
			syslog(LOG_CRIT,"SIGHANDLER, unable to init server - index up but unknown by main server");
			break;
		  }
		  break;
		case SIGINT:
		case SIGHUP:
		case SIGTERM:
			LOG("Recieved SIGTERM");
			term_sig = 1;
			//sleep(2);
			syslog(LOG_DEBUG, "recieved SIGTERM %d", sig);
			if (kill_server() < 0) 
			{
				syslog(LOG_CRIT,"SIGHANDLER: unable to kill server");
				LOG("SIGHANDLER: unable to kill server");
			}

			if (kill_index() < 0) 
			{	
				syslog(LOG_CRIT,"SIGHANDLER: unable to kill index");
				LOG("SIGHANDLER: unable to kill index");
			}

			//kill_process();
      zmq_close(main_clients);
      zmq_close(main_workers);
			zmq_term(main_ctx);
			exit(0);
      break;
    }
}

void WriteLastId2File()
{
  FILE* fp;
  char log[80];
  fp = fopen("./POSID.cfg", "w");
  fseek(fp, 0, SEEK_SET);
  fprintf(fp, "%d\n", g_lastid);
  sprintf(log, "[write]last posid: %u", g_lastid);
  LOG(log);
  fclose(fp);
}

void ReadLastIdFromFile(uint32_t* posid)
{
  FILE* fp;
  char log[80];
  fp = fopen("./POSID.cfg", "r");
  if (!fp)
  {
    LOG("Can not find or read POSID.cfg");
    exit(1);
  }
  fseek(fp, 0, SEEK_SET);
  fscanf(fp, "%d", posid);
  sprintf(log, "[Read]last posid: %u", *posid);
  LOG(log);
  fclose(fp);
}



int init_index()
{
    // init tmp index for new submissions 
    indexfile[0] = '\0';
    tmpindexfile[0] = '\0';
    snprintf(indexfile, FILENAME_MAX, "%s.idx", GlobalArgs.index_name);
    snprintf(tmpindexfile, FILENAME_MAX, "%s.tmp", GlobalArgs.index_name);
    syslog(LOG_DEBUG,"init index");
	  LOG("init index");
    
    //Get last ID
    ReadLastIdFromFile(&g_lastid);

	  int err;
    struct stat tmpidx_info, idx_info;
  //  if (!stat(tmpindexfile, &tmpidx_info) && !stat(indexfile, &idx_info))
	  {
	// if size if greater enough, merge temp into permanent index 
    //  syslog(LOG_DEBUG,"merge %s into %s", tmpindexfile, indexfile);
	//	  LOG("Can not merge");
		//  err = merge_audioindex(indexfile, tmpindexfile);
	//	  if (err < 0)
	//	    syslog(LOG_ERR, "unable to merge %s into %s", tmpindexfile, indexfile);
	//    else if (err > 0)
	//		  syslog(LOG_DEBUG,"no need to merge %s into %s", tmpindexfile, indexfile);
    } 

   // syslog(LOG_DEBUG,"open index, %s", indexfile);


    pthread_mutex_lock(&access_mutex);
    audioindex = open_audioindex(indexfile, 0, NB_BUCKETS_TABLE_SIZE);
    if (audioindex) 
		pthread_cond_signal(&access_cond);
	  pthread_mutex_unlock(&access_mutex);
    if (audioindex == NULL)
	  {
		  LOG("unable to open index audioindex");
		  syslog(LOG_CRIT, "unable to open index, %s", indexfile);
		  return -1;
    }
   
    syslog(LOG_DEBUG, "open tmp index, %s", tmpindexfile);

    pthread_mutex_lock(&tmpaccess_mutex);
    audioindex_tmp = open_audioindex(tmpindexfile, 1, NB_BUCKETS_TMP_TABLE_SIZE);
    if (audioindex_tmp) pthread_cond_signal(&tmpaccess_cond);
    pthread_mutex_unlock(&tmpaccess_mutex);
    if (audioindex_tmp == NULL)
	  {	
		  LOG("unable to open index audioindex_tmp");
		  syslog(LOG_CRIT, "unable to open index, %s", tmpindexfile);
		  return -1;
    } 

    return 0;
}

int kill_index()
{
    int err = 0;
    // write last id to file
    WriteLastId2File();

    syslog(LOG_DEBUG,"KILLINDEX: close index");
	  LOG("KILLINDEX: close index");
    /* wait for number accessing the index to be 0 */
    waitfor_main_index(0);
    err = close_audioindex(audioindex, 0);
    audioindex = NULL;
    post_main_index();

    if (err < 0)
	  {
      syslog(LOG_ERR,"KILLINDEX: unable to close index");
	    LOG("KILLINDEX: unable to close index");
      return -1;
    }
	
	  LOG("KILLINDEX: close tmp audioindex");
    syslog(LOG_DEBUG,"KILLINDEX: close tmp audioindex");

    /* wait for tmp index to close and NULL it */
    waitfor_tmp_index(0, 0, NULL);
	  LOG("flush tmp index file");
    err = flush_audioindex(audioindex_tmp, tmpindexfile);
	  LOG("close tmp index file");
    err = close_audioindex(audioindex_tmp, 1);
    audioindex_tmp = NULL;
	  LOG("post tmp file");
    post_tmp_index();
	  LOG("finish posting");
    if (err < 0)
	  {
	    LOG("KILLINDEX: unable to close tmp index");
      syslog(LOG_ERR,"KILLINDEX: unable to close tmp index, err = %d", err);
      err = -2;
    }
	printf("g_process_amount = %u\n", g_process_amount);
    return err;
}

void init_process()
{
    int fv, i;

    if (getpid() == 1) return; 

    fv = fork();
    if (fv < 0)
	  {
		  fprintf(stderr,"cannot fork\n"); 
		  exit(1);
    }
    if (fv > 0) exit(0);

    /* daemon continues */
    pid_t sid = setsid();
    if (sid < 0)
    {
		  fprintf(stderr, "Can not Create a new SID for the child process\n");
		  exit(1);
    }

    /* close all file descrs */ 
    for (i=getdtablesize();i >= 0; --i) close(i);

    /* redirect stdin, stdout, stderr */ 
    i = open("/dev/null", O_RDWR);
    if (i < 0) exit(1);
    dup(i);
    dup(i);
    
    umask(0);

    chdir(GlobalArgs.wd);
    
	// Setup syslog
    openlog(LOG_IDENT, LOG_PID, LOG_USER);
    setlogmask(GlobalArgs.level);

    /*first instance only */
    signal(SIGCHLD, SIG_IGN); /* ignore child */ 
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */ 
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP,  SIG_IGN);
    signal(SIGTERM, handle_signal);
    signal(SIGUSR1, handle_signal);
    signal(SIGUSR2, handle_signal);
    signal(SIGINT, handle_signal);
    syslog(LOG_DEBUG, "INITPROCESS: init completed");
}

int init_server(void* ctx, void** clients, void** workers)
{
    char addr[32];
    syslog(LOG_DEBUG, "INIT SERVER");
    
    *clients = zmq_socket(ctx, ZMQ_XREP);
	  zmq_bind(clients, addr);
	  if (!clients)
	  {	
		  LOG("INITSERVER: unable to bind clients");
		  syslog(LOG_CRIT,"INITSERVER: unable to bind clients");
		  return -1;
    }

    syslog(LOG_DEBUG, "INIT SERVER :bind clients");
	  LOG("INIT SERVER: bind clients");
		uint8_t tn= 0;
    /* set static global variable */
    table_number = tn;

    syslog(LOG_DEBUG, "INITSERVER: table number %u, init complete", tn);
	  LOG("init server complete");
    return 0;
}

// kill server
int kill_server()
{
    char addr[32];
    void *ctx = main_ctx;
	  void *clients = main_clients;
	  void *workers = main_workers;

// 	  zmq_close(clients);
// 	  zmq_close(workers);
	  //zmq_term(ctx);
	  LOG("KILLSERVER: server killed");
    syslog(LOG_DEBUG,"KILLSERVER: server killed");

    return 0;
}

void kill_process()
{
    closelog();
}


// aux function to worker threads for sending the results
static int send_results(void *receiver, uint8_t threadnb, uint32_t posid, float cs, uint8_t cmd)
{
	char log[100];

  syslog(LOG_DEBUG,"SEND: send thr = %u, id = %u, cs = %f", threadnb, posid, cs);
	
	if (cmd == INSERT)
	{
		sprintf(log, "SEND insert result: send thr = %u, posid = %u", threadnb, posid);
		LOG(log);
		send_msg_vsm(receiver, &posid, sizeof(uint32_t));

	}
	else if (cmd == LOOKUP)
  {
		sprintf(log, "SEND lookup result: send thr = %u, posid = %u, cs = %f", threadnb, posid, cs);
		LOG(log);
		sendmore_msg_vsm(receiver, &posid, sizeof(uint32_t));
		send_msg_vsm(receiver, &cs, sizeof(float));
	}

    return 0;
}

// assign int value to worker thread for logging purposes
static int thread_count = 0;


// aux function to worker thread to execute commands 
static int execute_command(uint8_t thrn, uint8_t cmd, uint32_t* hash, uint32_t nbframes, uint32_t *posid, float *cs)
{
  int err = 0;
  uint8_t table_n;
  *cs = -1.00f;
	char log[80];
  switch (cmd)
	{
		case 3:  // This is reserved for lookup in Main Index File
		if (audioindex)
		{
			
			waitfor_main_index(GlobalArgs.nbthreads+1);
			sprintf(log, "WORKER%d: do lookup for hash[%d]", thrn, nbframes);
			LOG(log);
			syslog(LOG_DEBUG,"WORKER%d: do lookup for hash[%d]", thrn, nbframes);
			err = lookupaudiohash(audioindex, (uint32_t*)hash, NULL, nbframes, 0, \
					              GlobalArgs.blocksize, GlobalArgs.threshold, posid, cs);
			post_main_index();

			if (err < 0)
			{
				sprintf(log, "WORKER%d: could not do lookup - err %d", thrn, err);
				LOG(log);
				syslog(LOG_ERR,"WORKER%d: could not do lookup - err %d", thrn, err);
				err = -1;
			}
		} 
		else
		{
			sprintf(log, "WORKER%d: index is down, unable to do lookup", thrn);
			LOG(log);
			syslog(LOG_DEBUG,"WORKER%d: index is down, unable to do lookup", thrn);
			err = -2;
		}
		case LOOKUP:
		if (audioindex)
		{
			
			waitfor_tmp_index(GlobalArgs.nbthreads+1, 0, NULL);
			while (audioindex_tmp == NULL)
				pthread_cond_wait(&tmpaccess_cond, &tmpaccess_mutex);
			sprintf(log, "WORKER%d: do lookup for hash[%d]", thrn, nbframes);
			LOG(log);
			syslog(LOG_DEBUG,"WORKER%d: do lookup for hash[%d]", thrn, nbframes);
			err = lookupaudiohash(audioindex_tmp, (uint32_t*)hash, NULL, nbframes, 0, \
					              GlobalArgs.blocksize, GlobalArgs.threshold, posid, cs);
			post_tmp_index();

			if (err < 0)
			{
				sprintf(log, "WORKER%d: could not do lookup - err %d", thrn, err);
				LOG(log);
				syslog(LOG_ERR,"WORKER%d: could not do lookup - err %d", thrn, err);
				err = -1;
			}
		} 
		else
		{
			sprintf(log, "WORKER%d: index is down, unable to do lookup", thrn);
			LOG(log);
			syslog(LOG_DEBUG,"WORKER%d: index is down, unable to do lookup", thrn);
			err = -2;
		}
		break;
		case INSERT:
		{ /* if meant for this table */
			
			uint32_t id = 0;
			waitfor_tmp_index(GlobalArgs.nbthreads+1, 1, &id);
			*posid = id;
			while (audioindex_tmp == NULL)
				pthread_cond_wait(&tmpaccess_cond, &tmpaccess_mutex);
			sprintf(log, "WORKER%d: inserting id = %u, hash[%d], g_lastid =%u, timer = %d", thrn, *posid, nbframes, g_lastid, time(NULL));
			LOG(log);
			
			syslog(LOG_DEBUG,"WORKER%d: inserting id = %d, hash[%d]", thrn, *posid, nbframes);
			err = insert_into_audioindex(audioindex_tmp, *posid, (uint32_t*)hash, nbframes);
			post_tmp_index();
			if (err < 0)
			{
				sprintf(log, "WORKER%d: unable to insert hash - err %d", thrn, err);
				LOG(log);
				syslog(LOG_ERR,"WORKER%d: unable to insert hash - err %d", thrn, err);
				err = -3;
			}
		}
		break;
		default:
			sprintf(log, "WORKER%d: cmd not recognized, %u", thrn, cmd);
			LOG(log);
			syslog(LOG_DEBUG, "WORKER%d: cmd not recognized, %u", thrn, cmd);
			err = -4;
	}

  return err;
}
#define RECEIVEMSG(SIZE,STORE,ERROR) \
recieve_msg(receiver, &msg_size, &more, &more_size, &data); \
			if (msg_size != (SIZE) || !more) \
			{ \
				printf("size not fit msg_size:%d, size:%d\n",msg_size,SIZE); \
				if (more) flushall_msg_parts(receiver); \
				free(data); \
				data = NULL; \
				send_empty_msg(receiver); \
				return ERROR; \
			} \
				memcpy(STORE, data, (SIZE)); \
				free(data); \
				data = NULL;

enum errorcodes{
	UNKNOWNERROR = -100 ,
	MISSINGCMD          ,
	NOSPHASH            ,
  MISSINGEARLYENDFLAG ,
	MISSINGAMOUNT       ,
	MISSINGID           ,
	MISSINGNBFRAMES     ,
	MISSINGPHASH        ,
	ENDEARLY
};

// aux message to worker thread to a message
static int pull_message(int thrn, void *receiver)
{
	if (term_sig == 1)
		   return -1;

	char log[512];
	sprintf(log, "***************\n[pull_message]...WORKER[%d]", thrn);
	LOG(log);
    phashbox_t m_phashframe;
	
	// init a frame
	m_phashframe.cmd = 0;
	m_phashframe.earlyendflag = 0;
	m_phashframe.amount = 0;
	m_phashframe.id = 0;
	m_phashframe.nbframes = 0;
	m_phashframe.phash = NULL;
	
  //return 0;
	// init other vals
  uint32_t nbframes, posid = 0;
  void* data = NULL;
  int i, err = 0;
  int64_t more;
  size_t msg_size, more_size = sizeof(int64_t);
  float cs = -1.0f;
    
    // pull cmd msg part
  err = RECEIVEMSG(sizeof(uint8_t),&m_phashframe.cmd,MISSINGCMD);
	sprintf(log, "RECEIVEMSG: WORKER%d m_phashframe.cmd:%d", thrn, m_phashframe.cmd);
	LOG(log);
  syslog(LOG_DEBUG,"WORKER%d: m_phashframe.cmd", thrn);

	// pull sphash msg part, simply ignore it now.
	if(m_phashframe.cmd == INSERT)
  {
		int err = recieve_msg(receiver, &msg_size, &more, &more_size, &data); 
		char* sphash = (char*)malloc(msg_size*sizeof(uint8_t)+1);
		memcpy(sphash, data, msg_size); 
		sphash[msg_size] = '\0';
		free(data); 
		data = NULL;
		sprintf(log, "RECEIVEMSG: WORKER%d sphash:%s, msg_size:%d", thrn, sphash, msg_size);
		LOG(log);
		free(sphash);
		sphash = NULL;
	  syslog(LOG_DEBUG,"WORKER%d: sphash", thrn);
	}

	// pull early end flag msg part
	err = RECEIVEMSG(sizeof(uint8_t),&m_phashframe.earlyendflag,MISSINGEARLYENDFLAG);
	syslog(LOG_DEBUG,"WORKER%d: m_phashframe.earlyendflag", thrn);
	sprintf(log, "RECEIVEMSG: WORKER%d m_phashframe.earlyendflag:%d", thrn, m_phashframe.earlyendflag);
	LOG(log);
  if (m_phashframe.earlyendflag == 1)
  {
		syslog(LOG_DEBUG,"WORKER%d: early end flag %d", thrn, m_phashframe.earlyendflag);
		if (more) flushall_msg_parts(receiver);
		send_empty_msg(receiver);
		return ENDEARLY;
  }

	// pull amount msg part
	err = RECEIVEMSG(sizeof(uint8_t),&m_phashframe.amount,MISSINGAMOUNT);
	syslog(LOG_DEBUG,"WORKER%d: m_phashframe.amount", thrn);
	sprintf(log, "RECEIVEMSG: WORKER%d m_phashframe.amount:%d", thrn, m_phashframe.amount);
	LOG(log);
	// pull id msg part
	err = RECEIVEMSG(sizeof(uint8_t),&m_phashframe.id,MISSINGID);
	syslog(LOG_DEBUG,"WORKER%d: m_phashframe.id", thrn);
	sprintf(log, "RECEIVEMSG: WORKER%d m_phashframe.id:%d",thrn, m_phashframe.id);
	LOG(log);
	// pull nbframes msg part 
	err = RECEIVEMSG(sizeof(uint32_t),&m_phashframe.nbframes,MISSINGNBFRAMES);
  m_phashframe.nbframes = nettohost32(m_phashframe.nbframes);
	syslog(LOG_DEBUG,"WORKER%d: m_phashframe.nbframes", thrn);
	sprintf(log, "RECEIVEMSG: WORKER%d m_phashframe.nbframes:%d",thrn, m_phashframe.nbframes);
	LOG(log);
	// pull hash msg part 
  err = recieve_msg(receiver, &msg_size, &more, &more_size, (void **)&m_phashframe.phash);
  if (msg_size != m_phashframe.nbframes*sizeof(uint32_t) || more)
	{
		sprintf(log, "WORKER%d: inconsistent hash msg part size = %d", thrn, msg_size);
		LOG(log);
		syslog(LOG_DEBUG,"WORKER%d: inconsistent hash msg part size = %d", thrn, msg_size);
		if (more) flushall_msg_parts(receiver);
		free(m_phashframe.phash);
		m_phashframe.phash = NULL;
		return MISSINGPHASH;
  }
	sprintf(log, "RECEIVEMSG: WORKER%d m_phashframe.phash[0]:%X", thrn, m_phashframe.phash[0]);
	LOG(log);

	// de-serialization
  for (i = 0;i < m_phashframe.nbframes; i++)
		((uint32_t*)m_phashframe.phash)[i] = nettohost32(((uint32_t*)m_phashframe.phash)[i]);
	
	if (m_phashframe.cmd != 0)
	{
		// use command
		err = execute_command(thrn, m_phashframe.cmd, m_phashframe.phash, m_phashframe.nbframes, &posid, &cs);
		if (err < 0)
		{
			sprintf(log, "WORKER%d: unable to execute command, err=%d", thrn, err);
			LOG(log);
			syslog(LOG_DEBUG,"WORKER%d: unable to execute command, err=%d", thrn, err);
		}
		free(m_phashframe.phash);
		m_phashframe.phash = NULL;

		if (cs >= GlobalArgs.threshold)
		{
			sprintf(log, "WORKER%d: %f cs, %u posid", thrn, cs, posid);
			LOG(log);
			syslog(LOG_DEBUG,"WORKER%d: %f cs, %u posid", thrn, cs, posid);
			posid = hosttonet32(posid);
			cs = hosttonetf(cs);	
		}
		send_results(receiver, thrn, posid, cs, m_phashframe.cmd);
		sprintf(log, "WORKER%d:  %f cs, %u posid", thrn, cs, posid);
		LOG(log);
	}

  return 0;
}

/* worker thread code */
void* dowork(void *arg)
{
  char addr[32];
  int thr_n  = thread_count++;/* for logging */
  void *ctx = arg;
  void *receiver = zmq_socket(ctx, ZMQ_REP);
	zmq_connect(receiver, "inproc://workers");

    /* do not respond to SIGUSR1 or SIGUSR2 */
  sigset_t set;
  sigemptyset(&set);
  sigaddset(&set, SIGUSR1);
  sigaddset(&set, SIGUSR2);
  sigaddset(&set, SIGINT);

	char log[80];
	sprintf(log, "WORKER%d: me", thr_n);
	LOG(log);
  if (pthread_sigmask(SIG_UNBLOCK, &set, NULL))
	{
	  LOG("unable to set sigmask");
    syslog(LOG_CRIT,"WORKER%d: unable to set sigmask", thr_n);
	}
	int err;
  while (1)
	{

		err = pull_message(thr_n, receiver);
		if (err == -1)
		{
      zmq_close(receiver);
			LOG("term sig");
			return NULL;
		}
		sleep(1);
  }

	return NULL;
}

typedef struct dev_param_t {
  void *c;            /* from */
  void *w;            /* to   */
}DevParam;

void* doQ(void* p)
{
  DevParam* arg = (DevParam*)p;

  // Connect work threads to client threads via a queue
  zmq_device(ZMQ_QUEUE, arg->c, arg->w);
  LOG("zmq_device exit");
  return NULL;
}

// Workers loop function
void workers_thread(void* ctx, void* clients, void* workers)
{
	int i;
	
  for (i = 0; i < GlobalArgs.nbthreads; i++)
	{
		char log[80];
		snprintf(log, 80, "running thread %d", i);
		LOG(log);
		pthread_t worker_thr;
		if (pthread_create(&worker_thr, NULL, dowork, ctx))
		{	
			sprintf(log,"Workers: unable to create worker thread, %d", i);
			LOG(log);
			syslog(LOG_CRIT,"Workers: unable to create worker thread, %d", i);
			return;
		}
  }
  zmq_device(ZMQ_QUEUE, clients, workers);
//  return;
/*
  pthread_t q;
  DevParam dp_queue;
  dp_queue.c = clients;
  dp_queue.w = workers;

  if (pthread_create(&q, NULL, doQ, &dp_queue))
  {
    LOG("Create Q thread error");
  }

  sleep(3);
*/
}

int main(int argc, char **argv)
{
    init_options();
    parse_options(argc, argv);
   
	/*first instance only */
    signal(SIGCHLD, SIG_IGN); /* ignore child */ 
    signal(SIGTSTP, SIG_IGN); /* ignore tty signals */ 
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGHUP,  SIG_IGN);
    signal(SIGTERM, handle_signal);
    signal(SIGUSR1, handle_signal);
    signal(SIGUSR2, handle_signal);
    signal(SIGINT, handle_signal);
    syslog(LOG_DEBUG, "INITPROCESS: init completed");

    if (GlobalArgs.helpflag || !GlobalArgs.index_name )
	  {
		AFServer_usage();
		return 0;
    }

    /* init daemon */ 
    //init_process();
	  LOG("[Main]Coming...");

    if (init_index() < 0)
	  {
		  LOG("MAIN ERR: unable to init index");
		  syslog(LOG_CRIT,"MAIN ERR: unable to init index");
		  exit(1);
    }
    
    void *ctx = zmq_init(1);
    if (!ctx)
	  {
		  LOG("MAIN ERR: unable to init zmq ctx");
		  syslog(LOG_CRIT,"MAIN ERR: unable to init zmq ctx");
		  exit(1);
    }

	  LOG("INIT SERVER");
    syslog(LOG_DEBUG, "INIT SERVER");

    // save to global variable to be used in signal handler
    main_ctx = ctx;
	  void* clients = NULL;
	  void* workers = NULL;
	
	  char addr[32];
    syslog(LOG_DEBUG, "INIT SERVER");

	  snprintf(addr, 32, "tcp://*:%d", GlobalArgs.port);
    clients = zmq_socket(ctx, ZMQ_XREP);
	  zmq_bind(clients, addr);
	  if (!clients)
	  {	
		  LOG("INITSERVER: unable to bind clients");
		  syslog(LOG_CRIT,"INITSERVER: unable to bind clients");
		  exit(1);
    }

    syslog(LOG_DEBUG, "INIT SERVER :bind clients");
	  LOG("INIT SERVER: bind clients");
	  workers = zmq_socket(ctx, ZMQ_XREQ);
	  zmq_bind(workers, "inproc://workers");
    if (!workers)
	  {	
		  LOG("INITSERVER: unable to bind workers");
		  syslog(LOG_CRIT,"INITSERVER: unable to bind workers");
		  exit(1);
    }
    syslog(LOG_DEBUG, "INIT SERVER: bind workers");
	  LOG("INIT SERVER: bind workers");
	  LOG("init server complete");
/*
    if (init_server(ctx, &clients, &workers) < 0)
	{	
		LOG("MAIN ERR: unable to init server");
		syslog(LOG_CRIT,"MAIN ERR: unable to init server");
		exit(1);
    }
*/
    main_clients = clients;
	  main_workers = workers;
	
	
	// create workers threads to deal with clients
	  LOG("Create workers threads to deal with clients");
	  workers_thread(ctx, clients, workers);
    
//     while (1 && !term_sig)
//     {
//       sleep(1);
//       LOG("main sleep 1");
//     }
    
    LOG("exit");
    zmq_close(clients);
	  zmq_close(workers);
    zmq_term(ctx);
    return 0;
}
