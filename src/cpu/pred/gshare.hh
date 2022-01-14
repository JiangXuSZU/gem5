#ifndef __CPU_PRED_GSHARE_HH__
#define __CPU_PRED_GSHARE_HH__

#include "base/sat_counter.hh"
#include "cpu/pred/bpred_unit.hh"
#include "params/GShareBP.hh"

namespace gem5
{

namespace branch_prediction
{

class GShareBP : public BPredUnit
{
  public:
    GShareBP(const GShareBPParams &params);
    void uncondBranch(ThreadID tid, Addr pc, void * &bp_history);
    void squashed(ThreadID tid,void *bp_history);
    bool lookup(ThreadID tid, Addr branch_addr, void * &bp_history);
    void btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history);
    void update(ThreadID tid,Addr branch_addr, bool taken, void *bphistory,
                    bool squashed,const StaticInstPtr & inst, Addr corrTarget);
  private:
    void updateGlobalHistReg(ThreadID tid, bool taken);
    struct BPHistory
    {
        unsigned globalHistoryReg;
        bool pred;
    };

    std::vector<unsigned> globalHistoryReg;
    unsigned globalHistoryBits;
    unsigned globalHistoryMask;
    std::vector<SatCounter8> counter;
    unsigned ctrBits;
    unsigned counterSize;
    unsigned indexMask;
};

}//namespace breanch_prediction

}//namespace gem5

#endif // __CPU_PRED_GSHARE_HH__
