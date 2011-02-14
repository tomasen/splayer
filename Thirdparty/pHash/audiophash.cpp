/*

    pHash, the open source perceptual hash library
    Copyright (C) 2009 Aetilius, Inc.
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Evan Klinger - eklinger@phash.org
    David Starkweather - dstarkweather@phash.org

*/
#include "audiophash.h"
#include "sndfile.h"
#include "samplerate.h"
#include "ph_fft.c"

//#define M_PI       3.14159265358979323846
#ifdef HAVE_LIBMPG123
#include <mpg123.h>
#endif

int ph_count_samples(const char *filename, int sr,int channels){

    SF_INFO sf_info;
    sf_info.format=0;
    SNDFILE *sndfile = sf_open(filename, SFM_READ, &sf_info);
    if (sndfile == NULL){
	return NULL;
    }
    int count = sf_info.frames;
    sf_close(sndfile);
    return count;
}
void sphash_freemem(float* buf, uint32_t* hash)
{
    free(hash);
    free(buf);
}

void sphash_freemem2(double* cs)
{
   delete cs;
}

#ifdef HAVE_LIBMPG123

static
float* readaudio_mp3(const char *filename,long *sr, const float nbsecs, unsigned int *buflen){
  mpg123_handle *m;
  int ret;

  if (mpg123_init() != MPG123_OK || ((m = mpg123_new(NULL,&ret)) == NULL)|| \
                         mpg123_open(m, filename) != MPG123_OK){
    fprintf(stderr,"unable to init mpg\n");
    return NULL;
  }

  /*turn off logging */
  mpg123_param(m, MPG123_ADD_FLAGS, MPG123_QUIET, 0);

  off_t totalsamples;

  mpg123_scan(m);
  totalsamples = mpg123_length(m);

  int meta = mpg123_meta_check(m);

  int channels, encoding;

  if (mpg123_getformat(m, sr, &channels, &encoding) != MPG123_OK){
    fprintf(stderr,"unable to get format\n");
    return NULL;
  }

  mpg123_format_none(m);
  mpg123_format(m, *sr, channels, encoding);

  size_t decbuflen = mpg123_outblock(m);
  unsigned char *decbuf = (unsigned char*)malloc(decbuflen);
  if (decbuf == NULL){
    printf("mem alloc error\n");
    return NULL;
  }

  unsigned int nbsamples = (nbsecs <= 0) ? totalsamples : nbsecs*(*sr);
  nbsamples = (nbsamples < totalsamples) ? nbsamples : totalsamples;

  size_t i, j, index = 0, done;


  float *buffer = (float*)malloc(nbsamples*sizeof(float));
  *buflen = nbsamples;

  do {

    ret = mpg123_read(m, decbuf, decbuflen, &done);
    switch (encoding) {
    case MPG123_ENC_SIGNED_16 :
      for (i = 0; i < done/sizeof(short); i+=channels){
	buffer[index] = 0.0f;
	for (j = 0; j < channels ; j++){
	  buffer[index] += (float)(((short*)decbuf)[i+j])/(float)SHRT_MAX;
	}
	buffer[index++] /= channels;
	if (index >= nbsamples) break;
      }
      break;
    case MPG123_ENC_SIGNED_8:
      for (i = 0; i < done/sizeof(char); i+=channels){
	buffer[index] = 0.0f;
	for (j = 0; j < channels ; j++){
	  buffer[index] += (float)(((char*)decbuf)[i+j])/(float)SCHAR_MAX;
	}
	buffer[index++] /= channels;
	if (index >= nbsamples) break;
      }
      break;
    case MPG123_ENC_FLOAT_32:
      for (i = 0; i < done/sizeof(float); i+=channels){
	buffer[index] = 0.0f;
	for (j = 0; j < channels; j++){
	  buffer[index] += ((float*)decbuf)[i+j];
	}
	buffer[index++] /= channels;
	if (index >= nbsamples) break;
      }
      break;
    default:
	done = 0;
    }

  } while (ret == MPG123_OK && index < nbsamples);

  free(decbuf);
  mpg123_close(m);
  mpg123_delete(m);
  mpg123_exit();

  return buffer;
}

#endif /*HAVE_LIBMPG123*/

