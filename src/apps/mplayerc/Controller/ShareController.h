
#pragma once

#include "LazyInstance.h"
#include <threadhelper.h>
#include "NetworkControlerImpl.h"

typedef enum
{
    UNKNOWN = 0,
    SUCCESS
} SP_APIRET;

class UserShareController:
    public NetworkControlerImpl,
    public ThreadHelperImpl<UserShareController>,
    public LazyInstanceImpl<UserShareController>
{
public:

    UserShareController();
    ~UserShareController();

    void _Thread();
    void Share(std::wstring uuid, std::wstring hash);

private:
    std::wstring m_uuid;
    std::wstring m_sphash;
    SP_APIRET m_apiret;
};
