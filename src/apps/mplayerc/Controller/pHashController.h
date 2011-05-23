#ifndef PHASHCONTROLLER_H
#define PHASHCONTROLLER_H

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

#include <threadhelper.h>
#include <fstream>
#include <Windows.h>
#include "LazyInstance.h"
#include "phashapi.h"
#include "pHashController.h"
#include "NetworkControlerImpl.h" 
#include "../Model/pHashModel.h"
#include "../MainFrm.h"
#include "HashController.h"
#include "Strings.h"
#define NORMALIZE_DB_MIN -145
#define NORMALIZE_DB_MAX 60


class pHashController:
  public LazyInstanceImpl<pHashController>,
  public ThreadHelperImpl<pHashController>,
  public NetworkControlerImpl
{
public:
  pHashController(void);
  ~pHashController(void);
  enum {
    LOOKUP         = 0x01 << 2,   // 0000 0100
    INSERT         = 0x01 << 3,   // 0000 1000
  };
  enum {NOCALCHASH = 0, CALPHASH};
  void _thread_GetAudiopHash();
  void _thread_GetpHash();
  HRESULT _thread_DigestpHashData();                        // down samplerate and mix
  HRESULT _thread_MonopHash();                              // test only. Get each channel data and calc phash
  
  void Init(CComQIPtr<IAudioSwitcherFilter> pASF, std::wstring m_fnCurPlayingFile);
  int GetSwitchStatus();
  void SetSwitchStatus(int status);
  int GetCmd();
  void SetCmd(uint8_t cmd);
  BOOL IsSeek();
  void SetSeek(BOOL seekflag);
  void Execute(BOOL isrun);
  void _Thread();
  void CheckEnv(int64_t timelength);
  void ResetAll();
  void IspHashInNeed(const wchar_t* filepath, int& result);
  void ReleasePhash(UINT pos);
  void ReleasePhashAll();
private:
  void HookData(CComQIPtr<IAudioSwitcherFilter> pASF);
  
  PHASHBLOCK m_phashblock;
  uint8_t m_cmd;
  std::wstring m_sphash;
  uint32_t** m_hashes;                                     // Storage pHashes
  int *m_lens;
  int m_phashlen;
  float* m_buffer;
  int m_bufferlen;
  int m_sr;                                               // sample rate to convert the stream
  int m_phashswitcher;
  int m_hashcount;
  int m_seekflag;
  CComQIPtr<IAudioSwitcherFilter> m_pASF;
  // Sample to float and normalized
  BOOL SampleToFloat(const unsigned char* const indata, float* outdata, int samples, int type);
  
  // DownSample
  BOOL DownSample(float* inbuf, int nsample, int des_sr, int org_sr, float** outbuf, int& outlen);
  
  // Mix
  BOOL MixChannels(float* buf, int samples, int channels, int nsample, float* MonoChannelBuf);
  void SixchannelsToStereo(float *output, float *input, int n);

  // Upload phash 
  void _thread_UploadpHash();
  void _thread_GetpHashAndSend(int cmd);
  int SendOnepHashFrame(phashbox phashframe);

};

class pHashSender:
  public ThreadHelperImpl<pHashSender>
{
public:
  pHashSender();
  ~pHashSender();
  
  void _Thread();
  void SetSphash(std::wstring phash);
  void SetPhash(phashbox*);
  
private:
  std::wstring m_sphash;
  int SendOnepHashFrame();
  phashbox* m_phashbox;
  const static int m_timeout  = 10;

};
#endif //PHASHCONTROLLER_H