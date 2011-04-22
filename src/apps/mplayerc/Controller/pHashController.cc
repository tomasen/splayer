#include "stdafx.h"
#include <Strings.h>
#include <io.h> 
#include <fcntl.h>
#include <sys/stat.h>
#include <string>
#include <fstream>
#include "samplerate.h"
#include "sndfile.h"
#include "pHashController.h"
#include "zmq/zmqhelper.h"
#include "../apps/mplayerc/Model/pHashModel.h"
#include "../FGManager.h"

#define FREE_PHASHMEM() \
  m_phashblock.phashdata.clear(); \
  m_phashblock.phashdata.resize(0);

#define PHASH_SERVER "tcp://192.168.10.45:5000"

pHashController::pHashController(void) :
  m_buffer(NULL),
  m_hashes(NULL),
  m_lens(NULL),
  m_sr(8000),
  m_phashswitcher(CALPHASH),
  m_bufferlen(0),
  m_phashlen(0),
  m_hashcount(0),
  m_seekflag(false)
{
  
}

pHashController::~pHashController(void)
{

}

int pHashController::GetSwitchStatus()
{
  return m_phashswitcher;
}

void pHashController::SetSwitchStatus(int status)
{
  m_phashswitcher = status;
}

int pHashController::GetCmd()
{
  return m_cmd;
}
void pHashController::SetCmd(uint8_t cmd)
{
  m_cmd = cmd;
}
BOOL pHashController::IsSeek()
{
  return m_phashblock.isseek; 
}

void pHashController::SetSeek(BOOL seekflag)
{  
  m_phashblock.isseek = seekflag;
}
void pHashController::Execute(BOOL isrun)
{
  m_phashblock.isrun = isrun;
}
void pHashController::_Thread()
{
  Logging( L"pHashController::_thread enter %x", m_phashswitcher);
  if (m_phashswitcher != NOCALCHASH)
  {
    //_thread_MonopHash();
    try
    {
      _thread_DigestpHashData();
      _thread_GetpHash();
    }
    catch(...)
    {
      Logging(L"pHashController thread crash");
    }
  }
  else
  {
    Logging(L"m_phashswitcher = NOCALCHASH ");
  }
  
  Logging( L"pHashController::_thread exit %x", m_phashswitcher);
}

HRESULT pHashController::_thread_MonopHash()
{
  int buflen = m_phashblock.phashdata.size();
  float *buf = new float[buflen];

  int samplebyte = (m_phashblock.format.wBitsPerSample >> 3);                          // the size of one sample in Byte unit
  int nsample = m_phashblock.phashdata.size()/m_phashblock.format.nChannels/samplebyte;    // each channel sample amount
  int samples = nsample * m_phashblock.format.nChannels;                               // all channels sample amount

  // input buffer for signal
  unsigned char* indata = &m_phashblock.phashdata[0];

  // Making all data into float type 
  if (SampleToFloat(indata, buf, samples, m_phashblock.type) == FALSE)
  {
    delete [] buf;
    FREE_PHASHMEM();
    return S_FALSE;
  }
  
  int chns = m_phashblock.format.nChannels;
  float* tmpmem = new float[nsample];
  
  for (int n = 0; n < chns; ++n)
  {
    memset(tmpmem, 0, sizeof(tmpmem));
    for (int j = 0; j < nsample; j++)
      tmpmem[j] = buf[n+chns*j];
    wfstream fs;
    wchar_t filepath[80];
    wsprintf(filepath, L"C:\\Users\\Staff\\Desktop\\splayer_shao\\hash\\txt_chn_%d.txt", n);
    fs.open(filepath, ios_base::out | ios_base::app);
    for (int i=0; i < nsample; ++i)
     fs << tmpmem[i] << L"\r\n";
    fs.close();
    float* outmem = NULL;
    int outnums;
    DownSample(tmpmem, nsample, m_sr, m_phashblock.format.nSamplesPerSec, &outmem, outnums);
    m_hashes[m_hashcount] = ph_audiohash(outmem, outnums, m_sr, m_phashlen);
    m_lens[m_hashcount] = m_phashlen;

    free(outmem);
    m_hashcount++;
  }
  
  delete [] tmpmem;
  FREE_PHASHMEM();

  if (m_hashcount < 7)
    return S_OK;


  for (int j = 0; j < 2; ++j)
  {
    for (int ii = 0; ii < 6; ++ii)
    {
      int nc;
      double* cs;
      cs = ph_audio_distance_ber(m_hashes[j], m_lens[j], m_hashes[ii+2], m_lens[ii+2],
        0.3, 256, nc);                                                             // threshold 0.3 , block_size: 256 
      if (!cs)
        ;
      else
      {
        double max_cs = 0.0;
        for (int i=0;i<nc;i++)
        {
          if (cs[i] > max_cs)
            max_cs = cs[i];
        }
        sphash_freemem2(cs);
      }   
    }
  }

  return S_OK;
}

