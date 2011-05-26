// ----------------------------------------------------------------------------
// WavPack lib for Matroska
// ----------------------------------------------------------------------------
// Copyright christophe.paris@free.fr
// Parts by David Bryant http://www.wavpack.com
// Distributed under the BSD Software License
// ----------------------------------------------------------------------------

#include "..\wavpack\wputils.h"
#include "wavpack_common.h"
#include "wavpack_frame.h"

// ----------------------------------------------------------------------------

frame_buffer* frame_buffer_new()
{
    frame_buffer* fb = wp_alloc(sizeof(frame_buffer));
    if(!fb)
    {
        return NULL;
    }

    frame_reserve_space(fb, 1024);

    return fb;
}

// ----------------------------------------------------------------------------

void frame_buffer_free(frame_buffer* fb)
{
    if(fb->data != NULL)
    {
        wp_free(fb->data);
        fb->data = NULL;
    }
    wp_free(fb);
}

// ----------------------------------------------------------------------------

int frame_reserve_space(frame_buffer* dst, int len)
{
    if(dst->data == 0)
    {
        dst->data = wp_alloc(len);
        dst->total_len = len;
    }
    else if(dst->len + len > dst->total_len)
    {
        dst->data = wp_realloc(dst->data, dst->len + len);        
        dst->total_len = dst->len + len;
    }

    return (dst->data != NULL);
}

// ----------------------------------------------------------------------------

int frame_append_data(frame_buffer* dst, char* src, int len)
{
    if(!frame_reserve_space(dst, len))
    {
        return -1;
    }

    wp_memcpy(dst->data + dst->pos, src, len);
    dst->pos += len;
    dst->len += len;

    return 0;
}

// ----------------------------------------------------------------------------

int frame_append_data2(frame_buffer* dst, stream_reader *io, int len)
{
    frame_reserve_space(dst, len);
    
    if(io->read_bytes(io, (dst->data + dst->pos), len) != len)
    {
        return -1;
    }    

    dst->pos += len;
    dst->len += len;

    return 0;
}

// ----------------------------------------------------------------------------

void frame_reset(frame_buffer* dst)
{
  dst->pos = 0;
  dst->len = 0;
  dst->nb_block = 0;
}

// ----------------------------------------------------------------------------

int reconstruct_wavpack_frame(
    frame_buffer *frame,
    common_frame_data *common_data,
    char *pSrc,
    uint32_t SrcLength,
    int is_main_frame,
    int several_blocks,
    int version)
{
    WavpackHeader header;
    int i = 0;
    uint32_t stripped_header_len = 0;
    uint32_t current_flag = 0;
    uint32_t current_crc = 0;
    uint32_t block_size = 0;
    char *pBlock = pSrc;

    frame_reset(frame);

    if(is_main_frame == TRUE)
    {
      wp_memclear(common_data, sizeof(common_frame_data));
    }

    while(pBlock < pSrc + SrcLength)
    {
        i = 0;
        // Special processing for main frame
        if(is_main_frame == TRUE)
        {
            // First block ?
            if(pBlock == pSrc)
            {
                common_data->block_samples = ((uint32_t*)pBlock)[i++];
            }
            current_flag = ((uint32_t*)pBlock)[i++];
            // Save main frame flags, we will use them for correction data
            common_data->array_flags[frame->nb_block] = current_flag;
        }
        else
        {
          // Reuse main frame block_samples and flags
          current_flag = common_data->array_flags[frame->nb_block];
        }

        current_crc = ((uint32_t*)pBlock)[i++];

        if(several_blocks == TRUE)
        {
            block_size = ((uint32_t*)pBlock)[i++];
        }
        else
        {
            block_size = SrcLength - (i * sizeof(uint32_t));
        }
        stripped_header_len = (i * sizeof(uint32_t));

        wp_memclear(&header, sizeof(WavpackHeader));
        wp_memcpy(header.ckID, "wvpk", 4);
        header.ckSize = block_size + sizeof(WavpackHeader) - 8;
        header.version = version;
        header.total_samples = -1;
        header.block_samples = common_data->block_samples;
        header.flags = current_flag;
        header.crc = current_crc;

        frame_append_data(frame, (char*)&header, sizeof(WavpackHeader));
        frame_append_data(frame, pBlock + stripped_header_len, block_size);

        frame->nb_block++;

        pBlock += (stripped_header_len + block_size);
    }

    return frame->len;
}

// ----------------------------------------------------------------------------

int strip_wavpack_block(frame_buffer *frame,
                        WavpackHeader *wphdr,
                        stream_reader *io,
                        uint32_t block_data_size,
                        int is_main_frame,
                        int several_blocks)
{
    char* pDst = NULL;
    uint32_t stripped_header_len = 0;
    uint32_t total_len = 0;

    if(is_main_frame == TRUE)
    {
        if(frame->nb_block == 0)
        {
            frame_append_data(frame, (char*)&wphdr->block_samples, sizeof(wphdr->block_samples));
        }        
        frame_append_data(frame, (char*)&wphdr->flags, sizeof(wphdr->flags));
    }
    frame_append_data(frame, (char*)&wphdr->crc, sizeof(wphdr->crc));

    if(several_blocks == TRUE)
    {
        frame_append_data(frame, (char*)&block_data_size, sizeof(block_data_size));
    }
    
    stripped_header_len = frame->len;

    if(frame_append_data2(frame, io, block_data_size) == -1)
    {
        return -1;
    }

    frame->nb_block++;
        
    return stripped_header_len;
}

// ============================================================================
