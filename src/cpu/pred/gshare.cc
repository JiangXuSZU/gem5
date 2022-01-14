#include "cpu/pred/gshare.hh"

#include "base/intmath.hh"
#include "base/logging.hh"
#include "base/trace.hh"
#include "debug/Fetch.hh"

namespace gem5
{

namespace branch_prediction
{

GShareBP::GShareBP(const GShareBPParams &params)
    : BPredUnit(params),
      globalHistoryReg(params.numThreads,0),
      globalHistoryBits(ceilLog2(params.counterSize)),
      counter(params.counterSize,SatCounter8(params.ctrBits)),
      ctrBits(params.ctrBits),
      counterSize(params.counterSize)
{
    if (!isPowerOf2(counterSize)){
        fatal("Invalid gshare pht size!\n");
    }

    globalHistoryMask=counterSize-1;
    indexMask = counterSize-1;

    DPRINTF(Fetch,"GHR bit: %i\n",globalHistoryBits);
    DPRINTF(Fetch,"Counter Size: %i\n",counterSize);
    DPRINTF(Fetch,"Counter Bits: %i\n",ctrBits);
    DPRINTF(Fetch,"index mask: %#x\n",indexMask);
}//GShareBP::GShareBP

void
GShareBP::squashed(ThreadID tid, void *bp_history)
{
    BPHistory *history = static_cast<BPHistory*>(bp_history);
    globalHistoryReg[tid] = history->globalHistoryReg;

    delete history;
}

void
GShareBP::btbUpdate(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    globalHistoryReg[tid] &= (globalHistoryMask & ~1ULL);
}//GShareBP::btbUpdate

bool
GShareBP::lookup(ThreadID tid, Addr branch_addr, void * &bp_history)
{
    unsigned index = ((branch_addr^globalHistoryReg[tid])&globalHistoryMask);
    bool pred = counter[index]>>(ctrBits-1);

    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->pred = pred;

    bp_history = static_cast<void*>(history);
    updateGlobalHistReg(tid, pred);
    return pred;
}

void
GShareBP::update(ThreadID tid, Addr branch_addr, bool taken, void *bp_history,
                    bool squashed, const StaticInstPtr &inst, Addr corrTarget)
{
    assert(bp_history == NULL);
    BPHistory *history = static_cast<BPHistory*>(bp_history);
    if (squashed) {
        globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
        return;
    }

    unsigned index = ((branch_addr^(history->globalHistoryReg))
                    &globalHistoryMask);
    if (taken) {
        DPRINTF(Fetch,"Branch updated as taken.\n");
        counter[index]++;
    } else {
        DPRINTF(Fetch, "Branch updated as not taken.\n");
        counter[index]++;
    }
    delete history;
}

void
GShareBP::uncondBranch(ThreadID tid, Addr pc, void *&bp_history)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->pred = true;
    bp_history = static_cast<void*>(history);
    updateGlobalHistReg(tid,true);
}

void
GShareBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= globalHistoryMask;

}


}//branch_prediction

}//gem5
