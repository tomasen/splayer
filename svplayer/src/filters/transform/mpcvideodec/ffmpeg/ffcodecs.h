#ifndef _FFCODECS_H_
#define _FFCODECS_H_

#define FFDSHOW_CODECS \
 CODEC_OP(CODEC_ID_COREPNG           ,38,"corepng") \
\
 /* Raw formats */ \
 CODEC_OP(CODEC_ID_RAW           ,300,"raw") \
 CODEC_OP(CODEC_ID_YUY2          ,301,"raw") \
 CODEC_OP(CODEC_ID_RGB2          ,302,"raw") \
 CODEC_OP(CODEC_ID_RGB3          ,303,"raw") \
 CODEC_OP(CODEC_ID_RGB5          ,304,"raw") \
 CODEC_OP(CODEC_ID_RGB6          ,305,"raw") \
 CODEC_OP(CODEC_ID_BGR2          ,306,"raw") \
 CODEC_OP(CODEC_ID_BGR3          ,307,"raw") \
 CODEC_OP(CODEC_ID_BGR5          ,308,"raw") \
 CODEC_OP(CODEC_ID_BGR6          ,309,"raw") \
 CODEC_OP(CODEC_ID_YV12          ,310,"raw") \
 CODEC_OP(CODEC_ID_YVYU          ,311,"raw") \
 CODEC_OP(CODEC_ID_UYVY          ,312,"raw") \
 CODEC_OP(CODEC_ID_VYUY          ,313,"raw") \
 CODEC_OP(CODEC_ID_I420          ,314,"raw") \
 CODEC_OP(CODEC_ID_Y800          ,316,"raw") \
 CODEC_OP(CODEC_ID_444P          ,317,"raw") \
 CODEC_OP(CODEC_ID_422P          ,318,"raw") \
 CODEC_OP(CODEC_ID_411P          ,319,"raw") \
 CODEC_OP(CODEC_ID_410P          ,320,"raw") \
 CODEC_OP(CODEC_ID_NV12          ,321,"raw") \
 CODEC_OP(CODEC_ID_NV21          ,322,"raw") \
 CODEC_OP(CODEC_ID_PAL1          ,323,"raw") \
 CODEC_OP(CODEC_ID_PAL4          ,324,"raw") \
 CODEC_OP(CODEC_ID_PAL8          ,325,"raw") \
 CODEC_OP(CODEC_ID_LPCM          ,398,"raw") \
 CODEC_OP(CODEC_ID_PCM           ,399,"raw") \
 \
 CODEC_OP(CODEC_ID_XVID4         ,400,"xvid") \
 \
 CODEC_OP(CODEC_ID_LIBMPEG2      ,500,"libmpeg2") \
 \
 CODEC_OP(CODEC_ID_THEORA_LIB    ,600,"libtheora") \
 \
 CODEC_OP(CODEC_ID_MP3LIB        ,700,"mp3lib") \
 \
 CODEC_OP(CODEC_ID_LIBMAD        ,800,"libmad") \
 \
 CODEC_OP(CODEC_ID_LIBFAAD       ,900,"faad2") \
 \
 CODEC_OP(CODEC_ID_WMV9_LIB      ,1000,"wmv9codec") \
 \
 CODEC_OP(CODEC_ID_AVISYNTH      ,1100,"avisynth") \
 \
 CODEC_OP(CODEC_ID_SKAL          ,1200,"skal's") \
 \
 CODEC_OP(CODEC_ID_LIBA52        ,1300,"liba52") \
 \
 CODEC_OP(CODEC_ID_SPDIF_AC3     ,1400,"s/pdif") \
 CODEC_OP(CODEC_ID_SPDIF_DTS     ,1401,"s/pdif") \
 \
 CODEC_OP(CODEC_ID_LIBDTS        ,1500,"libdts") \
 \
 CODEC_OP(CODEC_ID_TREMOR        ,1600,"tremor") \
 \
 CODEC_OP(CODEC_ID_REALAAC       ,1700,"realaac") \
 \
 CODEC_OP(CODEC_ID_AUDX          ,1800,"Aud-X") \
 \
 CODEC_OP(CODEC_ID_X264          ,1900,"x264") \
 CODEC_OP(CODEC_ID_X264_LOSSLESS ,1901,"x264 lossless") \

