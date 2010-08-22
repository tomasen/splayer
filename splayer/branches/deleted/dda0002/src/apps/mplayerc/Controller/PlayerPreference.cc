#include "stdafx.h"
#include "SPlayerDefs.h"
#include "PlayerPreference.h"

PlayerPreference::PlayerPreference(void)
{
  //init
  //read from registry
}

PlayerPreference::~PlayerPreference(void)
{
  //write into registry
}

int PlayerPreference::GetIntVar(int id)
{
  std::map<int, int>::iterator it = m_map_intvar.find(id);
  if (it != m_map_intvar.end())
    return it->second;
  return 0;
}

void PlayerPreference::SetIntVar(int id, int value_in)
{
  m_map_intvar[id] = value_in;
}

std::wstring PlayerPreference::GetStringVar(int id)
{
  std::map<int, std::wstring>::iterator it = m_map_strvar.find(id);
  if(it != m_map_strvar.end())
    return it->second;
  return L"";
}

void PlayerPreference::SetStringVar(int id, std::wstring &value_in)
{
  m_map_strvar[id] = value_in;
}

std::vector<long long> PlayerPreference::GetInt64Array(int id)
{
  std::map<int, std::vector<long long>>::iterator it = m_map_int64array.find(id);
  if(it != m_map_int64array.end())
    return it->second;
  return std::vector<long long>();
}

void PlayerPreference::SetInt64Array(int id, std::vector<long long> &value_in)
{
  m_map_int64array[id] = value_in;
}

std::vector<int> PlayerPreference::GetIntArray(int id)
{
  std::map<int, std::vector<int>>::iterator it = m_map_intarray.find(id);
  if(it != m_map_intarray.end())
    return it->second;
  return std::vector<int>();
}

void PlayerPreference::SetIntArray(int id, std::vector<int> &value_in)
{
  m_map_intarray[id] = value_in;
}

std::vector<std::wstring> PlayerPreference::GetStrArray(int id)
{
  std::map<int, std::vector<std::wstring>>::iterator it = m_map_strarray.find(id);
  if(it != m_map_strarray.end())
    return it->second;
  return std::vector<std::wstring>();
}

void PlayerPreference::SetStrArray(int id, std::vector<std::wstring> &value_in)
{
  m_map_strarray[id] = value_in;
}
