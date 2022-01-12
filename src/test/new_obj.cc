#include "test/new_obj.hh"

#include <iostream>

namespace gem5
{

NewObj::NewObj(const NewObjParams &params) :
    SimObject(params),
    param_1(params.param_1)
{
    std::cout << "A new SimObject! param_1:"<<param_1<< std::endl;
}

}
