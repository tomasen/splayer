#include "rhash_ex.h"
#include <sstream>
#include <boost/filesystem.hpp>

// Func : create_link
// the msg_id can be the combination of RHASH_ED2K, RHASH_SHA1, RHASH_BTIH
void RHash::create_link(unsigned hash_id, const std::string &file, std::vector<std::string> &result)
{
  using namespace boost::filesystem;

  // initialization
  rhash ctx = rhash_init(hash_id);
  if (ctx == 0) return;

  FILE *fd = fopen(file.c_str(), "rb");
  if (fd == 0) return;

  fseek(fd, 0, SEEK_END);
  long size = ftell(fd);
  rewind(fd);  // rewind the pointer
  std::stringstream ssSize;
  ssSize << size;

  // update the file hash
  rhash_file_update(ctx, fd);

  bool b = true;
  std::string value;
  std::string to_save;

  // deal with ed2k link
  b = _get_link_internal(RHASH_ED2K, ctx, value);
  if (b)
  {
    // ed2k://|file|[WMV和3GP互转工具].4Media.WMV.3GP.Converter.v6.0.2.0415.Incl.Keygen-Lz0.zip|34494025|DEDC307D2C7D6CC67D44D87957F94908|/
    to_save = "";
    to_save += "ed2k://|file|" + path(file).filename().string();
    to_save += "|" + ssSize.str();
    to_save += "|" + value + "|/";
    result.push_back(to_save);
  }

  // deal with sha-1 magnet link
  b = _get_link_internal(RHASH_SHA1, ctx, value);
  if (b)
  {
    // magnet:?xt=urn:sha1:YNCKHTQCWBTRNJIV4WNAE52SJUQCZO5C
    to_save = "";
    to_save += "magnet:?xt=urn:sha1:" + value;
    result.push_back(to_save);
  }

  // deal with btih magnet link
  b = _get_link_internal(RHASH_BTIH, ctx, value);
  if (b)
  {
    // magnet:?xt=urn:btih:XAKVF3ZS3LRJJUITOAQSNZCJOIM64NRI
    to_save = "";
    to_save += "magnet:?xt=urn:btih:" + value;
    result.push_back(to_save);
  }

  // clean
  fclose(fd);
  fd = 0;

  rhash_free(ctx);
  ctx = 0;
}

bool RHash::_get_link_internal(unsigned hash_id, rhash ctx, std::string &result)
{
  if (ctx->hash_id & hash_id)
  {
    // init
    unsigned char *final = new unsigned char[rhash_get_digest_size(hash_id)];
    char *result_temp = new char[rhash_get_hash_length(hash_id) + 1];
    result_temp[rhash_get_hash_length(hash_id)] = '\0';  // set the last char to '\0'

    // calc
    rhash_final(ctx, hash_id, final);
    rhash_print_bytes(result_temp, final, rhash_get_digest_size(hash_id), RHPR_HEX | RHPR_UPPERCASE);
    result = result_temp;

    // clean
    delete [] final;
    delete [] result_temp;

    return true;
  }
  else
  {
    return false;
  }  
}