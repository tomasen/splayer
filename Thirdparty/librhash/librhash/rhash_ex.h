#pragma once

#include "rhash.h"
#include <string>
#include <vector>

class RHash
{
public:
  // the msg_id can be the combination of RHASH_ED2K, RHASH_SHA1, RHASH_BTIH
  static void create_link(unsigned hash_id, const std::string &file, std::vector<std::string> &result);

protected:
  // some helper functions, should not used directly
  static bool _get_link_internal(unsigned hash_id, rhash ctx, std::string &result);
};