void pHashController::ReleasePhash(UINT pos)
{
  sphash_freemem(NULL, m_hashes[pos]);
  m_hashes[pos] = NULL;
  m_lens[pos] = 0;
  if (pos == (g_phash_collectcfg[CFG_PHASHTIMES] - 1))
  {
    ReleasePhashAll();
    m_phashblock.prevcnt = -1;
    m_phashblock.phashcnt = 0;
    Execute(FALSE);
  }
}

void pHashController::ResetAll()
{
  m_hashes = (uint32_t**)malloc(g_phash_collectcfg[CFG_PHASHTIMES]*sizeof(uint32_t*));
  m_lens = (int*)malloc(g_phash_collectcfg[CFG_PHASHTIMES]*sizeof(int));
  m_phashblock.prevcnt = -1;
  m_phashblock.phashcnt = 0;
  SetSeek(FALSE);
}

void pHashController::ReleasePhashAll()
{
  if (m_hashes)
    free(m_hashes);
  if (m_lens)
    free(m_lens);
  m_hashes = NULL;
  m_lens = NULL;
  FREE_PHASHMEM();
}
void pHashController::_thread_GetpHash()
{
  if (IsSeek())
  {
    SetSwitchStatus(NOCALCHASH);
    Execute(FALSE);
    return;
  }

  // calc phash
  UINT pos = m_phashblock.phashcnt-1;

  m_hashes[pos] = ph_audiohash(m_buffer, m_bufferlen, m_sr, m_phashlen);
  m_lens[pos] = m_phashlen;

  free(m_buffer);
  m_buffer = NULL;
  m_bufferlen = 0;

  // send phash
  pHashSender* sender = new pHashSender;

  phashbox* hashbox = new phashbox;
  hashbox->cmd = GetCmd();
  hashbox->amount = g_phash_collectcfg[CFG_PHASHTIMES];
  hashbox->id = pos;    
  hashbox->nbframes = m_lens[pos];
  hashbox->phash = m_hashes[pos];
  hashbox->earlyendflag = 0;

  if (GetCmd() == INSERT)
      sender->SetSphash(m_sphash);
  
  sender->SetPhash(hashbox);

  sender->_Start();
}

