
#include "stdafx.h"
#include <sinet.h>
#include "../../../filters/switcher/audioswitcher/AudioSwitcher.h"
#include "pHashController.h"
#include "../Model/PHashComm.h"
#include "HashController.h"
#include "samplerate.h"
#include "sndfile.h"
#include "phashapi.h"
#include "zmq/zmqhelper.h"
#include "Strings.h"

pHashController::pHashController(void)
{
  PHashCommCfg.stop = TRUE;
  PHashCommCfg.index = -1;
  
  PHashCommCfg.stime = 0;
  PHashCommCfg.etime = 0;

  // collect times
  PHashCommCfg.cfg.push_back(1);
  // collect duration (secs)
  PHashCommCfg.cfg.push_back(10);
  // delimiter (don't modify)
  PHashCommCfg.cfg.push_back(0);
  // collect point of time
  PHashCommCfg.cfg.push_back(5);
  PHashCommCfg.cfg.push_back(20);

  m_data.clear();
  m_datarefs = 0;
}

pHashController::~pHashController(void)
{

}

void pHashController::_Thread()
{
  m_sphash = HashController::GetInstance()->GetSPHash(m_file.GetBuffer());
  m_file.ReleaseBuffer();

  sinet::refptr<sinet::pool>    net_pool = sinet::pool::create_instance();
  sinet::refptr<sinet::task>    net_task = sinet::task::create_instance();
  sinet::refptr<sinet::request> net_rqst = sinet::request::create_instance();
  
  wchar_t url[512];
  wsprintf(url, L"http://webpj:8080/misc/phash.php?req=%d&sphs=%s\n", 1, m_sphash.c_str());

  net_rqst->set_request_url(url);
  net_rqst->set_request_method(REQ_GET);
  net_task->append_request(net_rqst);
  net_pool->execute(net_task);
    
  while (net_pool->is_running_or_queued(net_task))
  {
    if ( _Exit_state(500))
      return;
  }
  
  //error code dealing
  if (net_rqst->get_response_errcode() != 0)
    return;

  // response data
  std::vector<unsigned char> st_buffer = net_rqst->get_response_buffer();
  st_buffer.push_back(0);
  std::string ret =  (char*)&st_buffer[0];
  ret.push_back(0);

  if (atoi(ret.c_str()) == 1)
  {
    NewData();
    PHashCommCfg.stop = FALSE;
  }

}

void pHashController::UnRefs()
{
  if (--m_datarefs < 1)
  {
    m_data.clear();
    m_data.resize(0);
  }
}

void pHashController::NewData()
{
  if (PHashCommCfg.index >= PHashCommCfg.cfg.front()-1)
  {
    PHashCommCfg.stop = TRUE;
    PHashHandler* handler = new PHashHandler(PHashCommCfg.data, m_sphash);
    handler->_Start();
    return;
  }

  if (PHashCommCfg.index >= 0)
  {
    PHashHandler* handler = new PHashHandler(PHashCommCfg.data, m_sphash);
    handler->_Start();
  }

  std::vector<BYTE> data;
  m_data.push_back(data);

  PHashCommCfg.data = &(m_data.back());
  PHashCommCfg.index += 1;

  int pos = 3 + PHashCommCfg.index;
  REFERENCE_TIME durtime = (10000000i64) * PHashCommCfg.cfg[1];

  PHashCommCfg.stime = (10000000i64) * PHashCommCfg.cfg[pos];
  PHashCommCfg.etime = PHashCommCfg.stime + durtime;

  m_datarefs++;
}

void pHashController::Check(REFERENCE_TIME& time, CComQIPtr<IMediaSeeking> ms,
                            CComQIPtr<IAudioSwitcherFilter> pASF, CString file)
{
  if (m_datarefs > 0)
    return;

  if (time != 0)
    return;

  if (!ms)
    return;

  __int64 totaltime = 0;
  ms->GetDuration(&totaltime);
  if (totaltime < (10000000i64)*60*45)
    return;

  m_file = file;
  pASF->SetPhashCfg(&PHashCommCfg);

  _Stop();
  _Start();
}

PHashHandler::PHashHandler(std::vector<BYTE>* data, std::wstring sphash)
{
  m_sr = 8000;

  m_data = data;
  m_sphash = sphash;

  m_phash = NULL;
  m_phashlen = 0;
}

PHashHandler::~PHashHandler(void)
{

}

