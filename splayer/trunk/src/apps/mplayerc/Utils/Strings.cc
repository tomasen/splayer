#include "StdAfx.h"
#include "Strings.h"

int Strings::Split(const wchar_t* input, const wchar_t* delimiter,
                   std::vector<std::wstring>& array_out)
{
  int pos = 0;
  int newpos = -1;
  int sizes2 = lstrlen(delimiter);
  int isize = lstrlen(input);

  std::vector<int> positions;

  // wcsstr returns the ptr to the found delimiter,
  // minus |input| to get the index of the delimiter character
  newpos = wcsstr(input, delimiter) - input;

  if (newpos < 0)
    return 0;

  int numfound = 0;

  while (newpos > pos)
  {
    numfound++;
    positions.push_back(newpos);
    pos = newpos;
    newpos = wcsstr(input + pos + sizes2, delimiter) - input;
  }

  for (int i=0; i <= (int)positions.size(); i++)
  {
    std::wstring s;
    if (i == 0)
      s.assign(input + i, positions[i] - i);
    else
    {
      int offset = positions[i-1] + sizes2;
      if (offset < isize)
      {
        if (i == positions.size())
          s.assign(input + offset);
        else if (i > 0)
          s.assign(input + positions[i-1] + sizes2, positions[i] - positions[i-1] - sizes2);
      }
    }
    array_out.push_back(s);
  }
  return numfound;
}