void pHashController::_thread_GetAudiopHash()
{
  int count = g_phash_collectcfg[CFG_PHASHTIMES]*2-1;
  int step = g_phash_collectcfg[CFG_PHASHTIMES];
  if (m_phashblock.phashcnt <= count)
  {
    int i = m_phashblock.phashcnt;
    m_hashes[i] = ph_audiohash(m_buffer, m_bufferlen, m_sr, m_phashlen);
    m_lens[i] = m_phashlen;
    Logging(L"m_phashblock.phashcnt:%d, length = %d, hashes[0]:%X, m_phashlen:%d",i, m_lens[i], m_hashes[i][0], m_phashlen);
    free(m_buffer);
    m_buffer = NULL;
    m_bufferlen = 0;
  }

  if (m_phashblock.phashcnt == count)
  {
      int nc;
      double* cs;
      for (int index = 0; index < step; index++)
      { 

        Logging(L"m_phash1[%d]:%X,%X,%X,%X mphash1len[%d]:%d",index, m_hashes[index][0], m_hashes[index][1],
          m_hashes[index][2],m_hashes[index][3], index, m_lens[index] );
        Logging(L"m_phash2[%d]:%X,%X,%X,%X mphash2len[%d]:%d",index, m_hashes[index+step][0], m_hashes[index+step][1],
          m_hashes[index+step][2],m_hashes[index+step][3], index, m_lens[index+step] );

        cs = ph_audio_distance_ber(m_hashes[index], m_lens[index], m_hashes[index+step], m_lens[index+step],
          0.3, 256, nc);                                                         // threshold 0.3 , block_size: 256 
        if (!cs)
          return;
        else
        {
          double max_cs = 0.0;
          for (int i=0;i<nc;i++)
          {
            if (cs[i] > max_cs)
              max_cs = cs[i];
          }
          Logging(L"Confidence Score: %f",max_cs);
          sphash_freemem2(cs);
          cs = NULL;
        }
        // free space
        sphash_freemem(NULL, m_hashes[index]);
        sphash_freemem(NULL, m_hashes[index+step]);
        m_hashes[index] = NULL;
        m_hashes[index+step] = NULL;
        m_lens[index] = 0;
        m_lens[index+step] = 0;  
      }
      free(m_lens);
      free(m_hashes);
  }
  m_phashblock.prevcnt = m_phashblock.phashcnt;
  m_phashblock.phashcnt++;
  if (m_phashblock.phashcnt > count)
    m_phashblock.phashcnt = 0;
}
void pHashController::HookData(CComQIPtr<IAudioSwitcherFilter> pASF)
{
  pASF->SetpHashControl(&m_phashblock);
}

void pHashController::CheckEnv(int64_t timelength)
{
  // check timelength
  if (timelength < (10000000i64)*60*45)
  {
    Logging("time length < 45 mins");
    SetSwitchStatus(FALSE);
    Execute(FALSE);
  }

  // TODO: check PC configuration, give up if the configuration is to low
  
}
void pHashController::Init(CComQIPtr<IAudioSwitcherFilter> pASF, std::wstring m_fnCurPlayingFile)
{
  if (pASF)
  {
    ResetAll();
    if (GetSwitchStatus() == NOCALCHASH)
      return;

    int result;
    IspHashInNeed(m_fnCurPlayingFile.c_str(), result);
    switch(result)
    {
    case NOCALCHASH:
      Execute(FALSE);
      break;
    case CALPHASH:
      // do insert 
      HookData(pASF);
      Execute(TRUE);
      uint8_t cmd = (result == 1) ? INSERT : LOOKUP;
      SetCmd(cmd);
      break;
    }
  }
}


