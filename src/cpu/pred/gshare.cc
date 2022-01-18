#include "cpu/pred/gshare.hh"
#include "base/bitfield.hh"
#include "base/intmath.hh"
#include "debug/Fetch.hh"
#include "debug/GShare.hh"

namespace gem5
{

namespace branch_prediction
{

GShareBP::GShareBP(const GShareBPParams &params)
    : BPredUnit(params),
      globalHistoryReg(params.numThreads, 0),
      globalHistoryBits(ceilLog2(params.counterSize)),
      ctrBits(params.ctrBits),
      counterSize(params.counterSize),
      counters(counterSize,SatCounter8(ctrBits))
{
    if (!isPowerOf2(counterSize))
        fatal("Invalid counter size.\n");

    historyRegisterMask = mask(globalHistoryBits);
    globalHistoryMask = counterSize - 1;

    DPRINTF(GShare,"globalHistoryBits:%i\n",globalHistoryBits);
    DPRINTF(GShare,"ctrBits:%i\n",ctrBits);
    DPRINTF(GShare,"counterSize:%i\n",counterSize);
    DPRINTF(GShare,"historyRegisterMask:%#x\n",historyRegisterMask);
}

void
GShareBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->pred = true;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid,true);
}

void
GShareBP::squash(ThreadID tid, void *bpHistory)
{
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryReg[tid] = history->globalHistoryReg;

    delete history;

}

bool
GShareBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    unsigned counterIdx = (((branchAddr >> instShiftAmt)
                                ^ globalHistoryReg[tid])
                                & globalHistoryMask);

    assert(counterIdx < counterSize);

    bool pred = counters[counterIdx]>>(ctrBits-1);

    BPHistory *history = new BPHistory;
    history->globalHistoryReg=globalHistoryReg[tid];
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, pred);

    return pred;
}

void
GShareBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    globalHistoryReg[tid] &= (historyRegisterMask & ~1ULL);
}

void
GShareBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed, const StaticInstPtr & inst, Addr corrTarget)
{
    assert(bpHistory);
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    if (squashed) {
        globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
        return;
    }
    unsigned counterIdx = (((branchAddr >> instShiftAmt)
                                ^ history->globalHistoryReg)
                                & globalHistoryMask);
    if (history->pred) {
        counters[counterIdx]++;
    }else{
        counters[counterIdx]--;
    }
    delete history;
}

void
GShareBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
        globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= historyRegisterMask;

}

} // namespace branch_prediction
} // namespace gem5
