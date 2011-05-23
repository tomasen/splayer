#include "stdafx.h"
#include "rsc_format.h"
#include "../Utils/zlib_utils.h"
#include "../Utils/strings.h"
#include "../Utils/base64_utils.h"

#include <fstream>
#include <yaml.h>

namespace rsc_format
{

bool Parse(const wchar_t* filename, std::map<std::wstring, std::wstring>& str_output,
           std::map<std::wstring, std::vector<unsigned char> >& buf_output)
{
  std::ifstream fs(filename, std::ios::binary);

  str_output.clear();
  buf_output.clear();

  if (!fs.is_open())
    return false;

  fs.seekg(0, std::ios::end);
  std::ifstream::pos_type file_size = fs.tellg();
  fs.seekg(0, std::ios::beg);

  if (static_cast<int>(file_size) == 0)
    return false;

  std::vector<unsigned char> buffer(file_size);
  fs.read((char*)&buffer[0], file_size);
  fs.close();

  std::vector<unsigned char> yaml_utf8s = zlib_utils::Uncompress(buffer);
  if (yaml_utf8s.empty())
    return false;

  std::stringstream yaml_istm;
  yaml_utf8s.push_back(0);
  yaml_istm << (char*)&yaml_utf8s[0];

  try
  {
    YAML::Parser parser(yaml_istm);
    YAML::Node doc;
    if (!parser.GetNextDocument(doc))
      return false;

    const YAML::Node& str_node = doc["string"];
    for (YAML::Iterator it = str_node.begin(); it != str_node.end(); it++)
      str_output[string_util::Utf8StringToWString(it.first())] =
        string_util::Utf8StringToWString(it.second());

    const YAML::Node& bin_node = doc["bin"];
    for (YAML::Iterator it = bin_node.begin(); it != bin_node.end(); it++)
    {
      std::string temp_b64;
      it.second() >> temp_b64;
      std::vector<unsigned char> buf = base64_utils::Decode(temp_b64);
      buf_output[string_util::Utf8StringToWString(it.first())] = buf;
    }
  }
  catch (YAML::ParserException& e)
  {
    UNREFERENCED_LOCAL_VARIABLE(e);
    return false;
  }

  return true;
}

} // namespace rsc_format