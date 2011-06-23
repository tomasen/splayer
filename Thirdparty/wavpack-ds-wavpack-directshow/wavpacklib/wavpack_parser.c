// ----------------------------------------------------------------------------
// WavPack lib for Matroska
// ----------------------------------------------------------------------------
// Copyright christophe.paris@free.fr
// Parts by David Bryant http://www.wavpack.com
// Distributed under the BSD Software License
// ----------------------------------------------------------------------------

#include "../wavpack/wputils.h"
#include "wavpack_common.h"
#include "wavpack_frame.h"
#include "wavpack_parser.h"

// ----------------------------------------------------------------------------

int add_block(WavPack_parser* wpp, uint32_t block_data_size);

static uint32_t find_sample(WavPack_parser* wpp,
                            uint32_t header_pos,
                            uint32_t sample);

static uint32_t find_header (stream_reader *reader,
                             void *id,
                             uint32_t filepos,
                             WavpackHeader *wphdr);

int32_t get_wp_block(WavPack_parser *wpp,
                     int* is_final_block);


static uint32_t seek_final_index (stream_reader *reader, void *id);

// ----------------------------------------------------------------------------

const uint32_t sample_rates [] = {
   6000,  8000,  9600, 11025, 12000, 16000, 22050,
  24000, 32000, 44100, 48000, 64000, 88200, 96000, 192000 };

// ----------------------------------------------------------------------------

#define ID_OPTIONAL_DATA	0x20
#define ID_ODD_SIZE		0x40
#define ID_LARGE		0x80
   
#define ID_DUMMY		0x0
#define ID_ENCODER_INFO		0x1
#define ID_DECORR_TERMS		0x2
#define ID_DECORR_WEIGHTS	0x3
#define ID_DECORR_SAMPLES	0x4
#define ID_ENTROPY_VARS		0x5
#define ID_HYBRID_PROFILE	0x6
#define ID_SHAPING_WEIGHTS	0x7
#define ID_FLOAT_INFO		0x8
#define ID_INT32_INFO		0x9
#define ID_WV_BITSTREAM		0xa
#define ID_WVC_BITSTREAM	0xb
#define ID_WVX_BITSTREAM	0xc
#define ID_CHANNEL_INFO		0xd
   
#define ID_RIFF_HEADER		(ID_OPTIONAL_DATA | 0x1)
#define ID_RIFF_TRAILER		(ID_OPTIONAL_DATA | 0x2)
#define ID_REPLAY_GAIN		(ID_OPTIONAL_DATA | 0x3)
#define ID_CUESHEET		(ID_OPTIONAL_DATA | 0x4)
#define ID_CONFIG_BLOCK		(ID_OPTIONAL_DATA | 0x5)
#define ID_MD5_CHECKSUM		(ID_OPTIONAL_DATA | 0x6)



int is_correction_block(uchar* blockbuff, uint32_t len)
{
    WavpackMetadata wpmd;
    uchar *blockptr_start = blockbuff;
    uchar *blockptr_end = blockbuff + len;
    int correction = 0;
    while (read_metadata_buff (&wpmd, &blockptr_start, blockptr_end))
    {
        if (!process_metadata (&wpmd, &correction))
        {
            break;
        }
    }
    return correction;
}

// ----------------------------------------------------------------------------