static
float *readaudio_snd(const char *filename, long *sr, const float nbsecs, unsigned int *buflen){

    SF_INFO sf_info;
    sf_info.format=0;
    SNDFILE *sndfile = sf_open(filename, SFM_READ, &sf_info);
    if (sndfile == NULL){
      return NULL;
    }

    /* normalize */
    sf_command(sndfile, SFC_SET_NORM_FLOAT, NULL, SF_TRUE);

    *sr = (long)sf_info.samplerate;

    //allocate input buffer for signal
    unsigned int src_frames = (nbsecs <= 0) ? sf_info.frames : (nbsecs*sf_info.samplerate);
    src_frames = (sf_info.frames < src_frames) ? sf_info.frames : src_frames;
    float *inbuf = (float*)malloc(src_frames*sf_info.channels*sizeof(float));

    /*read frames */
    sf_count_t cnt_frames = sf_readf_float(sndfile, inbuf, src_frames);
    float *buf = (float*)malloc(cnt_frames*sizeof(float));

    //average across all channels
    int  i,j,indx=0;
    for (i=0;i<cnt_frames*sf_info.channels;i+=sf_info.channels){
	buf[indx] = 0;
	for (j=0;j<sf_info.channels;j++){
	    buf[indx] += inbuf[i+j];
	}
	buf[indx++] /= sf_info.channels;
    }
    free(inbuf);

    *buflen = indx;
    return buf;
}

float* ph_readaudio2(const char *filename, int sr, float *sigbuf, int &buflen, const float nbsecs){
  long orig_sr;
  float *inbuffer = NULL;
  unsigned int inbufferlength;
  buflen = 0;

  const char *suffix = strrchr(filename, '.');
  if (suffix == NULL) return NULL;
  if (!strcasecmp(suffix+1, "mp3")) {
#ifdef HAVE_LIBMPG123
    inbuffer = readaudio_mp3(filename, &orig_sr, nbsecs, &inbufferlength);
#endif /* HAVE_LIBMPG123 */
  } else {
    inbuffer = readaudio_snd(filename, &orig_sr, nbsecs, &inbufferlength);
  }

  if (inbuffer == NULL){
    return NULL;
  }

  /* resample float array */
  /* set desired sr ratio */
  double sr_ratio = (double)(sr)/(double)orig_sr;
  if (src_is_valid_ratio(sr_ratio) == 0){
    free(inbuffer);
    return NULL;
  }

  /* allocate output buffer for conversion */
  unsigned int outbufferlength = sr_ratio*inbufferlength;
  float *outbuffer = (float*)malloc(outbufferlength*sizeof(float));
  if (!outbuffer){
    free(inbuffer);
    return NULL;
  }

  int error;
  SRC_STATE *src_state = src_new(SRC_LINEAR, 1, &error);
  if (!src_state){
    free(inbuffer);
    free(outbuffer);
    return NULL;
  }

  SRC_DATA src_data;
  src_data.data_in = inbuffer;
  src_data.data_out = outbuffer;
  src_data.input_frames = inbufferlength;
  src_data.output_frames = outbufferlength;
  src_data.end_of_input = SF_TRUE;
  src_data.src_ratio = sr_ratio;

  /* sample rate conversion */
  if (src_process(src_state, &src_data)){
    free(inbuffer);
    free(outbuffer);
    src_delete(src_state);
    return NULL;
  }

  buflen = src_data.output_frames;

  src_delete(src_state);
  free(inbuffer);

  return outbuffer;
}


float* ph_readaudio(const char *filename, int sr, int channels, float *sigbuf, int &buflen,\
		    const float nbsecs){
	if(!filename || sr <= 0)
		return NULL;
    return ph_readaudio2(filename, sr, sigbuf, buflen, nbsecs);
}