void pHashController::IspHashInNeed(const wchar_t* filepath, int& result)
{
  m_sphash = HashController::GetInstance()->GetSPHash(filepath);

  sinet::refptr<sinet::pool>    net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task>    net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request> net_rqst = sinet::request::create_instance();
  sinet::refptr<sinet::config>  net_cfg  = sinet::config::create_instance();
  
  net_task->use_config(net_cfg);
  wchar_t tmpurl[512];
  _snwprintf_s(tmpurl, 512, 512, L"http://webpj:8080/misc/phash.php?req=%d&sphs=%s\n", 1, m_sphash.c_str());
  net_rqst->set_request_url(tmpurl);
  net_rqst->set_request_method(REQ_GET);
  net_task->append_request(net_rqst);
  net_pool->execute(net_task);
    
  while (net_pool->is_running_or_queued(net_task))
  {
    if ( _Exit_state(500))
      return;
  }
  
  //error code dealing
  if (net_rqst->get_response_errcode()!= 0)
  {
    Logging(L"ERROR: sending failed");
    result = -1;
    return;
  }

  // response data
  std::vector<unsigned char> st_buffer = net_rqst->get_response_buffer();
  st_buffer.push_back(0);
  std::string reply =  (char*)&st_buffer[0];
  std::string ret = reply.substr(0, reply.find_first_of("\n"));
  
  if ( ret == "1")
    result = 1;
  else if (ret == "0")
    result = 0;

}
BOOL pHashController::SampleToFloat(const unsigned char* const indata, float* outdata, int samples, int type)
{
  int samplecnt = 0;
  int tmp;

  switch (type)
  {
   // PCM8
  case CAudioSwitcherFilter::WETYPE_PCM8:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)      
      outdata[samplecnt] = ((float)(((BYTE*)indata)[samplecnt]) -0x7f) / 0x80; //UCHAR_MAX
    break;
   // PCM16
  case CAudioSwitcherFilter::WETYPE_PCM16:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
      outdata[samplecnt] = (float)(((short*)indata)[samplecnt])/(float)SHRT_MAX;
    break;  
   // PCM24
  case CAudioSwitcherFilter::WETYPE_PCM24:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
    {
      memcpy(((BYTE*)&tmp)+1, &indata[3*samplecnt], 3);  
      outdata[samplecnt] += (float)(tmp >> 8) / ((1<<23)-1);
    }
    break;
   // PCM32
  case CAudioSwitcherFilter::WETYPE_PCM32:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
      outdata[samplecnt] = (float)(((int*)indata)[samplecnt]);
    break;
    // FPCM32
  case CAudioSwitcherFilter::WETYPE_FPCM32:
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
      outdata[samplecnt] = (float)(((float*)indata)[samplecnt]);
    break;
    // FPCM64: did not have it tested
  case CAudioSwitcherFilter::WETYPE_FPCM64: 
    for (samplecnt = 0; samplecnt < samples; samplecnt++)
      outdata[samplecnt] = (float)(((double*)indata)[samplecnt]);
    break;

  default:
    return FALSE;
  }

  return TRUE;
}

