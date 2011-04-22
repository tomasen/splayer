#ifndef PHASHMODEL_H
#define PHASHMODEL_H

#include <vector>
#include <stdint.h>
typedef struct phashblock_t
{
public:
  phashblock_t():
    phashcnt(0),
    prevcnt(-1),
    isseek(FALSE),
    isrun(FALSE){}

  WAVEFORMATEX format;
  std::vector<unsigned char> phashdata;
  int phashcnt;
  int prevcnt;
  int type;
  BOOL isseek;
  BOOL isrun;
} PHASHBLOCK;

// sending pHash frame
typedef struct phashbox_t{
  uint8_t cmd;           
  uint8_t earlyendflag;  // if end earlier , set the flag to 1; 
  uint8_t amount;        // amount of phash times
  uint8_t id;            // order of phash
  uint32_t nbframes;     // length of phash 
  uint32_t* phash;
} phashbox;



enum {
  CFG_PHASHTIMES = 0,
  CFG_PHASHDATASECS,
  CFG_PHASHSTARTTIME
};

// We have checked the time length is long enough, so it is safe to use this cfg
const int g_phash_collectcfg[] = {
  // times;
  2,
  // datasecs;
  20,
  // start collect time;
  0,  // this is a placeholder
  0,
  30,
  20,
  30
};


#endif //PHASHMODEL_H