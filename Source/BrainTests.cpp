#include "Swarm.h"
#include <iostream>
int main(){
    for(int rank=0;rank<flock::brainPermutationCount;++rank){auto order=flock::decodeOrder(rank);auto sorted=order;std::sort(sorted.begin(),sorted.end());for(int i=0;i<9;++i)if(sorted[i]!=i)return 1;if(flock::encodeOrder(order)!=rank)return 2;}
    std::cout<<"All 362880 Brain orders encode/decode without duplicate stages\n";
    auto late=flock::decodeOrder(0),early=flock::moveStage(late,7,0),echoFirst=flock::moveStage(late,8,0);
    flock::Swarm a,b,c;std::array<float,7> weights{1,.7f,.8f,.5f,.7f,.8f,.4f};
    for(int step=0;step<1600;++step){for(auto* swarm:{&a,&b,&c})for(auto& energy:swarm->state.echoActivity)energy=.3f;
        a.update(weights,.4f,step%100==0?1.f:0,.3f,1,1,{.3f,.2f,0},{-.3f,0,0},false,.45f,&late,1,1,true);
        b.update(weights,.4f,step%100==0?1.f:0,.3f,1,1,{.3f,.2f,0},{-.3f,0,0},false,.45f,&early,1,1,true);
        c.update(weights,.4f,step%100==0?1.f:0,.3f,1,1,{.3f,.2f,0},{-.3f,0,0},false,.45f,&echoFirst,1,1,true);
        for(auto* swarm:{&a,&b,&c})for(int i=0;i<64;++i){if(!std::isfinite(swarm->state.birds[i].p.x)||swarm->state.birds[i].p.length()>1||swarm->state.echoPositions[i].length()>1)return 3;}
    }
    float movementDifference=0,echoDifference=0;for(int i=0;i<64;++i){movementDifference+=(a.state.birds[i].p-b.state.birds[i].p).length()/64;echoDifference+=(a.state.echoPositions[i]-c.state.echoPositions[i]).length()/64;}
    std::cout<<"Moving Scatter changes mean boid positions by "<<movementDifference<<"; moving Echo changes capture positions by "<<echoDifference<<"\n";
    if(movementDifference<.03f||echoDifference<.03f)return 4;
    flock::Swarm calm,burst;
    for(int i=0;i<500;++i){calm.update(weights,.4f,0,.2f,1,1,{},{},false,.45f,&late,0,0,true);burst.update(weights,.4f,0,.2f,1,1,{},{},false,.45f,&late,1,0,true);}
    std::cout<<"Adding Scatter changes spread from "<<calm.state.spread<<" to "<<burst.state.spread<<"\n";
    if(burst.state.spread<calm.state.spread+.05f)return 5;
    std::cout<<"Ordered Brain, Echo capture, Scatter and bounded movement PASS\n";
}