enum CodecID
{
 CODEC_ID_UNSUPPORTED=-1, 
 CODEC_ID_NONE=0,

 /* video codecs */
 CODEC_ID_MPEG1VIDEO,
 CODEC_ID_MPEG2VIDEO, ///< preferred ID for MPEG-1/2 video decoding
 CODEC_ID_MPEG2VIDEO_XVMC,
 CODEC_ID_H261,
 CODEC_ID_H263,
 CODEC_ID_RV10,
 CODEC_ID_RV20,
 CODEC_ID_MJPEG,
 CODEC_ID_MJPEGB,
 CODEC_ID_LJPEG,
 CODEC_ID_SP5X,
 CODEC_ID_JPEGLS,
 CODEC_ID_MPEG4,
 CODEC_ID_RAWVIDEO,
 CODEC_ID_MSMPEG4V1,
 CODEC_ID_MSMPEG4V2,
 CODEC_ID_MSMPEG4V3,
 CODEC_ID_WMV1,
 CODEC_ID_WMV2,
 CODEC_ID_H263P,
 CODEC_ID_H263I,
 CODEC_ID_FLV1,
 CODEC_ID_SVQ1,
 CODEC_ID_SVQ3,
 CODEC_ID_DVVIDEO,
 CODEC_ID_HUFFYUV,
 CODEC_ID_CYUV,
 CODEC_ID_H264,
 CODEC_ID_INDEO3,
 CODEC_ID_VP3,
 CODEC_ID_THEORA,
 CODEC_ID_ASV1,
 CODEC_ID_ASV2,
 CODEC_ID_FFV1,
 CODEC_ID_4XM,
 CODEC_ID_VCR1,
 CODEC_ID_CLJR,
 CODEC_ID_MDEC,
 CODEC_ID_ROQ,
 CODEC_ID_INTERPLAY_VIDEO,
 CODEC_ID_XAN_WC3,
 CODEC_ID_XAN_WC4,
 CODEC_ID_RPZA,
 CODEC_ID_CINEPAK,
 CODEC_ID_WS_VQA,
 CODEC_ID_MSRLE,
 CODEC_ID_MSVIDEO1,
 CODEC_ID_IDCIN,
 CODEC_ID_8BPS,
 CODEC_ID_SMC,
 CODEC_ID_FLIC,
 CODEC_ID_TRUEMOTION1,
 CODEC_ID_VMDVIDEO,
 CODEC_ID_MSZH,
 CODEC_ID_ZLIB,
 CODEC_ID_QTRLE,
 CODEC_ID_SNOW,
 CODEC_ID_TSCC,
 CODEC_ID_ULTI,
 CODEC_ID_QDRAW,
 CODEC_ID_VIXL,
 CODEC_ID_QPEG,
 CODEC_ID_XVID,
 CODEC_ID_PNG,
 CODEC_ID_PPM,
 CODEC_ID_PBM,
 CODEC_ID_PGM,
 CODEC_ID_PGMYUV,
 CODEC_ID_PAM,
 CODEC_ID_FFVHUFF,
 CODEC_ID_RV30,
 CODEC_ID_RV40,
 CODEC_ID_VC1,
 CODEC_ID_WMV3,
 CODEC_ID_LOCO,
 CODEC_ID_WNV1,
 CODEC_ID_AASC,
 CODEC_ID_INDEO2,
 CODEC_ID_FRAPS,
 CODEC_ID_TRUEMOTION2,
 CODEC_ID_BMP,
 CODEC_ID_CSCD,
 CODEC_ID_MMVIDEO,
 CODEC_ID_ZMBV,
 CODEC_ID_AVS,
 CODEC_ID_SMACKVIDEO,
 CODEC_ID_NUV,
 CODEC_ID_KMVC,
 CODEC_ID_FLASHSV,
 CODEC_ID_CAVS,
 CODEC_ID_JPEG2000,
 CODEC_ID_VMNC,
 CODEC_ID_VP5,
 CODEC_ID_VP6,
 CODEC_ID_VP6F,
 CODEC_ID_TARGA,
 CODEC_ID_DSICINVIDEO,
 CODEC_ID_TIERTEXSEQVIDEO,
 CODEC_ID_TIFF,
 CODEC_ID_GIF,
 CODEC_ID_FFH264,
 CODEC_ID_DXA,
 CODEC_ID_DNXHD,
 CODEC_ID_THP,
 CODEC_ID_SGI,
 CODEC_ID_C93,
 CODEC_ID_BETHSOFTVID,
 CODEC_ID_PTX,
 CODEC_ID_TXD,
 CODEC_ID_VP6A,
 CODEC_ID_AMV,
 CODEC_ID_VB,
 CODEC_ID_PCX,
 CODEC_ID_SUNRAST,
 CODEC_ID_INDEO4,
 CODEC_ID_INDEO5,
 CODEC_ID_MIMIC,
 CODEC_ID_RL2,
 CODEC_ID_8SVX_EXP,
 CODEC_ID_8SVX_FIB,
 CODEC_ID_ESCAPE124,
 CODEC_ID_DIRAC,
 CODEC_ID_BFI,
 CODEC_ID_CMV,
 CODEC_ID_MOTIONPIXELS,
 CODEC_ID_TGV,
 CODEC_ID_TGQ,
 CODEC_ID_TQI,
 CODEC_ID_AURA,
 CODEC_ID_AURA2,
 CODEC_ID_V210X,
 CODEC_ID_TMV,
 CODEC_ID_V210,
 CODEC_ID_DPX,
 CODEC_ID_MAD,
 CODEC_ID_FRWU,