WavPack_parser* wavpack_parser_new(stream_reader* io, int is_correction)
{
	uint32_t bcount = 0;
	int is_final_block = FALSE;
    int striped_header_len = 0;
    WavPack_parser* wpp = wp_alloc(sizeof(WavPack_parser));
    if(!wpp)
	{
        return NULL;
	}

    wpp->io = io;
    wpp->is_correction = is_correction;
	
    // TODO :we could determinate if it's a correction file by parsing first block metadata

    wpp->fb = frame_buffer_new();
    if(!wpp->fb)
    {
        wavpack_parser_free(wpp);
        return NULL;
    }

	// Read the first frame
	do {
		// Get next WavPack block info
		bcount = get_wp_block(wpp, &is_final_block);
		if(bcount == -1)
		{
			break;
		}

		if(wpp->fb->nb_block == 0)
		{
            // Store the first header
            wpp->first_wphdr = wpp->wphdr;
            wpp->several_blocks = !is_final_block;
            // Assume those data never change
            wpp->bits_per_sample = wpp->block_bits_per_sample;
            wpp->sample_rate = wpp->block_sample_rate;
            // Assume first block is the biggest ???
            wpp->samples_per_block = wpp->block_samples_per_block;

            // Make sure we have the total number of samples
            if(wpp->first_wphdr.total_samples == (uint32_t)-1)
            {
                // Seek at the end of the file to guess total_samples
                uint32_t curr_pos = wpp->io->get_pos(wpp->io);
                uint32_t final_index = seek_final_index (wpp->io, wpp->io);
                if (final_index != (uint32_t) -1)
                {
                    wpp->first_wphdr.total_samples = final_index - wpp->first_wphdr.block_index;
                }                
                // restaure position
                wpp->io->set_pos_abs(wpp->io, curr_pos);
            }
		}

        wpp->channel_count += wpp->block_channel_count;

        striped_header_len = strip_wavpack_block(wpp->fb, &wpp->wphdr, wpp->io, bcount,
            !wpp->is_correction, wpp->several_blocks);

        /*
        // check metadata to know if it's a correction block
        if((wpp->fb->nb_block == 1) &&
           ((wpp->first_wphdr.flags & WV_HYBRID_FLAG) == WV_HYBRID_FLAG))
        {
            int correction = is_correction_block(wpp->fb->data + striped_header_len,
                wpp->fb->len - striped_header_len);

        }
        */

	} while(is_final_block == FALSE);
    
	if(wpp->fb->len > 0)
	{
        // Calculate the suggested buffer size, we use the size of the RAW
        // decoded data as reference and add 10% to be safer
        wpp->suggested_buffer_size = (int)(wpp->samples_per_block * 4 *
            wpp->channel_count * 1.1);        
        wpp->suggested_buffer_size += (sizeof(WavpackHeader) * wpp->fb->nb_block);
		return wpp;
	}
	else
	{
		wavpack_parser_free(wpp);
		return NULL;
	}
}

// ----------------------------------------------------------------------------

unsigned long wavpack_parser_read_frame(
	WavPack_parser* wpp,
    unsigned char* dst,
    unsigned long* FrameIndex,
    unsigned long* FrameLen)
{
	int is_final_block = FALSE;
	uint32_t bcount = 0;
	uint32_t frame_len_bytes = 0;

	if(wpp->fb->len > 0)
	{
		*FrameIndex = wpp->wphdr.block_index;
		*FrameLen = wpp->wphdr.block_samples;        
		wp_memcpy(dst, wpp->fb->data, wpp->fb->len);
	}
	else
	{
		do {
			// Get next WavPack block info
			bcount = get_wp_block(wpp, &is_final_block);
			if(bcount == -1)
			{
                wpp->wvparser_eof = 1;
				break;
			}
            strip_wavpack_block(wpp->fb, &wpp->wphdr, wpp->io, bcount,
                !wpp->is_correction,  wpp->several_blocks);
		} while(is_final_block == FALSE);

		*FrameIndex = wpp->wphdr.block_index;
		*FrameLen = wpp->wphdr.block_samples;
		wp_memcpy(dst, wpp->fb->data, wpp->fb->len);
	}

	frame_len_bytes = wpp->fb->len;

    frame_reset(wpp->fb);

	return frame_len_bytes;
}

// ----------------------------------------------------------------------------


