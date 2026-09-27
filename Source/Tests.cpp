#include "Engine.h"
#include "Swarm.h"
#include <iostream>
int main() {
    for(int d=0;d<=100;++d)for(int basis=0;basis<8;++basis){
        std::array<float,8> input{};input[basis]=1;
        auto output=flock::scatter(input,std::cos(d*.01f*.785398163f),std::sin(d*.01f*.785398163f));
        float norm=0;for(float x:output)norm+=x*x;if(std::abs(norm-1)>1e-5f)return 9;
        for(int other=0;other<basis;++other){std::array<float,8> second{};second[other]=1;auto transformed=flock::scatter(second,std::cos(d*.01f*.785398163f),std::sin(d*.01f*.785398163f));float dot=0;for(int k=0;k<8;++k)dot+=output[k]*transformed[k];if(std::abs(dot)>1e-5f)return 10;}
    }
    std::cout<<"Diffusion preserves energy and orthogonality across its full range\n";
    flock::Swarm swarm;
    for(int i=0;i<2000;++i) {
        swarm.update({.7f,.45f,.55f,.25f,.6f,.3f,.15f},.3f,i%100==0?1.f:0.f,.5f,1,1,{.2f,.1f,0},{-.4f,0,0},false);
        for(auto& b:swarm.state.birds)if(!std::isfinite(b.p.x)||b.p.length()>1.001f)return 3;
    }
    std::cout << "Swarm bounds pass over 20 simulated seconds\n";
    for(float strength:{0.f,1.f}){
        flock::Swarm extreme;
        for(int i=0;i<3000;++i){extreme.update({strength,strength,strength,strength,strength,strength,strength},1,1,1,2,3,{.85f,.85f,0},{-.85f,-.85f,0},false,i%2?.15f:.8f);
            for(auto& b:extreme.state.birds)if(!std::isfinite(b.p.x)||b.p.length()>1.001f||b.v.length()>.631f)return 4;}
    }
    flock::Swarm goal;float initial=0;for(auto& b:goal.state.birds)initial+=b.p.length();
    for(int i=0;i<2000;++i)goal.update({0,0,0,1,0,0,0},.5f,0,0,0,1,{},{},false);
    float finalDistance=0;for(auto& b:goal.state.birds)finalDistance+=b.p.length();if(finalDistance>=initial*.5f)return 5;
    auto frozen=goal.state.birds;goal.update({1,1,1,1,1,1,1},1,1,1,1,1,{},{},true);
    for(int i=0;i<64;++i)if((frozen[i].p-goal.state.birds[i].p).length()!=0)return 6;
    flock::Swarm drum,pad;
    for(int i=0;i<600;++i){drum.update({.7f,.45f,.55f,.25f,1,.3f,.15f},.3f,i%50==0?1.f:0.f,.3f,2,1,{},{},false);pad.update({.7f,.45f,.55f,.25f,1,.3f,.15f},.3f,0,.3f,2,1,{},{},false);}
    if(std::abs(drum.state.spread-pad.state.spread)<.01f)return 7;
    std::cout<<"Zero/max rules, goal convergence, freeze and distinct transient response pass\n";
    for(double sr:{44100.,48000.,96000.}) {
        flock::Reverb e; e.prepare(sr); double early=0,late=0;
        for(int i=0;i<(int)(sr*5);++i) { auto w=e.tick(i==0?1.f:0.f,i==0?1.f:0.f,1,1,.5f,1,0,0,.8f); for(float x:w) { if(!std::isfinite(x)||std::abs(x)>2)return 1; if(i<sr)early+=x*x; if(i>4*sr)late+=x*x; } }
        if(early<=0 || late>early*.001) return 2;
        std::cout << "Impulse decay passes at " << sr << " Hz\n";
        e.prepare(sr);uint32_t rng=7;
        for(int i=0;i<(int)(sr*8);++i){rng=1664525u*rng+1013904223u;float noise=(float)(rng>>8)/8388608.f-1;auto w=e.tick(i<sr*3?noise:0,0,i%20000<10000?.4f:3.5f,20,i%7000<3500?0:1,2,1,std::sin(i*.0001f)*5,i%2000<1000?0:1);for(float x:w)if(!std::isfinite(x)||std::abs(x)>10)return 8;}
        std::cout<<"Extreme delay/damping/diffusion automation remains bounded at "<<sr<<" Hz\n";
    }
}
