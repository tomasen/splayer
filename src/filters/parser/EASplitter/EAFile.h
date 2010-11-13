#pragma once

#include "..\BaseSplitter\BaseSplitter.h"


class CEAFile  : public CBaseSplitterFile
{

	HRESULT Init();

public:
	CEAFile(IAsyncReader* pAsyncReader, HRESULT& hr);

	bool Probe();
};
