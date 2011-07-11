#ifndef PHASHCOMM_H
#define PHASHCOMM_H

#include <vector>

typedef struct phashcommcfg_t
{
  BOOL stop;
  WAVEFORMATEX format;
  int pcmtype;
  std::vector<BYTE>* data;
  int index;

  REFERENCE_TIME stime;
  REFERENCE_TIME etime;

  std::vector<int> cfg;
} PHashCommCfg_st;
 
#endif