void wavpack_parser_seek(WavPack_parser* wpp, uint64 seek_pos_100ns)
{
    uint32_t sample_pos = (uint32_t)((seek_pos_100ns / 10000000.0) * wpp->sample_rate);
    uint32_t newpos = find_sample(wpp, 0, sample_pos);

    DebugLog("%c wavpack_parser_seek : seeking at pos = %d",
        wpp->is_correction ? 'C': 'N',
        newpos);

    if(wpp->io->set_pos_abs(wpp->io, newpos) == 0)
    {
        wpp->wvparser_eof = 0;
    }
    else
    {
        wpp->wvparser_eof = 1;
    }
}

// ----------------------------------------------------------------------------

int wavpack_parser_eof(WavPack_parser* wpp)
{
	return wpp->wvparser_eof;
}

// ----------------------------------------------------------------------------

void wavpack_parser_free(WavPack_parser* wpp)
{
    if(wpp != NULL)
    {
        if(wpp->fb != NULL)
        {
            frame_buffer_free(wpp->fb);
            wpp->fb = NULL;
        }        
	    wp_free(wpp);
    }
}

// ----------------------------------------------------------------------------

#define WavpackHeaderFormat "4LS2LLLLL"

static void
little_endian_to_native(void *data,
                        char *format)
{
    uint8_t *cp = (uint8_t *)data;
    int32_t temp;
    
    while (*format) 
    {
        switch (*format) 
        {
        case 'L':
            temp = cp[0] + ((long)cp[1] << 8) + ((long)cp[2] << 16) +
                ((long)cp[3] << 24);
            *(long *)cp = temp;
            cp += 4;
            break;
            
        case 'S':
            temp = cp[0] + (cp[1] << 8);
            * (short *)cp = (short)temp;
            cp += 2;
            break;
            
        default:
            if (isdigit(*format))
                cp += *format - '0';
            
            break;
        }
        
        format++;
    }
}

// ----------------------------------------------------------------------------

// Read from current file position until a valid 32-byte WavPack 4.0 header is
// found and read into the specified pointer. The number of bytes skipped is
// returned. If no WavPack header is found within 1 meg, then a -1 is returned
// to indicate the error. No additional bytes are read past the header and it
// is returned in the processor's native endian mode. Seeking is not required.

static uint32_t read_next_header (stream_reader *reader, void *id, WavpackHeader *wphdr)
{
    char buffer [sizeof (*wphdr)], *sp = buffer + sizeof (*wphdr), *ep = sp;
    uint32_t bytes_skipped = 0;
    int bleft;
    
    while (1) {
        if (sp < ep) {
            bleft = ep - sp;
            wp_memcpy (buffer, sp, bleft);
        }
        else
            bleft = 0;
        
        if (reader->read_bytes (id, buffer + bleft, sizeof (*wphdr) - bleft) != sizeof (*wphdr) - bleft)
            return -1;
        
        sp = buffer;
        
        if (*sp++ == 'w' && *sp == 'v' && *++sp == 'p' && *++sp == 'k' &&
            !(*++sp & 1) && sp [2] < 16 && !sp [3] && sp [5] == 4 && sp [4] >= 2 && sp [4] <= 0xf) {
            wp_memcpy (wphdr, buffer, sizeof (*wphdr));
            little_endian_to_native (wphdr, WavpackHeaderFormat);
            return bytes_skipped;
        }
        
        while (sp < ep && *sp != 'w')
            sp++;
        
        if ((bytes_skipped += sp - buffer) > 1024 * 1024)
            return -1;
    }
}


// ----------------------------------------------------------------------------