// mix into one channel and reduced samplerate to 8000HZ for further cal
HRESULT pHashController::_thread_DigestpHashData()
{
  if (GetSwitchStatus() == NOCALCHASH)
  { 
    ReleasePhashAll();
    return S_FALSE;
  }

  int buflen = m_phashblock.phashdata.size();
  float *buf = new float[buflen];

  int samplebyte = (m_phashblock.format.wBitsPerSample >> 3);                             // the size of one sample in Byte unit
  int nsample = m_phashblock.phashdata.size()/m_phashblock.format.nChannels/samplebyte;       // each channel sample amount
  int samples = nsample * m_phashblock.format.nChannels;                                  // all channels sample amount
  
  // alloc input buffer for signal
  unsigned char* indata = &m_phashblock.phashdata[0];

  // Making all data into float type 
  if (SampleToFloat(indata, buf, samples, m_phashblock.type) == FALSE)
  {
    delete [] buf;
    FREE_PHASHMEM();
    return S_FALSE;
  }
  else
  {
    FREE_PHASHMEM();
  }

  // Mix as Mono channel
  int MonoLen = buflen/m_phashblock.format.nChannels;
  float* MonoChannelBuf = new float[MonoLen];
  if (MixChannels(buf, samples, m_phashblock.format.nChannels, nsample, MonoChannelBuf) == TRUE)
  {
    delete[] buf;
    FREE_PHASHMEM();
  }
  else
  {
    delete[] buf;
    delete[] MonoChannelBuf;
    FREE_PHASHMEM();
    return S_FALSE;
  }

  // Samplerate to 8kHz 
  float* outmem = NULL;
  int outnums = 0;
  if (DownSample(MonoChannelBuf, nsample, m_sr, m_phashblock.format.nSamplesPerSec, &outmem, outnums) == TRUE)
  {
    delete[] MonoChannelBuf;
    m_buffer = outmem;
    m_bufferlen = outnums;
  }
  else
  {
    free(outmem);
    FREE_PHASHMEM();
    delete[] MonoChannelBuf;
    return S_FALSE;
  }

  return S_OK;
}
BOOL pHashController::DownSample(float* inbuf, int nsample, int des_sr, int org_sr, float** outbuf, int& outlen)
{
  // resample float array , set desired samplerate ratio
  double sr_ratio = (double)(des_sr)/(double)org_sr;
  if (src_is_valid_ratio(sr_ratio) == 0)
  {
    Logging(L"sr ratio is invalid,sr_ratio:%f,sr:%d,samplerate:%d",sr_ratio, des_sr, org_sr);
    FREE_PHASHMEM();
    return FALSE;
  }

  // allocate output buffer for conversion
  outlen = sr_ratio * nsample;
  *outbuf = (float*)malloc(outlen * sizeof(float)); 
  if (!outbuf)
  {
    Logging(L"outbuffer create failed!");
    FREE_PHASHMEM();
    return FALSE;
  }
  int error;
  SRC_STATE *src_state = src_new(SRC_LINEAR, 1, &error);
  if (!src_state)
  {
    free(*outbuf);
    FREE_PHASHMEM();
    Logging(L"src_state failed");
    return FALSE;
  }

  SRC_DATA src_data;
  src_data.data_in = inbuf;
  src_data.data_out = *outbuf;
  src_data.input_frames = nsample;
  src_data.output_frames = outlen;
  src_data.end_of_input = SF_TRUE;
  src_data.src_ratio = sr_ratio;

  // Sample rate conversion
  if (src_process(src_state, &src_data))
  {
    free(*outbuf);
    free(inbuf);
    src_delete(src_state);
    FREE_PHASHMEM();
    Logging(L"src_process: failed");
    return FALSE;
  }
  src_delete(src_state);
  return TRUE;
}
BOOL pHashController::MixChannels(float* buf, int samples, int channels, int nsample, float* MonoChannelBuf)
{
  int bufindx = 0;
  if (channels == 2)
  {
    do 
    {
      for (int j = 0; j < samples; j += channels)
      {
        MonoChannelBuf[bufindx] = 0.0f;
        for (int i = 0; i < channels; ++i)
          MonoChannelBuf[bufindx] += buf[j+i];
        MonoChannelBuf[bufindx++] /= channels;
      }
    }while (bufindx < nsample);
  }
  else if (channels == 6)
  { // TODO: now i won't work actually
    float* temp = new float[nsample*2];
    memset(temp, 0, sizeof(temp));
    SixchannelsToStereo(temp, buf, nsample);
    for (int i = 1; i < nsample; i++)
    {
       MonoChannelBuf[i] = 0.0f;
       for (int j = 0; j < 2; j++)
      {
        MonoChannelBuf[i] = temp[i+2]; 
      }
      MonoChannelBuf[i] /= 2;
    }
    delete [] temp;
  }
  return TRUE;
}

void pHashController::SixchannelsToStereo(float* output, float* input, int n)
{
  float *p = input;

  while(n-- > 0)
  {
     *output++ = (p[0] + p[1] + p[3] + p[5]) / 3;
     *output++ = (p[1] + p[2] + p[4] + p[5]) / 3;
    p += 6;
  }
}



pHashSender::pHashSender() :
        m_phashbox(NULL)
{

}

pHashSender::~pHashSender()
{

}

void pHashSender::_Thread()
{
  try
  {
    Logging(L"pHashSender::_thread enter ");
    SendOnepHashFrame();
    Logging(L"pHashSender::_thread exit ");
  }
  catch(...)
  {
    Logging(L"pHashSender thread crash");
  }
  
}

