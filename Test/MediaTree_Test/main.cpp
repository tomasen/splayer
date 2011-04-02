#include "stdafx.h"
#include "..\..\src\apps\mplayerc\Model\MediaComm.h"
#include "..\..\src\apps\mplayerc\Model\MediaTreeModel.h"
#include <boost/thread.hpp>
#include <boost/bind.hpp>

using namespace std;
int i = 0;
void show()
{
  static int xx = i;
  while (true)
  {
    if (xx != i)
    {
      wcout << i << endl;
      xx = i;
    }

    ::Sleep(50);
  }
}

int _tmain(int argc, _TCHAR* argv[])
{
  locale loc("chs");
  wcout.imbue(loc);

  MediaTreeModel model;

  // ---------------------------------------------------------------------------
  // prepare data
  vector<MediaPath> vtMPs;  // media path
  vector<MediaData> vtMDs;  // media data(include path and filename)

  MediaPath mp;
  mp.path = L"C:\\test\\New folder (2)\\";
  vtMPs.push_back(mp);

  MediaData md;
  md.path = L"C:\\cjbw1234\\New folder (2)\\";
  md.filename = L"splaa.mkv";

  //// ---------------------------------------------------------------------------
  //// test addPath's ability
  //wcout << L"test addPath's ability" << endl;
  //boost::thread tr(boost::bind(show));

  //wcout << L"start 100000 times test for addPath" << endl;
  //clock_t tStartAddPath = ::clock();

  //for (i = 0; i < 100000; ++i)
  //{
  //  model.addFolder(mp);
  //}

  //clock_t tEndAddPath = ::clock();
  //wcout << L"addPath's elapsed time is " << tEndAddPath - tStartAddPath << L"ms" << endl;
  //wcout << L"test addPath's ability end" << endl << endl;

  // ---------------------------------------------------------------------------
  // test addFile's ability
  wcout << L"test addFile's ability" << endl;
  boost::thread tr(boost::bind(show));

  wcout << L"start 100000 times test for addFile" << endl;
  clock_t tStartAddFile = ::clock();

  for (i = 0; i < 100000; ++i)
  {
    model.addFile(md);
  }

  clock_t tEndAddFile = ::clock();
  wcout << L"addFile's elapsed time is " << tEndAddFile - tStartAddFile << L"ms" << endl;
  wcout << L"test addFile's ability end" << endl << endl;

  // ---------------------------------------------------------------------------
  // test spider's correctness

  wcout << L"***************END***************" << endl;
  //tr.join();
  ::system("pause");
	return 0;
}