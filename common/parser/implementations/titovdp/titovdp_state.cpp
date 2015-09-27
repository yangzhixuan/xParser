//
// Created by 杨至轩 on 9/6/15.
//

#include <include/ngram.h>
#include "titovdp_state.h"

namespace titovdp{
    ScoredPushComputation::ScoredPushComputation(const PushComputation &pc, const ScoreInformation &info)
            : pc(pc), attr(info) {

    }

    bool operator<(const ScoreInformation & a1, const ScoreInformation & a2) {
        return a1.score < a2.score;
    }
    bool operator<(const ScoredPushComputation & pc1, const ScoredPushComputation & pc2) {
        return pc1.attr.score < pc2.attr.score;
    }
    bool operator>(const ScoredPushComputation & pc1, const ScoredPushComputation & pc2) {
        return pc1.attr.score > pc2.attr.score;
    }

    bool operator==(const PushComputation & pc1, const PushComputation & pc2) {
        if(pc1.i == pc2.i
           && pc1.j == pc2.j
           && pc1.x == pc2.x
           && pc1.y == pc2.y
           && pc1.z == pc2.z
     //      && pc1.jls == pc2.jls
     //      && pc1.jrs == pc2.jrs
           && pc1.yj_connected == pc2.yj_connected
           && pc1.jy_connected == pc2.jy_connected
           && pc1.zj_connected == pc2.zj_connected
           && pc1.jz_connected == pc2.jz_connected
           && pc1.yz_swapped == pc2.yz_swapped) {
            return true;
        }
        return false;
    }

    bool operator==(const Deduction & d1, const Deduction & d2) {
        if(d1.type != d2.type) {
            return false;
        }
        if(!(d1.pc1 == d2.pc1)) {
            return false;
        }
        if(d1.type == REDUCE && !(d1.pc2 == d2.pc2)) {
            return false;
        }
        return true;
    }

    PushComputation shift_pc(int x, int i) {
        PushComputation pc;
        pc.i = i, pc.j = i+1, pc.x = x, pc.y = x, pc.z = i;
        pc.jls = pc.jrs = -1;
        pc.yj_connected = 0, pc.jy_connected = 0;
        pc.zj_connected = 0, pc.jz_connected = 0;
        pc.yz_swapped = 0;
        return pc;
    }

    PushComputation singleton_pc(const PushComputation & pc, Transition action, bool inverse) {
        PushComputation newpc(pc);
        int v = (int)!inverse;
        switch(action) {
            case SWAP:
                newpc.yz_swapped = v;
                std::swap(newpc.y, newpc.z);
                std::swap(newpc.zj_connected, newpc.yj_connected);
                std::swap(newpc.jz_connected, newpc.jy_connected);
                break;
            case LEFT_ARC:
                newpc.jz_connected = v;
                if(newpc.jls == -1) {
                    newpc.jls = newpc.jrs = newpc.z;
                } else {
                    newpc.jls = std::min(newpc.jls, newpc.z);
                    newpc.jrs = std::max(newpc.jrs, newpc.z);
                }
                break;
            case RIGHT_ARC:
                newpc.zj_connected = v;
                break;
            default:
                break;
        }
        return newpc;
    }

    PushComputation reduce_pc(const PushComputation & pc1, const PushComputation & pc2) {
        PushComputation pc;
        pc.i = pc1.i, pc.j = pc2.j;
        pc.jls = pc2.jls, pc.jrs = pc2.jrs;
        pc.x = pc1.x; pc.y = pc1.y; pc.z = pc2.y;
        pc.zj_connected = pc2.yj_connected;
        pc.jz_connected = pc2.jy_connected;
        pc.yj_connected = 0;
        pc.jy_connected = 0;
        pc.yz_swapped = 0;
        return pc;
    }

    std::ostream& operator<<(std::ostream& out, const Deduction& d) {
        if(d.type == REDUCE) {
            out<<d.pc1<<" + "<<d.pc2<<"--re-->"<<reduce_pc(d.pc1, d.pc2);
        } else {
            switch(d.type) {
                case SHIFT:
                    out<<d.pc1;
                    break;
                case SWAP:
                    out<<d.pc1<<"--sw-->"<<singleton_pc(d.pc1, d.type);
                    break;
                case LEFT_ARC:
                    out<<d.pc1<<"--la-->"<<singleton_pc(d.pc1, d.type);
                    break;
                case RIGHT_ARC:
                    out<<d.pc1<<"--ra-->"<<singleton_pc(d.pc1, d.type);
                    break;
            }
        }
        return out;
    }

    std::ostream& operator<<(std::ostream& out, const PushComputation& pc) {
        out<<"["<<pc.i<<", "<<pc.x<<", "<<pc.y<<", "<<pc.z<<", "<<pc.j<<", "<<pc.zj_connected<<pc.jz_connected<<pc.yj_connected<<pc.jy_connected<<pc.yz_swapped<<"]";
        return out;
    }

}