int32_t
get_wp_block(WavPack_parser *wpp, int* is_final_block)
{
	uint32_t bcount = 0;
	uint32_t data_size = 0;
    uint32_t bytes_per_sample = 0;

	*is_final_block = FALSE;

    wpp->block_channel_count = 0;
    wpp->block_sample_rate = 0;
    wpp->block_bits_per_sample = 0;
	wpp->block_samples_per_block = 0;
    
    DebugLog("%c get_wp_block : current pos = %d",
        wpp->is_correction ? 'C' : 'N',
        wpp->io->get_pos(wpp->io));

	// read next WavPack header
	bcount = read_next_header(wpp->io, wpp->io, &wpp->wphdr);

    if(bcount > 0)
    {
        DebugLog("%c get_wp_block : skipped %d",
            wpp->is_correction ? 'C' : 'N',
            bcount);
    }

	if (bcount == (uint32_t) -1)
	{
        return -1;
	}

	// if there's audio samples in there...
	if (wpp->wphdr.block_samples)
	{
        if((wpp->wphdr.flags & WV_SRATE_MASK) == WV_SRATE_MASK)
        {
            wpp->block_sample_rate = 44100;
        }
        else
        {
            wpp->block_sample_rate = (wpp->wphdr.flags & WV_SRATE_MASK) >> WV_SRATE_LSB;
            wpp->block_sample_rate = sample_rates[wpp->block_sample_rate];
        }

        bytes_per_sample = ((wpp->wphdr.flags & WV_BYTES_STORED) + 1);
      
        wpp->block_bits_per_sample = (bytes_per_sample * 8) - 
            ((wpp->wphdr.flags & WV_SHIFT_MASK) >> WV_SHIFT_LSB);

        wpp->block_samples_per_block = wpp->wphdr.block_samples;

        wpp->block_channel_count = (wpp->wphdr.flags & WV_MONO_FLAG) ? 1 : 2;

        if (wpp->wphdr.flags & WV_FINAL_BLOCK)
        {
            *is_final_block = TRUE;
        }
	}
	else
	{
		// printf ("non-audio block found\n");
		return -1;
	}

	data_size = wpp->wphdr.ckSize - sizeof(WavpackHeader) + 8;
	
	return data_size;
}

// ----------------------------------------------------------------------------

// Find a valid WavPack header, searching either from the current file position
// (or from the specified position if not -1) and store it (endian corrected)
// at the specified pointer. The return value is the exact file position of the
// header, although we may have actually read past it. Because this function
// is used for seeking to a specific audio sample, it only considers blocks
// that contain audio samples for the initial stream to be valid.

#define BUFSIZE 4096