 /* various PCM "codecs" */
 CODEC_ID_PCM_S16LE= 0x10000,
 CODEC_ID_PCM_S16BE,
 CODEC_ID_PCM_U16LE,
 CODEC_ID_PCM_U16BE,
 CODEC_ID_PCM_S8,
 CODEC_ID_PCM_U8,
 CODEC_ID_PCM_MULAW,
 CODEC_ID_PCM_ALAW,
 CODEC_ID_PCM_S32LE,
 CODEC_ID_PCM_S32BE,
 CODEC_ID_PCM_U32LE,
 CODEC_ID_PCM_U32BE,
 CODEC_ID_PCM_S24LE,
 CODEC_ID_PCM_S24BE,
 CODEC_ID_PCM_U24LE,
 CODEC_ID_PCM_U24BE,
 CODEC_ID_PCM_S24DAUD,
 CODEC_ID_PCM_ZORK,
 CODEC_ID_PCM_S16LE_PLANAR,
 CODEC_ID_PCM_DVD,
 CODEC_ID_PCM_F32BE,
 CODEC_ID_PCM_F32LE,
 CODEC_ID_PCM_F64BE,
 CODEC_ID_PCM_F64LE,
 CODEC_ID_PCM_BLURAY,

 /* various ADPCM codecs */
 CODEC_ID_ADPCM_IMA_QT= 0x11000,
 CODEC_ID_ADPCM_IMA_WAV,
 CODEC_ID_ADPCM_IMA_DK3,
 CODEC_ID_ADPCM_IMA_DK4,
 CODEC_ID_ADPCM_IMA_WS,
 CODEC_ID_ADPCM_IMA_SMJPEG,
 CODEC_ID_ADPCM_MS,
 CODEC_ID_ADPCM_4XM,
 CODEC_ID_ADPCM_XA,
 CODEC_ID_ADPCM_ADX,
 CODEC_ID_ADPCM_EA,
 CODEC_ID_ADPCM_G726,
 CODEC_ID_ADPCM_CT,
 CODEC_ID_ADPCM_SWF,
 CODEC_ID_ADPCM_YAMAHA,
 CODEC_ID_ADPCM_SBPRO_4,
 CODEC_ID_ADPCM_SBPRO_3,
 CODEC_ID_ADPCM_SBPRO_2,
 CODEC_ID_ADPCM_THP,
 CODEC_ID_ADPCM_IMA_AMV,
 CODEC_ID_ADPCM_EA_R1,
 CODEC_ID_ADPCM_EA_R3,
 CODEC_ID_ADPCM_EA_R2,
 CODEC_ID_ADPCM_IMA_EA_SEAD,
 CODEC_ID_ADPCM_IMA_EA_EACS,
 CODEC_ID_ADPCM_EA_XAS,
 CODEC_ID_ADPCM_EA_MAXIS_XA,
 CODEC_ID_ADPCM_IMA_ISS,

