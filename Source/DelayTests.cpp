#include "BoidDelay.h"
#include <iostream>
int main(){
    for(int sr:{44100,48000,96000}){
        flock::Snapshot state;for(auto& b:state.birds)b.energy=1;
        flock::BoidDelay delay;delay.prepare(sr,state,100,0,0);float early=0,peak=0;
        for(int n=0;n<sr/3;++n){auto out=delay.tick(n==0?.1f:0,n==0?.1f:0);if(n<sr/10-1)early=std::max(early,std::abs(out[0]));if(std::abs(n-sr/10)<2)peak=std::max(peak,std::abs(out[0]));}
        if(early>1e-7f||peak<.05f){std::cerr<<"Arrival failed at "<<sr<<": early="<<early<<", peak="<<peak<<", delay="<<delay.taps[0].delay<<"\n";return 1;}
        // All 64 agents must produce a distinct, measurable arrival.
        for(int i=0;i<64;++i){state.birds[i].p={-.8f,-.9f+1.8f*i/63,0};}
        delay.prepare(sr,state,300,.8f,0);std::vector<float> left(sr),right(sr);
        for(int n=0;n<sr;++n){auto out=delay.tick(n==0?.1f:0,n==0?.1f:0);left[n]=out[0];right[n]=out[1];}
        for(int i=0;i<64;++i){int arrival=(int)delay.taps[i].target;float l=0,r=0;for(int n=arrival-1;n<=arrival+2;++n){l+=std::abs(left[n]);r+=std::abs(right[n]);}if(l<.0005f||l<r*2){std::cerr<<"Tap "<<i<<" failed: L="<<l<<" R="<<r<<"\n";return 2;}}
        delay.prepare(sr,state,300,1,.85f);uint32_t rng=73;double earlyEnergy=0,lateEnergy=0;
        for(int n=0;n<sr*14;++n){if(n%(sr/100)==0){for(int i=0;i<64;++i){state.birds[i].p.y=.9f*std::sin(n*.0002f+i);state.birds[i].p.x=.9f*std::cos(n*.00017f+i);}delay.setTargets(state,n<sr*4?(n%(sr/2)<sr/4?20:1500):300,1,.85f);}
            rng=1664525u*rng+1013904223u;float x=n<sr*4?((float)(rng>>8)/8388608.f-1)*.8f:0;auto out=delay.tick(x,-x*.8f);
            for(float sample:out){if(!std::isfinite(sample)||std::abs(sample)>2)return 3;if(n>=sr*4&&n<sr*5)earlyEnergy+=sample*sample;if(n>=sr*13)lateEnergy+=sample*sample;}}
        if(earlyEnergy<=0||lateEnergy>earlyEnergy*.005){std::cerr<<"Decay failed: "<<earlyEnergy<<" -> "<<lateEnergy<<"\n";return 4;}
        delay.describe(state);for(float ms:state.tapMs)if(!std::isfinite(ms)||ms<9.9f||ms>3000)return 5;
        delay.prepare(sr,state,100,0,0);for(int n=0;n<sr/5;++n){auto out=delay.tick(0,0);if(out[0]!=0||out[1]!=0)return 6;}
        std::cout<<"Boid delay passes at "<<sr<<" Hz: arrival time, 64 separate echoes, pan, automation, maximum feedback decay, reset\n";
    }
}

