//
// Created by 杨至轩 on 9/6/15.
//

#include <include/ngram.h>
#include "titovdp_state.h"

namespace titovdp {

    Transition shift_actions[] = { SHIFT, LA_SHIFT, RA_SHIFT };
    Transition swap_actions[] = { SWAP, LA_SWAP, RA_SWAP };
    Transition reduce_actions[] = { REDUCE, LA_REDUCE, RA_REDUCE };

    ScoredPushComputation::ScoredPushComputation(const PushComputation &pc, const ScoreInformation &info)
            : pc(pc), attr(info) {

    }

    bool operator<(const ScoreInformation &a1, const ScoreInformation &a2) {
        return a1.prefix_score < a2.prefix_score;
    }

    bool operator<(const ScoredPushComputation &pc1, const ScoredPushComputation &pc2) {
        return pc1.attr.prefix_score < pc2.attr.prefix_score;
    }

    bool operator>(const ScoredPushComputation &pc1, const ScoredPushComputation &pc2) {
        return pc1.attr.prefix_score> pc2.attr.prefix_score;
    }

    bool PtrSPCCompare::operator()(const shared_ptr<const ScoredPushComputation> &p1,
                                   const shared_ptr<const ScoredPushComputation> &p2) {
        return p1->attr.prefix_score> p2->attr.prefix_score;
    }

    bool operator==(const PushComputation &pc1, const PushComputation &pc2) {
        if (pc1.i == pc2.i
            && pc1.j == pc2.j
            && pc1.y == pc2.y
            && pc1.z == pc2.z
            && pc1.jls == pc2.jls
            && pc1.yls == pc2.yls
            && pc1.yrs == pc2.yrs
            && pc1.zls == pc2.zls
            && pc1.zrs == pc2.zrs
            ){
            return true;
        }
        return false;
    }

    bool is_reduce(Transition t) {
        return t == REDUCE || t == LA_REDUCE || t == RA_REDUCE;
    }

    bool is_shift(Transition t) {
        return t == SHIFT || t ==  LA_SHIFT || t == RA_SHIFT;
    }


    bool PCMergeEqual::operator()(const PushComputation &pc1, const PushComputation &pc2) const {
        if (pc1.i == pc2.i
            && pc1.j == pc2.j
            && pc1.y == pc2.y
            && pc1.z == pc2.z
            && pc1.jls == pc2.jls
            && pc1.yls == pc2.yls
            && pc1.yrs == pc2.yrs
            && pc1.zls == pc2.zls
            && pc1.zrs == pc2.zrs
                ){
            return true;
        }
        return false;
    }

    size_t PCMergeHash::operator()(const PushComputation &pc) const {
        size_t seed = 0;
        hash_combine(seed, pc.i);
        hash_combine(seed, pc.j);
        hash_combine(seed, pc.y);
        hash_combine(seed, pc.z);
        hash_combine(seed, pc.jls);
        hash_combine(seed, pc.yls);
        hash_combine(seed, pc.yrs);
        hash_combine(seed, pc.zls);
        hash_combine(seed, pc.zrs);
        return seed;
    }

    bool operator==(const Deduction &d1, const Deduction &d2) {
        if (d1.type != d2.type) {
            return false;
        }
        if (!(d1.pc1 == d2.pc1)) {
            return false;
        }
        if (is_reduce(d1.type) && !(d1.pc2 == d2.pc2)) {
            return false;
        }
        return true;
    }

    PushComputation shift_pc(const PushComputation &pc, Transition action) {
        PushComputation newpc;
        newpc.i = pc.j;
        newpc.j = pc.j+1;
        newpc.y = pc.z;
        newpc.z = pc.j;
        newpc.jls = -1;
        newpc.yls = pc.zls, newpc.yrs = pc.zrs;
        newpc.zls = pc.jls, newpc.zrs = -1;
        if(action == LA_SHIFT) {
            newpc.zls = (pc.jls == -1 ? pc.z : std::min(pc.z, pc.jls));
        } else if(action == RA_SHIFT) {
            newpc.yrs = pc.j;
        }
        return newpc;
    }

    PushComputation swap_pc(const PushComputation &pc, Transition action) {
        PushComputation newpc(pc);
        if(action == LA_SWAP) {
            newpc.jls = (newpc.jls == -1 ? newpc.z : std::min(newpc.z, newpc.jls));
        } else if(action == RA_SWAP) {
            newpc.zrs = newpc.j;
        }
        std::swap(newpc.y, newpc.z);
        std::swap(newpc.yls, newpc.zls);
        std::swap(newpc.yrs, newpc.zrs);
        return newpc;
    }

    PushComputation reduce_pc(const PushComputation &pc1, const PushComputation &pc2, Transition action) {
        PushComputation pc;
        pc.i = pc1.i, pc.j = pc2.j;
        pc.y = pc1.y, pc.z = pc2.y;
        pc.jls = pc2.jls;
        pc.yls = pc1.yls, pc.yrs = pc1.yrs;
        pc.zls = pc2.yls, pc.zrs = pc2.yrs;
        if(action == LA_REDUCE) {
            pc.jls = (pc.jls == -1 ? pc2.z : std::min(pc2.z, pc.jls));
        } else if(action == RA_REDUCE) {
            ; // nothing
        }
        return pc;
    }

    std::string action_str(Transition t) {
        std::string s;
        switch(t) {
            case SHIFT:
                s = "----sh---->";
                break;
            case LA_SHIFT:
                s = "---la_sh-->";
                break;
            case RA_SHIFT:
                s = "---ra_sh-->";
                break;
            case SWAP:
                s = "----sw---->";
                break;
            case LA_SWAP:
                s = "---la_sw-->";
                break;
            case RA_SWAP:
                s = "---ra_sw-->";
                break;
            case REDUCE:
                s = "----re---->";
                break;
            case LA_REDUCE:
                s = "--la_re--->";
                break;
            case RA_REDUCE:
                s = "---ra_re-->";
                break;
        }
        return s;
    }

    std::ostream &operator<<(std::ostream &out, const Deduction &d) {
        switch (d.type) {
            case SHIFT:
            case LA_SHIFT:
            case RA_SHIFT:
                out << d.pc1 << action_str(d.type) << shift_pc(d.pc1, d.type);
                break;
            case SWAP:
            case LA_SWAP:
            case RA_SWAP:
                out << d.pc1 << action_str(d.type) << swap_pc(d.pc1, d.type);
                break;
            default:
                out << d.pc1 << " + " << d.pc2 << action_str(d.type) << reduce_pc(d.pc1, d.pc2, d.type);
        }
        return out;
    }

    std::ostream &operator<<(std::ostream &out, const PushComputation &pc) {
        out << "[" << pc.i << ", " << pc.y << ", " << pc.z << ", " << pc.j << /*"," << pc.jls<<", "<<pc.zls<<","<<pc.zrs<<", "<<pc.yls<<","<<pc.yrs <<*/ "]";
        return out;
    }

}
