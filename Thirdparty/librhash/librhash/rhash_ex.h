#pragma once

#include "rhash.h"
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// some useful functions, mainly used in SPlayer by ourself
// Func : create_link
// the msg_id can be the combination of RHASH_ED2K, RHASH_SHA1, RHASH_BTIH
extern void create_link(unsigned hash_id, const std::string &file, std::vector<std::string> &result);

// some helper functions, should not used directly
bool _get_link_internal(unsigned hash_id, rhash ctx, std::string &result);