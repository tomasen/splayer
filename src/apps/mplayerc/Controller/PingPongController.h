
#pragma once

#include "LazyInstance.h"
#include <threadhelper.h>
#include "NetworkControlerImpl.h"
#include "../UserInterface/Dialogs/MovieComment_Win.h"
#include <comutil.h>

class PingPongController:
    public NetworkControlerImpl,
    public ThreadHelperImpl<PingPongController>,
    public LazyInstanceImpl<PingPongController>
{
public:

    PingPongController();
    ~PingPongController();

    void _Thread();
    void PingPong();

};