uint32_t* ph_audiohash(float *buf, int N, int sr, int &nb_frames){

   int frame_length = 4096;//2^12
   int nfft = frame_length;
   int nfft_half = 2048;
   int start = 0;
   int end = start + frame_length - 1;
   int overlap = (int)(31*frame_length/32);
   int advance = frame_length - overlap;
   int index = 0;
   nb_frames = (int)(floor((float)(N/advance)) - floor((float)(frame_length/advance)) + 1);
   double window[frame_length];
   for (int i = 0;i<frame_length;i++){
       //hamming window
       window[i] = 0.54 - 0.46*cos(2*M_PI*i/(frame_length-1));
   }

   double frame[frame_length];
   //fftw_complex *pF;
   //fftw_plan p;
   //pF = (fftw_complex*)fftw_malloc(sizeof(fftw_complex)*nfft);
   scomplex *pF = (scomplex*)malloc(sizeof(scomplex)*nfft);


   double magnF[nfft_half];
   double maxF = 0.0;
   double maxB = 0.0;

   double minfreq = 300;
   double maxfreq = 3000;
   double minbark = 6*asinh(minfreq/600.0);
   double maxbark = 6*asinh(maxfreq/600.0);
   double nyqbark = maxbark - minbark;
   int nfilts = 33;
   double stepbarks = nyqbark/(nfilts - 1);
   int nb_barks = (int)(floor(nfft_half/2 + 1));
   double barkwidth = 1.06;

   double freqs[nb_barks];
   double binbarks[nb_barks];
   double curr_bark[nfilts];
   double prev_bark[nfilts];
   for (int i=0;i< nfilts;i++){
       prev_bark[i] = 0.0;
   }
   uint32_t *hash = (uint32_t*)malloc(nb_frames*sizeof(uint32_t));
   double lof,hif;

   for (int i=0; i < nb_barks;i++){
       binbarks[i] = 6*asinh(i*sr/nfft_half/600.0);
       freqs[i] = i*sr/nfft_half;
   }
   double **wts = new double*[nfilts];
   for (int i=0;i<nfilts;i++){
      wts[i] = new double[nfft_half];
   }
   for (int i=0;i<nfilts;i++){
       for (int j=0;j<nfft_half;j++){
	   wts[i][j] = 0.0;
       }
   }

   //calculate wts for each filter
   for (int i=0;i<nfilts;i++){
       double f_bark_mid = minbark + i*stepbarks;
       for (int j=0;j<nb_barks;j++){
	   double barkdiff = binbarks[j] - f_bark_mid;
           lof = -2.5*(barkdiff/barkwidth - 0.5);
           hif = barkdiff/barkwidth + 0.5;
           double m = std::min(lof,hif);
           m = std::min(0.0,m);
           m = pow(10,m);
           wts[i][j] = m;
       }
   }

   //p = fftw_plan_dft_r2c_1d(frame_length,frame,pF,FFTW_ESTIMATE);

   while (end < N){
       maxF = 0.0;
       maxB = 0.0;
       for (int i = 0;i<frame_length;i++){
	   frame[i] = window[i]*buf[start+i];
       }
       //fftw_execute(p);

       if (fft(frame, frame_length, pF) < 0){
	   return NULL;
       }
       for (int i=0; i < nfft_half;i++){
	   //magnF[i] = sqrt(pF[i][0]*pF[i][0] +  pF[i][1]*pF[i][1] );
           magnF[i] = cabs(pF[i]);
	   if (magnF[i] > maxF){
	       maxF = magnF[i];
	   }
       }

       for (int i=0;i<nfilts;i++){
	   curr_bark[i] = 0;
	   for (int j=0;j < nfft_half;j++){
	       curr_bark[i] += wts[i][j]*magnF[j];
	   }
           if (curr_bark[i] > maxB)
	       maxB = curr_bark[i];
       }

       uint32_t curr_hash = 0x00000000u;
       for (int m=0;m<nfilts-1;m++){
	   double H = curr_bark[m] - curr_bark[m+1] - (prev_bark[m] - prev_bark[m+1]);
	   curr_hash = curr_hash << 1;
	   if (H > 0)
	       curr_hash |= 0x00000001;
       }


       hash[index] = curr_hash;
       for (int i=0;i<nfilts;i++){
	   prev_bark[i] = curr_bark[i];
       }
       index += 1;
       start += advance;
       end   += advance;
   }
   //fftw_destroy_plan(p);
   //fftw_free(pF);
   free(pF);
   for (int i=0;i<nfilts;i++){
       delete [] wts[i];
   }
   delete [] wts;
   return hash;
}


int ph_bitcount(uint32_t n){

    //parallel bit count
    #define MASK_01010101 (((uint32_t)(-1))/3)
    #define MASK_00110011 (((uint32_t)(-1))/5)
    #define MASK_00001111 (((uint32_t)(-1))/17)

    n = (n & MASK_01010101) + ((n >> 1) & MASK_01010101) ;
    n = (n & MASK_00110011) + ((n >> 2) & MASK_00110011) ;
    n = (n & MASK_00001111) + ((n >> 4) & MASK_00001111) ;
    return n % 255;

}

