#ifndef SINET_TASK_OBSERVER_H
#define SINET_TASK_OBSERVER_H

namespace sinet
{

//////////////////////////////////////////////////////////////////////////
//
//  Interface for observer/callback mechanism
//
class itask_observer
{
public:
  virtual void progress_change(int new_progress) = 0;
  virtual void status_change(int new_status) = 0;
};

} // namespace sinet

#endif // SINET_TASK_OBSERVER_H