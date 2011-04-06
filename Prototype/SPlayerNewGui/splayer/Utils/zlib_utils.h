#ifndef ZLIB_UTILS_H
#define ZLIB_UTILS_H

namespace zlib_utils
{

std::vector<unsigned char> Compress(std::vector<unsigned char>& buffer_to_compress);
std::vector<unsigned char> Uncompress(std::vector<unsigned char>& buffer_to_uncompress);

} // namespace zlib_utils

#endif // ZLIB_UTILS_H