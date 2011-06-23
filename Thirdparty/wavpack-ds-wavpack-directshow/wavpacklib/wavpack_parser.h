// ----------------------------------------------------------------------------
// WavPack lib for Matroska
// ----------------------------------------------------------------------------
// Copyright christophe.paris@free.fr
// Parts by David Bryant http://www.wavpack.com
// Distributed under the BSD Software License
// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------
#ifndef _WAVPACK_PARSER_H_
#define _WAVPACK_PARSER_H_
// ----------------------------------------------------------------------------

typedef struct {
    stream_reader* io;
    int wvparser_eof;
    int is_correction;

    WavpackHeader first_wphdr;
    WavpackHeader wphdr; // last header read

    // last block meta data
    int block_channel_count;
    int block_bits_per_sample;
    uint32_t block_sample_rate;
    uint32_t block_samples_per_block;

    // global metadata
    int channel_count;
    int bits_per_sample;
    uint32_t sample_rate;
    uint32_t samples_per_block;

    int several_blocks;

    frame_buffer* fb;
    uint32_t suggested_buffer_size;

} WavPack_parser;

// ----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
// ----------------------------------------------------------------------------


WavPack_parser* wavpack_parser_new(stream_reader* io, int is_correction);

unsigned long wavpack_parser_read_frame(
    WavPack_parser* wpp,
    unsigned char* dst,
    unsigned long* FrameIndex,
    unsigned long* FrameLen);

void wavpack_parser_seek(WavPack_parser* wpp, uint64 seek_pos_100ns);

int wavpack_parser_eof(WavPack_parser* wpp);

void wavpack_parser_free(WavPack_parser* wpp);


// ----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif  
// ----------------------------------------------------------------------------

#endif // _WAVPACK_PARSER_H_

// ----------------------------------------------------------------------------
