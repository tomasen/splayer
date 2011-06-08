// ----------------------------------------------------------------------------
// WavPack lib for Matroska
// ----------------------------------------------------------------------------
// Copyright christophe.paris@free.fr
// Parts by David Bryant http://www.wavpack.com
// Distributed under the BSD Software License
// ----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
#ifndef WAVPACK_BUFFER_DECODER_H_
#define WAVPACK_BUFFER_DECODER_H_
//-----------------------------------------------------------------------------

typedef struct {
    stream_reader sr;
    
    char* buffer;
    uint32_t position;
    uint32_t length;
} frame_stream_reader;

typedef struct {
    frame_stream_reader* fsr;
    frame_stream_reader* fsrc;
    WavpackContext *wpc;
    char wavpack_error_msg[512];
} wavpack_buffer_decoder;

//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
    
// ----------------------------------------------------------------------------

wavpack_buffer_decoder* wavpack_buffer_decoder_new();

int wavpack_buffer_decoder_load_frame(wavpack_buffer_decoder* wbd,
                                      char* data, int length,
                                      char* correction_data, int cd_length);

uint32_t wavpack_buffer_decoder_unpack(wavpack_buffer_decoder* wbd,
                                       int32_t* buffer, uint32_t samples);

void wavpack_buffer_decoder_free(wavpack_buffer_decoder* wbd);

void wavpack_buffer_format_samples(wavpack_buffer_decoder* wbd,
                                   uchar *dst, long *src, uint32_t samples);


// ----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif  

//-----------------------------------------------------------------------------
#endif // WAVPACK_BUFFER_DECODER_H_
//-----------------------------------------------------------------------------