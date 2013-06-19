#ifndef PNG_UTILS_H
#define PNG_UTILS_H

namespace png_utils
{

HBITMAP LoadBuffer(std::vector<unsigned char>& input_png);
HBITMAP LoadFile(const wchar_t* filename);

} // namespace png_utils

#endif // PNG_UTILS_H