double ph_compare_blocks(const uint32_t *ptr_blockA,const uint32_t *ptr_blockB, const int block_size){
    double result = 0;
    for (int i=0;i<block_size;i++){
	uint32_t xordhash = ptr_blockA[i]^ptr_blockB[i];
        result += ph_bitcount(xordhash);
    }
    result = result/(32*block_size);
    return result;
}

double* ph_audio_distance_ber(uint32_t *hash_a , const int Na, uint32_t *hash_b, const int Nb, const float threshold, const int block_size, int &Nc){

    uint32_t *ptrA, *ptrB;
    int N1, N2;
    if (Na <= Nb){
	ptrA = hash_a;
	ptrB = hash_b;
	Nc = Nb - Na + 1;
	N1 = Na;
        N2 = Nb;
    } else {
	ptrB = hash_a;
	ptrA = hash_b;
	Nc = Na - Nb + 1;
	N1 = Nb;
	N2 = Na;
    }

    double *pC = new double[Nc];
    if (!pC)
	return NULL;
    int k,M,nb_above, nb_below, hash1_index,hash2_index;
    double sum_above, sum_below,above_factor, below_factor;

    uint32_t *pha,*phb;
    double *dist = NULL;

    for (int i=0; i < Nc;i++){

	M = (int)floor(std::min(N1,N2-i)/block_size);

        pha = ptrA;
        phb = ptrB + i;

	double *tmp_dist = (double*)realloc(dist, M*sizeof(double));
        if (!tmp_dist){
	    return NULL;
        }
        dist = tmp_dist;
	dist[0] = ph_compare_blocks(pha,phb,block_size);

	k = 1;

	pha += block_size;
	phb += block_size;

	hash1_index = block_size;
	hash2_index = i + block_size;

	while ((hash1_index < N1 - block_size)  && (hash2_index < N2 - block_size)){
	    dist[k++] = ph_compare_blocks(pha,phb,block_size);
	    hash1_index += block_size;
	    hash2_index += block_size;
	    pha += block_size;
	    phb += block_size;
	}
        sum_above = 0;
	sum_below = 0;
	nb_above = 0;
	nb_below = 0;
	for (int n = 0; n < M; n++){

	    if (dist[n] <= threshold){
		sum_below += 1-dist[n];
		nb_below++;
	    } else {
		sum_above += 1-dist[n];
		nb_above++;
	    }
	}
	above_factor = sum_above/M;
	below_factor = sum_below/M;
	pC[i] = 0.5*(1 + below_factor - above_factor);
    }

    free(dist);
    return pC;
}
#ifdef HAVE_PTHREAD

void *ph_audio_thread(void *p)
{
        slice *s = (slice *)p;
        for(int i = 0; i < s->n; ++i)
        {
                DP *dp = (DP *)s->hash_p[i];
                int N, count;
		pair<int,int> *p = (pair<int,int> *)s->hash_params;
                float *buf = ph_readaudio(dp->id, p->first, p->second, NULL, N);
                uint32_t *hash = ph_audiohash(buf, N, p->first, count);
                free(buf);
                buf = NULL;
                dp->hash = hash;
                dp->hash_length = count;
        }
}

DP** ph_audio_hashes(char *files[], int count, int sr, int channels, int threads)
{
        if(!files || count == 0)
	        return NULL;

        int num_threads;
        if(threads > count)
        {
                num_threads = count;
        }
	else if(threads > 0)
        {
                num_threads = threads;
        }
	else
	{
                num_threads = ph_num_threads();
        }

	DP **hashes = (DP**)malloc(count*sizeof(DP*));

        for(int i = 0; i < count; ++i)
        {
                hashes[i] = (DP *)malloc(sizeof(DP));
                hashes[i]->id = strdup(files[i]);
        }

	pthread_t thds[num_threads];

        int rem = count % num_threads;
        int start = 0;
        int off = 0;
        slice *s = new slice[num_threads];
        for(int n = 0; n < num_threads; ++n)
        {
                off = (int)floor((count/(float)num_threads) + (rem>0?num_threads-(count % num_threads):0));

                s[n].hash_p = &hashes[start];
                s[n].n = off;
		s[n].hash_params = new pair<int,int>(sr,channels);
                start += off;
                --rem;
                pthread_create(&thds[n], NULL, ph_audio_thread, &s[n]);
        }
	for(int i = 0; i < num_threads; ++i)
        {
                pthread_join(thds[i], NULL);
		delete (pair<int,int>*)s[i].hash_params;
        }
        delete[] s;

	return hashes;

}

#endif
