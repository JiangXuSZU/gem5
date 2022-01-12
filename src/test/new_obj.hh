#ifndef __NEWOBJ_HH__
#define __NEWOBJ_HH__

#include "params/NewObj.hh"
#include "sim/sim_object.hh"

namespace gem5
{

class NewObj : public SimObject
{
  public:
    NewObj(const NewObjParams &p);
  private:
    const int param_1;
};

}
#endif // __NEWOBJ_HH__
