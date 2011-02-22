// HotkeySchemeParser_UnitTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../src/apps/mplayerc/Model/HotkeySchemeParser.h"

int _tmain(int argc, _TCHAR* argv[])
{
  HotkeySchemeParser parser;
  printf("HotkeySchemeParser unit test\n");
  printf("----------------------------------------------------------\n");
  printf("Populating default scheme (HotkeySchemeParser::PopulateDefaultScheme) ...\n");
  parser.PopulateDefaultScheme();
  printf("Attempting to write scheme file (HotkeySchemeParser::WriteToFile) ... ");
  if (!parser.WriteToFile(L"SPlayerHotKey.txt"))
  {
    printf("[FAILED]\n");
    return 0;
  }
  printf("[OK]\n");
  printf("Attempting to read scheme file (HotkeySchemeParser::ReadFromFile) ... ");
  if (!parser.ReadFromFile(L"SPlayerHotKey.txt"))
  {
    printf("[FAILED]\n");
    return 0;
  }
  printf("[OK]\n");
  HotkeySchemeParser target;
  target.PopulateDefaultScheme();
  printf("Comparing results (scheme name)... ");
  if (target.GetSchemeName() != parser.GetSchemeName())
  {
    printf("[ERROR]\n");
    return 0;
  }
  printf("[OK]\n");
  printf("Comparing results (list)... ");
  std::vector<HotkeyCmd> parser_list = parser.GetScheme();
  std::vector<HotkeyCmd> target_list = target.GetScheme();
  if (parser_list.size() != target_list.size())
  {
    printf("[SIZE MISMATCH]\n");
    return 0;
  }
  for (std::vector<HotkeyCmd>::iterator it1 = parser_list.begin(),
    it2 = target_list.begin();
    it1 != parser_list.end(),
    it2 != target_list.end();
    it1++, it2++)
  {
    if (it1->cmd != it2->cmd ||
      it1->key != it2->key ||
      it1->fVirt != it2->fVirt ||
      it1->appcmd != it2->appcmd ||
      it1->mouse != it2->mouse)
    {
      printf("[ERROR @ %d]\n", std::distance(parser_list.begin(), it1));
      return 0;
    }
  }
  printf("[OK]\n");

	return 0;
}
