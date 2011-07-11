// ----------------------------------------------------------------------------
// WavPack lib for Matroska
// ----------------------------------------------------------------------------
// Copyright christophe.paris@free.fr
// Parts by David Bryant http://www.wavpack.com
// Distributed under the BSD Software License
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#ifndef WAVPACK_FRAME_H_
#define WAVPACK_FRAME_H_
// ---------------------------------------------------------------------------

typedef struct {
  uint32_t block_samples; // number of samples per block
  uint32_t array_flags[10]; // flags of main frame (up to 10 blocks)
} common_frame_data;

typedef struct {
    char* data;
    int pos; // position on the data buffer
    int len; // total number of used bytes
    int total_len; // total len including free space
    uint32_t nb_block;
} frame_buffer;

//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
    
// ----------------------------------------------------------------------------

frame_buffer* frame_buffer_new();
void frame_buffer_free(frame_buffer* fb);

int frame_reserve_space(frame_buffer* dst, int len);

int frame_append_data(frame_buffer* dst, char* src, int len);
int frame_append_data2(frame_buffer* dst, stream_reader *io, int len);

void frame_reset(frame_buffer* dst);

int reconstruct_wavpack_frame(
    frame_buffer *frame,
    common_frame_data *common_data,
    char *pSrc,
    uint32_t SrcLength,
    int is_main_frame,
    int several_blocks,
    int version);

int strip_wavpack_block(frame_buffer *frame,
                        WavpackHeader *wphfr,
                        stream_reader *io,
                        uint32_t block_data_size,
                        int is_main_frame,
                        int several_blocks);

// ----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif  

// ---------------------------------------------------------------------------
#endif //WAVPACK_FRAME_H_
// ---------------------------------------------------------------------------