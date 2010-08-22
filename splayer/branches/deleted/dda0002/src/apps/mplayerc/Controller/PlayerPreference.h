#ifndef PLAYERPREFERENCE_H
#define PLAYERPREFERENCE_H

#include "LazyInstance.h"
#include <set>

class PlayerPreference :
  public LazyInstanceImpl<PlayerPreference>
{
public:
  PlayerPreference(void);
  ~PlayerPreference(void);

  int GetIntVar(int id);
  void SetIntVar(int id, int value_in);

  std::wstring GetStringVar(int id);
  void SetStringVar(int id, std::wstring &value_in);

  std::vector<long long> GetInt64Array(int id);
  void SetInt64Array(int id, std::vector<long long> &value_in);

  std::vector<int> GetIntArray(int id);
  void SetIntArray(int id, std::vector<int> &value_in);

  std::vector<std::wstring> GetStrArray(int id);
  void SetStrArray(int id, std::vector<std::wstring> &value_in);

private:
  std::map<int, int>          m_map_intvar;
  std::map<int, std::wstring> m_map_strvar;
  std::map<int, std::vector<long long>>    m_map_int64array;
  std::map<int, std::vector<int>>          m_map_intarray;
  std::map<int, std::vector<std::wstring>> m_map_strarray;
};

#endif //PLAYERPREFERENCE_H
#ifndef PLAYERPREFERENCE_H
#define PLAYERPREFERENCE_H

#include "LazyInstance.h"

//////////////////////////////////////////////////////////////////////////
//
//  PlayerPreference is the program-wide instance for manage
//  preferences and settings.
//
class PlayerPreference :
  public LazyInstanceImpl<PlayerPreference>
{
public:
  PlayerPreference(void);
  ~PlayerPreference(void);

  int GetIntVar(int id);
  void SetIntVar(int id, int value_in);

private:
  std::map<int, int> m_map_intvar;
};

#endif // PLAYERPREFERENCE_H