#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace flock {
inline float clamp(float x,float a,float b) { return std::max(a,std::min(b,x)); }
// Three stages of paired rotations followed by a permutation. Each stage
// preserves energy for every diffusion setting, including while it changes.
inline std::array<float,8> scatter(std::array<float,8> values,float cosine,float sine) {
    for(int stride=1;stride<=4;stride*=2)
        for(int base=0;base<8;base+=stride*2)
            for(int j=0;j<stride;++j){int a=base+j,b=a+stride;float x=values[a],y=values[b];values[a]=cosine*x+sine*y;values[b]=cosine*y-sine*x;}
    return {values[1],values[2],values[3],values[4],values[5],values[6],values[7],values[0]};
}
struct Reverb {
    std::array<std::vector<float>,8> lines;
    std::array<float,8> delay{},low{},dcIn{},dcOut{};
    std::array<float,8> targetDelay{},gain{},targetGain{};
    int head=0, capacity=0,control=0; float sr=48000,coefficient=.2f,targetCoefficient=.2f,smoothWidth=1,smoothDiffusion=.8f;
    float rotationCos=.80901699f,rotationSin=.58778525f;
    const std::array<float,8> times{.029f,.037f,.043f,.053f,.061f,.071f,.079f,.083f};
    void prepare(double rate) {
        sr=(float)rate; capacity=(int)(sr*.6)+4; head=0;
        for(int i=0;i<8;++i) { lines[i].assign((size_t)capacity,0); delay[i]=times[i]*sr; }
        low.fill(0); dcIn.fill(0); dcOut.fill(0); targetDelay=delay; gain.fill(.8f);targetGain=gain;control=0;coefficient=targetCoefficient=.2f;smoothWidth=1;smoothDiffusion=.8f;
        rotationCos=std::cos(smoothDiffusion*.785398163f);rotationSin=std::sin(smoothDiffusion*.785398163f);
    }
    std::array<float,2> tick(float l,float r,float size,float decay,float damping,float width,float motion,float phase,float diffusion) {
        std::array<float,8> v{};
        if(control++%64==0){
            float cutoff=std::min(sr*.45f,1200+15000*(1-damping)*(1-damping));
            targetCoefficient=1-std::exp(-6.2831853f*cutoff/sr);
            rotationCos=std::cos(clamp(smoothDiffusion,0,1)*.785398163f);
            rotationSin=std::sin(clamp(smoothDiffusion,0,1)*.785398163f);
            for(int i=0;i<8;++i){targetDelay[i]=clamp(sr*times[i]*size+motion*sr*.00035f*std::sin(phase*(1+.13f*i)+i),2,(float)capacity-2);
                targetGain[i]=std::min(.995f,std::exp(-6.907755f*delay[i]/(sr*std::max(.2f,decay))));}
            control=1;
        }
        coefficient+=(targetCoefficient-coefficient)*.002f;
        smoothWidth+=(width-smoothWidth)*.002f; smoothDiffusion+=(diffusion-smoothDiffusion)*.002f;
        for(int i=0;i<8;++i) {
            delay[i]+=clamp(targetDelay[i]-delay[i],-.002f,.002f);
            double pos=(double)head-(double)delay[i]; if(pos<0) pos+=(double)capacity;
            int p=(int)pos; float f=(float)(pos-p);
            float x=lines[i][(size_t)p]*(1-f)+lines[i][(size_t)((p+1)%capacity)]*f;
            low[i]+=coefficient*(x-low[i]);
            // Loop DC blocker is scaled so its maximum frequency gain is unity.
            float y=.9975f*(low[i]-dcIn[i])+.995f*dcOut[i]; dcIn[i]=low[i]; dcOut[i]=y;
            v[i]=y;
        }
        auto feedback=scatter(v,rotationCos,rotationSin);
        float outL=0,outR=0;
        for(int i=0;i<8;++i) {
            gain[i]+=(targetGain[i]-gain[i])*.002f;
            float input=.16f*((i&1)?r:l);
            float value=input+gain[i]*feedback[i];
            lines[i][(size_t)head]=std::isfinite(value)? value/(1+.035f*std::abs(value)):0;
            outL+=v[i]*((i&1)?-.25f:.25f); outR+=v[i]*((i&2)?-.25f:.25f);
        }
        if(++head==capacity) head=0;
        float mid=(outL+outR)*.5f, side=(outL-outR)*.5f*smoothWidth;
        return {mid+side,mid-side};
    }
};
}