static uint32_t find_header (stream_reader *reader,
                             void *id,
                             uint32_t filepos,
                             WavpackHeader *wphdr)
{
    char *buffer = wp_alloc(BUFSIZE), *sp = buffer, *ep = buffer;
    
    if (filepos != (uint32_t) -1 && reader->set_pos_abs(id, filepos))
    {
        wp_free(buffer);
        return -1;
    }
    
    while (1)
    {
        int bleft;
        
        if (sp < ep)
        {
            bleft = ep - sp;
            wp_memcpy(buffer, sp, bleft);
            ep -= (sp - buffer);
            sp = buffer;
        }
        else
        {
            if (sp > ep)
            {
                if (reader->set_pos_rel (id, sp - ep, SEEK_CUR))
                {
                    wp_free(buffer);
                    return -1;
                }
            }
                
            sp = ep = buffer;
            bleft = 0;
        }
        
        ep += reader->read_bytes (id, ep, BUFSIZE - bleft);
        
        if (ep - sp < 32)
        {
            wp_free(buffer);
            return -1;
        }
        
        while (sp + 32 <= ep)
        {
            if (*sp++ == 'w' && *sp == 'v' && *++sp == 'p' && *++sp == 'k' &&
                !(*++sp & 1) && sp [2] < 16 && !sp [3] && sp [5] == 4 && sp [4] >= 2 && sp [4] <= 0xf) {
                wp_memcpy(wphdr, sp - 4, sizeof (*wphdr));
                little_endian_to_native(wphdr, WavpackHeaderFormat);
                
                if (wphdr->block_samples && (wphdr->flags & WV_INITIAL_BLOCK)) {
                    wp_free(buffer);
                    return reader->get_pos (id) - (ep - sp + 4);
                }
                
                if (wphdr->ckSize > 1024)
                {
                    sp += wphdr->ckSize - 1024;
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------

// Find the WavPack block that contains the specified sample. If "header_pos"
// is zero, then no information is assumed except the total number of samples
// in the file and its size in bytes. If "header_pos" is non-zero then we
// assume that it is the file position of the valid header image contained in
// the first stream and we can limit our search to either the portion above
// or below that point. If a .wvc file is being used, then this must be called
// for that file also.

static uint32_t find_sample(WavPack_parser* wpp,
                            uint32_t header_pos,
                            uint32_t sample)
{
    uint32_t file_pos1 = 0, file_pos2 = wpp->io->get_length(wpp->io);
    uint32_t sample_pos1 = 0, sample_pos2 = wpp->first_wphdr.total_samples;
    double ratio = 0.96;
    int file_skip = 0;

    if (sample >= wpp->first_wphdr.total_samples)
    {
        return -1;
    }	

    if (header_pos)
    {
	    if (wpp->wphdr.block_index > sample)
        {
    	    sample_pos2 = wpp->wphdr.block_index;
	        file_pos2 = header_pos;
	    }
	    else if (wpp->wphdr.block_index + wpp->wphdr.block_samples <= sample)
        {
    	    sample_pos1 = wpp->wphdr.block_index;
	        file_pos1 = header_pos;
	    }
	    else
        {
    	    return header_pos;
        }
    }

    while (1)
    {
	    double bytes_per_sample;
	    uint32_t seek_pos;

	    bytes_per_sample = file_pos2 - file_pos1;
	    bytes_per_sample /= sample_pos2 - sample_pos1;
	    seek_pos = file_pos1 + (file_skip ? 32 : 0);
	    seek_pos += (uint32_t)(bytes_per_sample * (sample - sample_pos1) * ratio);
	    seek_pos = find_header(wpp->io, wpp->io, seek_pos, &wpp->wphdr);

	    if (seek_pos == (uint32_t) -1 || seek_pos >= file_pos2)
        {
	        if (ratio > 0.0)
            {
    		    if ((ratio -= 0.24) < 0.0)
                {
    		        ratio = 0.0;
                }
	        }
	        else
            {
                return -1;
            }
        }
	    else if (wpp->wphdr.block_index > sample)
        {
	        sample_pos2 = wpp->wphdr.block_index;
	        file_pos2 = seek_pos;
	    }
	    else if (wpp->wphdr.block_index + wpp->wphdr.block_samples <= sample)
        {
	        if (seek_pos == file_pos1)
            {            
		        file_skip = 1;
            }
	        else
            {
		        sample_pos1 = wpp->wphdr.block_index;
		        file_pos1 = seek_pos;
	        }
	    }
	    else
        {       
	        return seek_pos;
        }
    }
}

// ----------------------------------------------------------------------------

// This function is used to seek to end of a file to determine its actual
// length in samples by reading the last header block containing data.
// Currently, all WavPack files contain the sample length in the first block
// containing samples, however this might not always be the case. Obviously,
// this function requires a seekable file or stream and leaves the file
// pointer undefined. A return value of -1 indicates the length could not
// be determined.

static uint32_t seek_final_index (stream_reader *reader, void *id)
{
    uint32_t result = (uint32_t) -1, bcount;
    WavpackHeader wphdr;
    
    if (reader->get_length (id) > 1200000L)
    {
        reader->set_pos_rel (id, -1048576L, SEEK_END);
    }
    
    while (1)
    {
        bcount = read_next_header (reader, id, &wphdr);
        
        if (bcount == (uint32_t) -1)
        {
            return result;
        }
                
        if (wphdr.block_samples && (wphdr.flags & WV_FINAL_BLOCK))
        {
            result = wphdr.block_index + wphdr.block_samples;
        }
        
        if (wphdr.ckSize > sizeof (WavpackHeader) - 8)
        {
            reader->set_pos_rel (id, wphdr.ckSize - sizeof (WavpackHeader) + 8, SEEK_CUR);
        }
    }
}

// ----------------------------------------------------------------------------