void PHashHandler::_Thread()
{
  if (!SamplesToPhash())
    return;

  if (!m_phashlen || !m_phash)
    return;

  void* context = zmq_init(1);
  if (!context)
  {
    ph_freemem_hash(NULL, m_phash);
    return;
  }

  void* client = socket_connect(context, ZMQ_REQ, "tcp://192.168.10.18:5000");
  if (!client)
  {
    ph_freemem_hash(NULL, m_phash);
    return;
  }

  std::string sphash = Strings::WStringToString(m_sphash);
  int sphashlen = sphash.size();
  sendmore_msg_data(client, &sphashlen, sizeof(int), NULL, NULL);
  sendmore_msg_data(client, &sphash[0], sphashlen, NULL, NULL);
  sendmore_msg_data(client, &m_phashlen, sizeof(int), NULL, NULL);
  send_msg_data(client, m_phash, m_phashlen*sizeof(uint32_t), NULL, NULL);

  void* data;
  size_t msgsize, moresize;
  int64_t more;
  recieve_msg(client, &msgsize, &more, &moresize, (void**)&data);
  
  ph_freemem_hash(NULL, m_phash);
  m_phash = NULL;
  m_phashlen = 0;

  zmq_close(client);
  zmq_term(context);
}

BOOL PHashHandler::SamplesToPhash()
{
  size_t buflen = m_data->size();
  float* buf = new float[buflen];
  Logging("~~~~~~~~~~~~~~~~~~ [phash] buffer length: %d", buflen);

  pHashController* phashctrl = pHashController::GetInstance();
  int samplebyte = (phashctrl->PHashCommCfg.format.wBitsPerSample >> 3);
  int nsample = buflen / phashctrl->PHashCommCfg.format.nChannels / samplebyte;
  int samples = nsample * phashctrl->PHashCommCfg.format.nChannels;
  
  BYTE* indata;
  BOOL ret;

  // Making all data into float type
  indata = &(*m_data)[0];
  ret = SampleToFloat(indata, buf, samples, phashctrl->PHashCommCfg.pcmtype);

  m_data->clear();
  m_data->resize(0);
  pHashController::GetInstance()->UnRefs();

  if (!ret)
  {
    delete [] buf;
    return ret;
  }

  // Mix as Mono channel
  int MonoLen = buflen / phashctrl->PHashCommCfg.format.nChannels;
  float* MonoChannelBuf = new float[MonoLen];
  ret = MixChannels(buf, samples, phashctrl->PHashCommCfg.format.nChannels, nsample, MonoChannelBuf);
  delete[] buf;

  if (!ret)
  {
    delete[] MonoChannelBuf;
    return ret;
  }

  // Samplerate to 8kHz 
  float* outbuff = NULL;
  int bufflen = 0;
  ret = DownSample(MonoChannelBuf, nsample, m_sr, phashctrl->PHashCommCfg.format.nSamplesPerSec, &outbuff, bufflen);
  delete[] MonoChannelBuf;

  if (!ret)
    return ret;


  int phashlen = 0;
  uint32_t* phash = NULL;

  m_phash = ph_audiohash(outbuff, bufflen, m_sr, m_phashlen);
  delete [] outbuff;

  if (!m_phash)
    m_phashlen = 0;

  return ret;
}

BOOL PHashHandler::DownSample(float* inbuf, int nsample, int des_sr, int org_sr, float** outbuf, int& outlen)
{
  // resample float array , set desired samplerate ratio
  double sr_ratio = (double)des_sr / (double)org_sr;
  if (!src_is_valid_ratio(sr_ratio))
    return FALSE;

  // allocate output buffer for conversion
  outlen = sr_ratio * (double)nsample;
  //*outbuf = (float*)malloc(outlen * sizeof(float)); 
  float* buffer = new float[outlen * sizeof(float)];
  if (!buffer)
    return FALSE;

  int error;
  SRC_STATE *src_state = src_new(SRC_LINEAR, 1, &error);
  if (!src_state)
    return FALSE;

  SRC_DATA src_data;
  src_data.data_in = inbuf;
  src_data.data_out = buffer;
  src_data.input_frames = nsample;
  src_data.output_frames = outlen;
  src_data.end_of_input = SF_TRUE;
  src_data.src_ratio = sr_ratio;

  // Sample rate conversion
  if (src_process(src_state, &src_data))
  {
    delete [] buffer;
    src_delete(src_state);
    return FALSE;
  }

  src_delete(src_state);
  *outbuf = buffer;
  return TRUE;
}

BOOL PHashHandler::MixChannels(float* buf, int samples, int channels, int nsample, float* MonoChannelBuf)
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
    } while (bufindx < nsample);
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

void PHashHandler::SixchannelsToStereo(float* output, float* input, int n)
{
  float *p = input;

  while(n-- > 0)
  {
    *output++ = (p[0] + p[1] + p[3] + p[5]) / 3;
    *output++ = (p[1] + p[2] + p[4] + p[5]) / 3;
    p += 6;
  }
}

BOOL PHashHandler::SampleToFloat(const unsigned char* const indata, float* outdata, int samples, int type)
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