int pHashSender::SendOnepHashFrame()
{
  // connection
  void* context = zmq_init(1);
  if (!context)
  {
    Logging(L"zmq_init error");
    return -1;
  }

  void* client = socket_connect(context, ZMQ_REQ, PHASH_SERVER);
  if (!client)
  {
    Logging(L"Client connection failed");
    return -1;
  }
 
  // sending all phashes  
  sendmore_msg_vsm(client, &m_phashbox->cmd, sizeof(uint8_t));
  if (m_phashbox->cmd == pHashController::INSERT)
  {
    std::string sphash = Strings::WStringToString(m_sphash);
    sendmore_msg_data(client, &sphash[0], sphash.length(), NULL, NULL);
  }
  sendmore_msg_vsm(client, &m_phashbox->earlyendflag, sizeof(uint8_t));
  sendmore_msg_vsm(client, &m_phashbox->amount, sizeof(uint8_t));
  sendmore_msg_vsm(client, &m_phashbox->id, sizeof(uint8_t));
  sendmore_msg_vsm(client, &m_phashbox->nbframes, sizeof(uint32_t));
  send_msg_data(client, m_phashbox->phash, m_phashbox->nbframes*sizeof(uint32_t), NULL, NULL);

  if (m_phashbox->earlyendflag == 0)
  {
     Logging("send:cmd:%d, earlyendflag:%d, amount:%d, id:%d, nbframes:%d, phash[0]:%X", 
       m_phashbox->cmd, m_phashbox->earlyendflag, m_phashbox->amount, m_phashbox->id , m_phashbox->nbframes, m_phashbox->phash[0]);
  }

  // get result
  uint8* data = NULL;
  int64_t more;
  size_t msg_size, more_size = sizeof(int64_t);
  int err = 0;
  if (m_phashbox->cmd == pHashController::INSERT)
  {
    uint32_t posid = 0;
    // Receive only positon ID
    err = recieve_msg_timeout(client, &msg_size, &more, &more_size, (void**)&data, m_timeout);
    if (err != 0)
    {
      Logging("Timeout");
      pHashController::GetInstance()->ReleasePhash(m_phashbox->id);
      zmq_close(client);
      zmq_term(context);
      return -1;
    }
    if (msg_size == sizeof(uint32_t))
    {
      memcpy(&posid, data, sizeof(uint32_t));
      Logging("Get Postion ID:%d", posid);
    }
    free(data);
    data = NULL;
    if (more)
      flushall_msg_parts(client);
  } 
  else if (m_phashbox->cmd == pHashController::LOOKUP)
  {
    uint32_t posid = 0;
    float cs = -1.0;
    // Receive positon ID and cs value
    err = recieve_msg_timeout(client, &msg_size, &more, &more_size, (void**)&data, m_timeout);
    if (err != 0)
    {
      Logging("Timeout");
      pHashController::GetInstance()->ReleasePhash(m_phashbox->id);
      zmq_close(client);
      zmq_term(context);
      return -1;
    }

    if (msg_size == sizeof(uint32_t))
    {
      memcpy(&posid, data, sizeof(uint32_t));
      Logging("Get Postion ID: %d", posid);
    }
    free(data);
    data = NULL;

    err = recieve_msg_timeout(client, &msg_size, &more, &more_size, (void**)&data, m_timeout);
    if (err != 0)
    {
      Logging("Timeout");
      pHashController::GetInstance()->ReleasePhash(m_phashbox->id);
      zmq_close(client);
      zmq_term(context);
      return -1;
    }

    if (msg_size == sizeof(float))
    {
      memcpy(&cs, data, sizeof(float));
      Logging("Get cs: %f", cs);
    }
    if (more)
      flushall_msg_parts(client);
  }

  pHashController::GetInstance()->ReleasePhash(m_phashbox->id);
  zmq_close(client);
  zmq_term(context);

  return 0;
}

void pHashSender::SetSphash(std::wstring phash)
{
  m_sphash = phash; 
}

void pHashSender::SetPhash(phashbox* hash)
{
  m_phashbox = hash;
}