 /* AMR */
 CODEC_ID_AMR_NB= 0x12000,
 CODEC_ID_AMR_WB,

 /* RealAudio codecs*/
 CODEC_ID_RA_144= 0x13000,
 CODEC_ID_RA_288,

 /* various DPCM codecs */
 CODEC_ID_ROQ_DPCM= 0x14000,
 CODEC_ID_INTERPLAY_DPCM,
 CODEC_ID_XAN_DPCM,
 CODEC_ID_SOL_DPCM,

 /* audio codecs */
 CODEC_ID_MP2= 0x15000,
 CODEC_ID_MP3, ///< preferred ID for decoding MPEG audio layer 1, 2 or 3
 CODEC_ID_AAC,
 CODEC_ID_AC3,
 CODEC_ID_DTS,
 CODEC_ID_VORBIS,
 CODEC_ID_DVAUDIO,
 CODEC_ID_WMAV1,
 CODEC_ID_WMAV2,
 CODEC_ID_MACE3,
 CODEC_ID_MACE6,
 CODEC_ID_VMDAUDIO,
 CODEC_ID_SONIC,
 CODEC_ID_SONIC_LS,
 CODEC_ID_FLAC,
 CODEC_ID_MP3ADU,
 CODEC_ID_MP3ON4,
 CODEC_ID_SHORTEN,
 CODEC_ID_ALAC,
 CODEC_ID_WESTWOOD_SND1,
 CODEC_ID_GSM, ///< as in Berlin toast format
 CODEC_ID_QDM2,
 CODEC_ID_COOK,
 CODEC_ID_TRUESPEECH,
 CODEC_ID_TTA,
 CODEC_ID_SMACKAUDIO,
 CODEC_ID_QCELP,
 CODEC_ID_WAVPACK,
 CODEC_ID_DSICINAUDIO,
 CODEC_ID_IMC,
 CODEC_ID_MUSEPACK7,
 CODEC_ID_MLP,
 CODEC_ID_GSM_MS, /* as found in WAV */
 CODEC_ID_ATRAC3,
 CODEC_ID_VOXWARE,
 CODEC_ID_APE,
 CODEC_ID_NELLYMOSER,
 CODEC_ID_MUSEPACK8,
 CODEC_ID_SPEEX,
 CODEC_ID_WMAVOICE,
 CODEC_ID_WMAPRO,
 CODEC_ID_WMALOSSLESS,
 CODEC_ID_ATRAC3P,
 CODEC_ID_EAC3,
 CODEC_ID_SIPR,
 CODEC_ID_MP1,
 CODEC_ID_TWINVQ,
 CODEC_ID_TRUEHD,
 CODEC_ID_MP4ALS,
 CODEC_ID_ATRAC1,

 /* subtitle codecs */
 CODEC_ID_DVD_SUBTITLE= 0x17000,
 CODEC_ID_DVB_SUBTITLE,
 CODEC_ID_TEXT,  ///< raw UTF-8 text
 CODEC_ID_XSUB,
 CODEC_ID_SSA,
 CODEC_ID_MOV_TEXT,
 CODEC_ID_HDMV_PGS_SUBTITLE,
 CODEC_ID_DVB_TELETEXT,




#define CODEC_OP(codecEnum,codecId,codecName)  codecEnum,
 FFDSHOW_CODECS
#undef CODEC_OP

 /* other specific kind of codecs (generally used for attachments) */
 CODEC_ID_TTF= 0x18000,

 CODEC_ID_PROBE= 0x19000, ///< codec_id is not known (like CODEC_ID_NONE) but lavf should attempt to identify it

