

//////////////////////////////////////////////////////////////////////////
//
//  pHashController is a global instance controller that calculates
//  phash for the current playback video file.
//
//  Soleo
//
//   Status: Now we use class pHashController to get set, calc pHash, 
//           and use class pHashSender to send pHash to Server.
//
//   pHash Demo: This is a demo with phash support.(Test Only) In this demo, we calculated 2 file's phash, and compared with each other. 
//               Each file deliverd four times decoded data which begin at 10 secs, 30 secs, 50secs, 110secs. The duration is 
//               10secs. We use audio phash following to the steps:
//   Related Key Methods: _thread_GetAudiopHash(), _thread_DigestpHashData()        
//
//   FILE 1: Samples to float --> Normalization and channels merged --> Reduce samplerate to 8kHz --> Calc pHash --
//                                                                                                                 |---> Comparing two phashes
//   FILE 2: Samples to float --> Normalization and channels merged --> Reduce samplerate to 8kHz --> Calc pHash --
//  
//          Now we are able to get 2 channels vs 2 channels(same channel amount) video and audio phash, the results are not bad.
//          But 6 channels vs 2 channels (different channel amount) situation is still a problem.
//          

#pragma once

#include "LazyInstance.h"
#include <threadhelper.h>
#include <stdint.h>
#include <list>

class pHashController:
  public LazyInstanceImpl<pHashController>,
  public ThreadHelperImpl<pHashController>
{
public:
  pHashController(void);
  ~pHashController(void);

  void _Thread();
  void NewData();
  void Check(REFERENCE_TIME& time, CComQIPtr<IMediaSeeking> ms,
    CComQIPtr<IAudioSwitcherFilter> pASF, CString file);
  void UnRefs();

  PHashCommCfg_st PHashCommCfg;
private:
  std::list<std::vector<BYTE> > m_data;
  int m_datarefs;
  CString m_file;
  std::wstring m_sphash;
};

class PHashHandler:
  public ThreadHelperImpl<PHashHandler>
{
public:
  PHashHandler(std::vector<BYTE>* data, std::wstring sphash);
  ~PHashHandler(void);

  void _Thread();

private:
  BOOL SamplesToPhash();
  BOOL DownSample(float* inbuf, int nsample, int des_sr,
    int org_sr, float** outbuf, int& outlen);
  BOOL MixChannels(float* buf, int samples, int channels,
    int nsample, float* MonoChannelBuf);
  void SixchannelsToStereo(float* output, float* input, int n);
  BOOL SampleToFloat(const unsigned char* const indata, float* outdata, int samples, int type);

private:
  std::vector<BYTE>* m_data;
  std::wstring m_sphash;
  uint32_t* m_phash;
  int m_phashlen;
  int m_sr;
};