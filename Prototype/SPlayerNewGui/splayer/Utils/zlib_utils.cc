#include "stdafx.h"
#include "zlib_utils.h"

#include <zlib.h>

namespace zlib_utils
{

const int block_size = 0x8000;
const int cpr_level = Z_BEST_COMPRESSION;

std::vector<unsigned char> Compress(std::vector<unsigned char>& buffer_to_compress)
{
  std::vector<unsigned char> retbuf;

  if (buffer_to_compress.empty())
    return retbuf;

  retbuf.resize(buffer_to_compress.size() + 
    (buffer_to_compress.size()/0x10) + 0x200 + block_size);

  z_stream zcpr;
  int ret = Z_OK;
  int orig_todo = buffer_to_compress.size();
  int orig_done = 0;
  int step = 0;
  memset(&zcpr, 0, sizeof(z_stream));
  deflateInit(&zcpr, cpr_level);

  zcpr.next_in = &buffer_to_compress[0];
  zcpr.next_out = &retbuf[0];

  do
  {
    long all_read_before = zcpr.total_in;
    zcpr.avail_in = min(orig_todo, block_size);
    zcpr.avail_out = block_size;
    ret = deflate(&zcpr,
      (zcpr.avail_in == (unsigned long)orig_todo) ? Z_FINISH : Z_SYNC_FLUSH);
    orig_done += (zcpr.total_in - all_read_before);
    orig_todo -= (zcpr.total_in - all_read_before);
    step++;
  } while (ret == Z_OK);

  retbuf.resize(zcpr.total_out);
  deflateEnd(&zcpr);

  return retbuf;
}

std::vector<unsigned char> Uncompress(std::vector<unsigned char>& buffer_to_uncompress)
{
  std::vector<unsigned char> retbuf;

  if (buffer_to_uncompress.empty())
    return retbuf;

  z_stream zcpr;
  int ret = Z_OK;
  long lOrigToDo = buffer_to_uncompress.size();
  long lOrigDone = 0;
  int step = 0;
  memset(&zcpr, 0, sizeof(z_stream));
  inflateInit(&zcpr);

  retbuf.resize(block_size);

  zcpr.next_in = &buffer_to_uncompress[0];
  zcpr.next_out = &retbuf[0];

  do
  {
    long all_read_before = zcpr.total_in;
    zcpr.avail_in = min(lOrigToDo, block_size);
    zcpr.avail_out = block_size;
    // check buffer before inflate, re-allocate if required
    if (retbuf.size() < block_size + zcpr.total_out)
    {
      retbuf.resize(block_size + zcpr.total_out);
      zcpr.next_out = &retbuf[zcpr.total_out];
    }
    ret = inflate(&zcpr, Z_SYNC_FLUSH);
    lOrigDone += (zcpr.total_in - all_read_before);
    lOrigToDo -= (zcpr.total_in - all_read_before);
    step++;
  } while (ret == Z_OK);

  retbuf.resize(zcpr.total_out);
  inflateEnd(&zcpr);

  return retbuf;
}

} // namespace zlib_utils