// testpHash.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>


#include "phashImpl.h"
#include "pHash.h"
#include "audiophash.h"
//#pragma comment(lib,"libpHash.lib")



int _tmain(int argc, _TCHAR* argv[])
{
  printf("Hello, Testing pHash\n");
  
  // a small test to know if the lib did work
  int a=SampleAddInt(1,3);
  printf("1+3=%d\n",a);

  const char *dir_name ="C:\\projects\\testpHash\\Debug\\a\\a.wav";  //path for the audio which needs testing
  const char *dir_name2 ="C:\\projects\\testpHash\\Debug\\b\\b.wav";
  const float threshold = 0.30;    //BER threshold
  const int block_size = 256;      //number of frames to compare at a time
  const int sr = 8000;             //sample rate to convert the stream
  const int channels = 1;

 
  float *buf, *buf2;
  int buflen;
  uint32_t *hash;
  int hashlen;
  uint32_t *hash2;
  int hashlen2;
  double *cs;
  int Nc;

  printf("Figure out the hash of %s........\n",dir_name);
  buf = ph_readaudio(dir_name, sr, channels, NULL, buflen);
  if (!buf)
  {
    printf("Unable to read audio\n");
    exit(0);
  }
 
  hash = ph_audiohash(buf, buflen, sr, hashlen);
  if (!hash)
  {
      printf("Unable to get the hash\n");
      exit(0);
  }
 
  printf("audio hash is  %u\n",*hash);
  printf("Hash length is %d\n",hashlen);

  printf("Figure out the hash of %s........\n",dir_name2);
  buf2 = ph_readaudio(dir_name2, sr, channels, NULL, buflen);
  if (!buf2)
  {
    printf("Unable to read audio\n");
    exit(0);
  }

  hash2 = ph_audiohash(buf2, buflen, sr, hashlen2);
  if (!hash2)
  {
    printf("Unable to get the hash\n");
    exit(0);
  }

  printf("audio hash is  %u\n",*hash2);
  printf("Hash length is %d\n",hashlen2);

  //dist
   cs = ph_audio_distance_ber(hash, hashlen, hash2, hashlen2, threshold, block_size, Nc);
   if (!cs)
   {
      printf("unable to calc distance\n");
      exit(0);
   }
  
   double max_cs = 0.0;
     for (int i=0;i<Nc;i++)
     {
        if (cs[i] > max_cs)
        {
          max_cs = cs[i];
        }
     }
   printf("max cs %f\n\n", max_cs);
  
   sphash_freemem(buf, hash);
   sphash_freemem(buf2, hash2);
   sphash_freemem2(cs);


  printf("end of the test.......\n");
	system("pause");
  return 0;
}

