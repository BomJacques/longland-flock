#pragma once
#include <array>
#include <algorithm>
namespace flock {
using BrainOrder=std::array<int,9>;
constexpr int brainPermutationCount=362880;
inline BrainOrder decodeOrder(int rank){
    rank=std::max(0,std::min(brainPermutationCount-1,rank));
    BrainOrder remaining{0,1,2,3,4,5,6,7,8},result{};int available=9,factorial=40320;
    for(int i=0;i<9;++i){int index=rank/factorial;rank%=factorial;result[i]=remaining[index];for(int j=index;j<available-1;++j)remaining[j]=remaining[j+1];--available;if(available>0)factorial/=available;}
    return result;
}
inline int encodeOrder(const BrainOrder& order){
    int rank=0,factorial=40320;for(int i=0;i<9;++i){int smaller=0;for(int j=i+1;j<9;++j)if(order[j]<order[i])++smaller;rank+=smaller*factorial;if(i<8)factorial/=8-i;}return rank;
}
inline BrainOrder moveStage(BrainOrder order,int from,int to){
    if(from<0||from>=9||to<0||to>=9)return order;int stage=order[from];
    if(from<to)for(int i=from;i<to;++i)order[i]=order[i+1];else for(int i=from;i>to;--i)order[i]=order[i-1];order[to]=stage;return order;
}
}
