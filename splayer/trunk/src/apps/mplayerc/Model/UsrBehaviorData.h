#ifndef USERBEHAVIORDATA_H
#define USERBEHAVIORDATA_H

#define USRBHV_STARTSPLAYER  1
#define USRBHV_CLOSESPLAYER  2

#define DATABASE_NAME        L"splayer_ubdb_%s_%d.log"

class UsrBehaviorData
{
public:
  ~UsrBehaviorData();
  void AppendEntry(int id, std::wstring data);
  static int GetWeekCount();

private:
  // create and fill a new environment data table
  void SetEnvironmentData();

  struct UsrBehaviorEntry
  {
    int          id;
    std::wstring data;
    double       time;
  };

  std::vector<UsrBehaviorEntry> ubhv_entries;
};
#endif // USERBEHAVIORDATA_H