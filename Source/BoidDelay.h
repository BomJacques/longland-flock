#pragma once
#include "Swarm.h"

namespace flock {
// 64 independently positioned read taps, sharing two preallocated recordings.
// Alternating agents read L/R; panning moves those echoes across the stereo field.
struct BoidDelay {
    struct Tap {float delay=1,destination=1,target=1,fade=1,left=0,right=0,targetLeft=0,targetRight=0;};
    std::array<Tap,64> taps{};
    std::array<float,64> echoPeaks{};
    std::array<std::vector<float>,2> buffer;
    int head=0,capacity=0;float sr=48000,smoothing=.001f,fadeStep=.001f,feedback=0,feedbackTarget=0,low=0,dcIn=0,dcOut=0,lowCoefficient=.2f,dcPole=.999f;
    void prepare(double rate,const Snapshot& state,float milliseconds,float spread,float repeats){
        sr=(float)rate;capacity=(int)(sr*3.2f)+4;head=0;low=dcIn=dcOut=0;echoPeaks.fill(0);
        smoothing=1-std::exp(-1/(sr*.025f));fadeStep=1/(sr*.05f);lowCoefficient=1-std::exp(-6.2831853f*3500/sr);dcPole=std::exp(-6.2831853f*15/sr);
        for(auto& channel:buffer)channel.assign((size_t)capacity,0);
        setTargets(state,milliseconds,spread,repeats);feedback=feedbackTarget;
        for(auto& t:taps){t.delay=t.destination=t.target;t.fade=1;t.left=t.targetLeft;t.right=t.targetRight;}
    }
    void setTargets(const Snapshot& state,float milliseconds,float spread,float repeats,bool fromBrain=false){
        milliseconds=clamp(milliseconds,20,1500);spread=clamp(spread,0,1);feedbackTarget=clamp(repeats,0,.85f);
        for(int i=0;i<64;++i){auto& tap=taps[i];const auto& bird=state.birds[i];
            const auto& position=fromBrain?state.echoPositions[i]:bird.p;
            tap.target=clamp(sr*.001f*milliseconds*(1+.9f*spread*position.y),sr*.01f,(float)capacity-2);
            float pan=clamp(position.x,-1,1),level=(.35f+.65f*clamp(bird.energy,0,1))/64;
            tap.targetLeft=level*std::sqrt((1-pan)*.5f);tap.targetRight=level*std::sqrt((1+pan)*.5f);
        }
    }
    float read(int channel,float delay)const{
        // Double arithmetic prevents a tiny negative position from rounding
        // up to capacity when wrapped (which would read past the buffer).
        double position=(double)head-(double)delay;if(position<0)position+=(double)capacity;int a=(int)position;float fraction=(float)(position-a);
        return buffer[channel][(size_t)a]*(1-fraction)+buffer[channel][(size_t)((a+1)%capacity)]*fraction;
    }
    std::array<float,2> tick(float l,float r){
        float left=0,right=0,returned=0;
        for(int i=0;i<64;++i){auto& t=taps[i];float sample;
            if(t.fade>=1&&std::abs(t.target-t.delay)>sr*.02f){t.destination=t.target;t.fade=0;}
            if(t.fade<1){sample=read(i&1,t.delay)*(1-t.fade)+read(i&1,t.destination)*t.fade;t.fade=std::min(1.f,t.fade+fadeStep);if(t.fade==1)t.delay=t.destination;}
            else{t.delay+=clamp(t.target-t.delay,-.003f,.003f);sample=read(i&1,t.delay);}
            t.left+=(t.targetLeft-t.left)*smoothing;t.right+=(t.targetRight-t.right)*smoothing;
            left+=sample*t.left;right+=sample*t.right;
            returned+=sample*(1.f/64);
            echoPeaks[i]=std::max(echoPeaks[i],std::abs(sample));
        }
        feedback+=(feedbackTarget-feedback)*smoothing;
        // Return weights sum to 1, independently of audible level/pan/energy;
        // convex read crossfades and
        // linear interpolation preserve that bound. Feedback is capped at .85.
        low+=lowCoefficient*(returned-low);
        float dc=(1+dcPole)*.5f*(low-dcIn)+dcPole*dcOut;dcIn=low;dcOut=dc;
        for(int c=0;c<2;++c){float value=(c==0?l:r)+feedback*dc;buffer[c][(size_t)head]=std::isfinite(value)?value/(1+.05f*std::abs(value)):0;}
        if(++head==capacity)head=0;return{left,right};
    }
    void describe(Snapshot& state)const{
        for(int i=0;i<64;++i){const auto& t=taps[i];state.tapMs[i]=(t.delay*(1-t.fade)+t.destination*t.fade)*1000/sr;if(t.fade>=1)state.tapMs[i]=t.delay*1000/sr;
            float norm=t.left*t.left+t.right*t.right;state.tapPan[i]=norm>0?(t.right*t.right-t.left*t.left)/norm:0;state.tapLevel[i]=std::sqrt(norm)*64;}
    }
    void transferActivity(Snapshot& state){for(int i=0;i<64;++i){state.echoActivity[i]+=.2f*(clamp(echoPeaks[i]*4,0,1)-state.echoActivity[i]);echoPeaks[i]=0;}}
};
}
