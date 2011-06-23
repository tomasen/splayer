// ----------------------------------------------------------------------------
// WavPack lib for Matroska
// ----------------------------------------------------------------------------
// Copyright christophe.paris@free.fr
// Parts by David Bryant http://www.wavpack.com
// Distributed under the BSD Software License
// ----------------------------------------------------------------------------

#include "..\wavpack\wputils.h"
#include "wavpack_common.h"
#include "wavpack_buffer_decoder.h"

// ----------------------------------------------------------------------------

void wavpack_buffer_format_samples(wavpack_buffer_decoder* wbd,
                                   uchar *dst, long *src, uint32_t samples)
{
    long temp;
    int bps = WavpackGetBytesPerSample(wbd->wpc);
    uint32_t samcnt = samples * WavpackGetNumChannels (wbd->wpc);

    switch (bps) {
        
    case 1:
        while (samcnt--)
            *dst++ = (uchar)(*src++ + 128);
        
        break;
        
    case 2:
        while (samcnt--) {
            *dst++ = (uchar)(temp = *src++);
            *dst++ = (uchar)(temp >> 8);
        }
        
        break;
        
    case 3:
        while (samcnt--) {
            *dst++ = (uchar)(temp = *src++);
            *dst++ = (uchar)(temp >> 8);
            *dst++ = (uchar)(temp >> 16);
        }
        
        break;
    }
}

// ============================================================================

int32_t frame_stream_reader_read_bytes(void *id, void *data, int32_t bcount)
{
    uint32_t bytes_left = 0;
    uint32_t bytes_to_read = 0;
    frame_stream_reader* fsr = (frame_stream_reader*)id;
    if(fsr->buffer == NULL)
    {
        return 0;
    }
    bytes_left = (fsr->length - fsr->position);
    bytes_to_read = min(bytes_left, (uint32_t)bcount);
    
    if(bytes_to_read > 0)
    {    
        wp_memcpy(data, fsr->buffer + fsr->position, bytes_to_read);
        fsr->position += bytes_to_read;
    }

    return bytes_to_read;
}

// ----------------------------------------------------------------------------

uint32_t frame_stream_reader_get_pos(void *id)
{
    frame_stream_reader* fsr = (frame_stream_reader*)id;
    return fsr->position;
}

// ----------------------------------------------------------------------------

int frame_stream_reader_set_pos_abs(void *id, uint32_t pos)
{
    frame_stream_reader* fsr = (frame_stream_reader*)id;
    fsr->position = min(pos, fsr->length);
    if(pos > fsr->length)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

// ----------------------------------------------------------------------------

int frame_stream_reader_set_pos_rel(void *id, int32_t delta, int mode)
{
    frame_stream_reader* fsr = (frame_stream_reader*)id;
    uint32_t new_pos = 0;
    switch(mode)
    {
        case SEEK_SET:
            new_pos = delta;            
            break;      
        case SEEK_CUR:
            new_pos = fsr->position + delta;
            break;
        case SEEK_END:
            new_pos = fsr->length + delta;
            break;
    }
    fsr->position = constrain(0, new_pos, fsr->length);

    if((new_pos < 0) || (new_pos > fsr->length))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

// ----------------------------------------------------------------------------

int frame_stream_reader_push_back_byte(void *id, int c)
{
    frame_stream_reader* fsr = (frame_stream_reader*)id;
    fsr->position = constrain(0, fsr->position - 1, fsr->length);
    return fsr->position;
}

// ----------------------------------------------------------------------------

uint32_t frame_stream_reader_get_length(void *id)
{
    frame_stream_reader* fsr = (frame_stream_reader*)id;
    return fsr->length;
}

// ----------------------------------------------------------------------------

int frame_stream_reader_can_seek(void *id)
{
    return 0;
}

// ----------------------------------------------------------------------------

frame_stream_reader* frame_stream_reader_new()
{
    frame_stream_reader* fsr = wp_alloc(sizeof(frame_stream_reader));
    if(!fsr)
    {
        return NULL;
    }
    
    fsr->sr.read_bytes = frame_stream_reader_read_bytes;
    fsr->sr.get_pos = frame_stream_reader_get_pos;
    fsr->sr.set_pos_abs = frame_stream_reader_set_pos_abs;
    fsr->sr.set_pos_rel = frame_stream_reader_set_pos_rel;
    fsr->sr.push_back_byte = frame_stream_reader_push_back_byte;
    fsr->sr.get_length = frame_stream_reader_get_length;
    fsr->sr.can_seek = frame_stream_reader_can_seek;

    fsr->buffer = 0;
    fsr->position = 0;

    return fsr;
}

// ----------------------------------------------------------------------------

void frame_stream_reader_free(frame_stream_reader* fsr)
{
    if(fsr != NULL)
    {
        wp_free(fsr);
    }
}

// ----------------------------------------------------------------------------

void frame_stream_reader_set_buffer(void *id, char* buffer, int length)
{
    frame_stream_reader* fsr = (frame_stream_reader*)id;
    fsr->buffer = buffer;
    fsr->length = length;
    fsr->position = 0;
}

// ============================================================================

wavpack_buffer_decoder* wavpack_buffer_decoder_new()
{
    wavpack_buffer_decoder* wbd = wp_alloc(sizeof(wavpack_buffer_decoder));
    if(!wbd)
    {
        return NULL;
    }

    wbd->fsr = frame_stream_reader_new();
    if(!wbd->fsr)
    {
        return NULL;
    }

    wbd->fsrc = frame_stream_reader_new();
    if(!wbd->fsrc)
    {
        return NULL;
    }

    wbd->wpc = NULL;

    return wbd;
}

// ----------------------------------------------------------------------------

int wavpack_buffer_decoder_load_frame(wavpack_buffer_decoder* wbd,
                                      char* data, int length,
                                      char* correction_data, int cd_length)
{
    frame_stream_reader_set_buffer(wbd->fsr, data, length);
    
    // Make sure to pass NULL is there is no correction data
    if(correction_data != 0)
    {
        frame_stream_reader_set_buffer(wbd->fsrc, correction_data, cd_length);
    }

    if(wbd->wpc == NULL)
    {
        wbd->wpc = WavpackOpenFileInputEx(
            (stream_reader*)wbd->fsr,
            wbd->fsr,
            (correction_data != NULL) ? wbd->fsrc : NULL,
            wbd->wavpack_error_msg, OPEN_STREAMING|OPEN_NORMALIZE, 0);
    }

    return (wbd->wpc != NULL);
}

// ----------------------------------------------------------------------------

uint32_t wavpack_buffer_decoder_unpack(wavpack_buffer_decoder* wbd,
                                       int32_t* buffer, uint32_t samples)
{
    return WavpackUnpackSamples (wbd->wpc, buffer, samples);
}

// ----------------------------------------------------------------------------

void wavpack_buffer_decoder_free(wavpack_buffer_decoder* wbd)
{
    if(wbd != NULL)
    {   
        frame_stream_reader_free(wbd->fsr);
        wbd->fsr = NULL;
        frame_stream_reader_free(wbd->fsrc);
        wbd->fsrc = NULL;
        if(wbd->wpc != NULL)
        {
            WavpackCloseFile(wbd->wpc);
            wbd->wpc = NULL;
        }
        wp_free(wbd);
    }
}

// ============================================================================