 CODEC_ID_MPEG2TS= 0x20000, /**< _FAKE_ codec to indicate a raw MPEG-2 TS
							* stream (only used by libavformat) */
};

#ifdef __cplusplus

template<> struct isPOD<CodecID> {enum {is=true};};

const FOURCC* getCodecFOURCCs(CodecID codecId);
//const char_t* getCodecName(CodecID codecId);

static __inline bool lavc_codec(int x)     {return x>0 && x<200;}
static __inline bool raw_codec(int x)      {return x>=300 && x<400;}
static __inline bool xvid_codec(int x)     {return x==CODEC_ID_XVID4;}
static __inline bool theora_codec(int x)   {return x==CODEC_ID_THEORA_LIB;}
static __inline bool mplayer_codec(int x)  {return x==CODEC_ID_MP3LIB;}
static __inline bool wmv9_codec(int x)     {return x>=1000 && x<1100;}
static __inline bool mpeg12_codec(int x)   {return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_LIBMPEG2;}
static __inline bool mpeg1_codec(int x)    {return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_LIBMPEG2;}
static __inline bool mpeg2_codec(int x)    {return x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_LIBMPEG2;}
static __inline bool mpeg4_codec(int x)    {return x==CODEC_ID_MPEG4 || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool spdif_codec(int x)    {return x==CODEC_ID_SPDIF_AC3 || x==CODEC_ID_SPDIF_DTS;}
static __inline bool huffyuv_codec(int x)  {return x==CODEC_ID_HUFFYUV || x==CODEC_ID_FFVHUFF;}
static __inline bool x264_codec(int x)     {return x==CODEC_ID_X264 || x==CODEC_ID_X264_LOSSLESS;}
static __inline bool lossless_codec(int x) {return huffyuv_codec(x) || x==CODEC_ID_LJPEG || x==CODEC_ID_FFV1 || x==CODEC_ID_DVVIDEO || x==CODEC_ID_X264_LOSSLESS;}

//I'm not sure of all these
static __inline bool sup_CBR(int x)           {return !lossless_codec(x) && !raw_codec(x);}
static __inline bool sup_VBR_QUAL(int x)      {return !lossless_codec(x) && !raw_codec(x) && x!=CODEC_ID_SKAL;}
static __inline bool sup_VBR_QUANT(int x)     {return (lavc_codec(x) || xvid_codec(x) || theora_codec(x) || x==CODEC_ID_SKAL || x==CODEC_ID_X264) && !lossless_codec(x) && x!=CODEC_ID_SNOW;}
static __inline bool sup_XVID2PASS(int x)     {return sup_VBR_QUANT(x) && x!=CODEC_ID_X264 && x!=CODEC_ID_SNOW;}
static __inline bool sup_LAVC2PASS(int x)     {return (lavc_codec(x) && !lossless_codec(x) && x!=CODEC_ID_MJPEG && !raw_codec(x)) || x==CODEC_ID_X264;}

static __inline bool sup_interlace(int x)         {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_gray(int x)              {return x!=CODEC_ID_LJPEG && x!=CODEC_ID_FFV1 && x!=CODEC_ID_SNOW && !theora_codec(x) && !wmv9_codec(x) && !raw_codec(x) && x!=CODEC_ID_SKAL && x!=CODEC_ID_DVVIDEO && !x264_codec(x);}
static __inline bool sup_globalheader(int x)      {return x==CODEC_ID_MPEG4;}
static __inline bool sup_part(int x)              {return x==CODEC_ID_MPEG4;}
static __inline bool sup_packedBitstream(int x)   {return xvid_codec(x);}
static __inline bool sup_minKeySet(int x)         {return x!=CODEC_ID_MJPEG && x!=CODEC_ID_SNOW && (!lossless_codec(x) || x==CODEC_ID_X264_LOSSLESS) && !wmv9_codec(x) && !raw_codec(x);}
static __inline bool sup_maxKeySet(int x)         {return x!=CODEC_ID_MJPEG && (!lossless_codec(x) || x==CODEC_ID_X264_LOSSLESS) && !raw_codec(x);}
static __inline bool sup_bframes(int x)           {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x) || x264_codec(x);}
static __inline bool sup_adaptiveBframes(int x)   {return lavc_codec(x) || x==CODEC_ID_X264;}
static __inline bool sup_closedGop(int x)         {return sup_bframes(x) && !x264_codec(x);}
static __inline bool sup_lavcme(int x)            {return lavc_codec(x) && x!=CODEC_ID_MJPEG && !lossless_codec(x);}
static __inline bool sup_quantProps(int x)        {return !lossless_codec(x) && !theora_codec(x) && !wmv9_codec(x) && !raw_codec(x) && x!=CODEC_ID_SNOW;}
static __inline bool sup_trellisQuant(int x)      {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_XVID4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P || x==CODEC_ID_SKAL || x==CODEC_ID_X264;}
static __inline bool sup_masking(int x)           {return x==CODEC_ID_MPEG4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x) || x==CODEC_ID_SKAL || x==CODEC_ID_X264;}
static __inline bool sup_lavcOnePass(int x)       {return (lavc_codec(x) && !lossless_codec(x)) || x==CODEC_ID_X264;}
static __inline bool sup_perFrameQuant(int x)     {return !lossless_codec(x) && !wmv9_codec(x) && !raw_codec(x) && !x264_codec(x) && x!=CODEC_ID_SNOW;}
static __inline bool sup_4mv(int x)               {return x==CODEC_ID_MPEG4 || x==CODEC_ID_H263 || x==CODEC_ID_H263P || x==CODEC_ID_SNOW || x==CODEC_ID_SKAL;}
static __inline bool sup_aspect(int x)            {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_XVID4 || x==CODEC_ID_THEORA_LIB || x264_codec(x);}
static __inline bool sup_PSNR(int x)              {return (lavc_codec(x) && !lossless_codec(x)) || xvid_codec(x) || x==CODEC_ID_SKAL || x264_codec(x);}
static __inline bool sup_quantBias(int x)         {return lavc_codec(x) && !lossless_codec(x);}
static __inline bool sup_MPEGquant(int x)         {return x==CODEC_ID_MPEG4 || x==CODEC_ID_MSMPEG4V3 || x==CODEC_ID_MPEG2VIDEO || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_lavcQuant(int x)         {return lavc_codec(x) && sup_quantProps(x);}
static __inline bool sup_customQuantTables(int x) {return x==CODEC_ID_MPEG4 || xvid_codec(x) || x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_SKAL || x==CODEC_ID_X264;}
static __inline bool sup_qpel(int x)              {return x==CODEC_ID_MPEG4 || x==CODEC_ID_SNOW || xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_gmc(int x)               {return xvid_codec(x) || x==CODEC_ID_SKAL;}
static __inline bool sup_me_mv0(int x)            {return sup_lavcme(x) && x!=CODEC_ID_SNOW;}
static __inline bool sup_cbp_rd(int x)            {return x==CODEC_ID_MPEG4;}
static __inline bool sup_qns(int x)               {return lavc_codec(x) && sup_quantProps(x) && x!=CODEC_ID_MSMPEG4V3 && x!=CODEC_ID_MSMPEG4V2 && x!=CODEC_ID_MSMPEG4V1 && x!=CODEC_ID_WMV1 && x!=CODEC_ID_WMV2 && x!=CODEC_ID_MJPEG && x!=CODEC_ID_SNOW;}
static __inline bool sup_threads(int x)           {return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_MPEG4 || x==CODEC_ID_XVID4 || x264_codec(x);}
static __inline bool sup_threads_dec(int x)       {return x==CODEC_ID_MPEG1VIDEO || x==CODEC_ID_MPEG2VIDEO || x==CODEC_ID_H264;}
static __inline bool sup_palette(int x)           {return x==CODEC_ID_MSVIDEO1 || x==CODEC_ID_8BPS || x==CODEC_ID_QTRLE || x==CODEC_ID_TSCC || x==CODEC_ID_QPEG || x==CODEC_ID_PNG;}

#endif

#endif
