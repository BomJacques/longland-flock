#pragma once
#include "Engine.h"
#include "BrainOrder.h"
namespace flock {
struct Vec {
    float x=0,y=0,z=0;
    Vec operator+(Vec b)const{return{x+b.x,y+b.y,z+b.z};}
    Vec operator-(Vec b)const{return{x-b.x,y-b.y,z-b.z};}
    Vec operator*(float s)const{return{x*s,y*s,z*s};}
    float length()const{return std::sqrt(x*x+y*y+z*z);}
    Vec limited(float m)const{float n=length();return n>m?*this*(m/n):*this;}
};
struct Boid { Vec p,v; std::array<Vec,9> forces{}; float energy=0; };
struct Snapshot {
    std::array<Boid,64> birds{};
    Vec centre{},goal{},obstacle{};
    std::array<float,9> activity{};
    std::array<Vec,64> echoPositions{};
    std::array<float,64> echoActivity{};
    std::array<float,64> tapMs{},tapPan{},tapLevel{};
    float spread=0,density=0,speed=0,energy=0,pulse=0,width=1,diffusion=.8f,delayScale=1,modulation=0;
};
struct Swarm {
    Snapshot state; uint32_t seed=0x193af; float clock=0;
    float random(){seed=1664525u*seed+1013904223u;return (float)(seed>>8)/16777216.f*2-1;}
    void reset(){state={};seed=0x193af;clock=0;for(int i=0;i<64;++i){auto& b=state.birds[i];b.p=Vec{random()*.75f,random()*.75f,random()*.75f}.limited(.95f);b.v=Vec{random()*.12f,random()*.12f,random()*.12f}.limited(.08f);state.echoPositions[i]=b.p;}}
    Swarm(){reset();}
    void update(const std::array<float,7>& weights,float input,float transient,float brightness,float response,float energyGain,Vec goal,Vec obstacle,bool freeze,float radius=.45f,const BrainOrder* order=nullptr,float scatter=0,float echoDrive=.75f,bool echoEnabled=true) {
        state.goal=goal;state.obstacle=obstacle;
        if(freeze)return;
        constexpr float dt=.01f; clock+=dt;
        state.pulse=std::max(state.pulse*.93f,clamp(transient*response*8,0,1));
        state.energy+=.035f*(clamp(input*energyGain*3,0,1)-state.energy);
        auto next=state.birds;state.activity.fill(0);
        for(int i=0;i<64;++i){auto& b=state.birds[i];auto& n=next[i];Vec centre{},velocity{},separate{};int count=0;
            for(int j=0;j<64;++j)if(i!=j){Vec delta=b.p-state.birds[j].p;float d=delta.length();if(d<radius){centre=centre+state.birds[j].p;velocity=velocity+state.birds[j].v;++count;if(d<.18f)separate=separate+delta*(1/std::max(.006f,d*d));}}
            std::array<Vec,9> f{};
            if(count){f[0]=(centre*(1.f/count)-b.p)*1.5f;f[2]=(velocity*(1.f/count)-b.v)*2;}
            f[1]=separate*.11f;
            f[3]=(goal+Vec{0,0,brightness*.3f}-b.p)*(.3f+state.energy);
            Vec away=b.p-obstacle;float d=away.length();if(d<.5f)f[4]=away*(3*(.5f-d)/std::max(.03f,d));
            Vec predator=b.p;float pd=predator.length();f[4]=f[4]+predator*(state.pulse*2/std::max(.06f,pd));
            f[5]=i==0?Vec{std::cos(clock*.27f)*.5f,std::sin(clock*.33f)*.5f,.15f*std::sin(clock*.19f)}-b.p:(state.birds[0].p-b.p)*.8f+(state.birds[0].v-b.v)*.6f;
            f[6]={std::sin(clock*.7f+i*2.4f)*.35f,std::cos(clock*.61f+i*1.7f)*.35f,std::sin(clock*.53f+i)*.2f};
            Vec radial=b.p-state.centre;float radialLength=radial.length();
            f[7]=radial*(1/std::max(.05f,radialLength))*(.6f+2.5f*state.pulse+1.5f*state.energy);
            Vec force{};n.forces.fill({});n.energy=state.energy;
            if(order){
                state.echoPositions[i]=b.p;
                for(int k:*order){
                    if(k==8){if(echoEnabled){
                        // Audible echoes add propulsion and energy at THIS chain position.
                        Vec impulse=b.v.limited(.63f)*(state.echoActivity[i]*echoDrive*2);
                        force=(force+impulse).limited(2);n.forces[8]=impulse;
                        state.echoPositions[i]=(b.p+(b.v+force*.35f)*(.7f*echoDrive)).limited(.98f);
                        n.energy=clamp(state.energy+state.echoActivity[i]*echoDrive*.65f,0,1);
                        state.activity[8]+=state.echoActivity[i]/64;
                    }continue;}
                    float weight=k==7?scatter:weights[k];
                    Vec contribution=f[k].limited(k==7?3.f:1.5f)*weight;
                    // Later active steering reshapes the previous result: order is audible.
                    force=(force*(1-.55f*clamp(contribution.length()/1.5f,0,1))+contribution).limited(2);
                    n.forces[k]=contribution;state.activity[k]+=contribution.length()/64;
                }
            }else{for(int k=0;k<7;++k){n.forces[k]=f[k].limited(1.5f)*weights[k];force=force+n.forces[k];state.activity[k]+=n.forces[k].length()/64;}}
            // Containment is independent of user rules.
            force=force-b.p*(std::max(0.f,b.p.length()-.78f)*5);
            n.v=(b.v*.992f+force.limited(2)*dt).limited(.08f+n.energy*.55f);
            n.p=b.p+n.v*dt;
            if(n.p.length()>.98f){n.p=n.p.limited(.98f);n.v=n.v*-.25f;}
            if(!order)state.echoPositions[i]=n.p;
        }
        state.birds=next;state.centre={};state.speed=0;float neighbours=0;
        for(auto& b:next){state.centre=state.centre+b.p*(1.f/64);state.speed+=b.v.length()/64;}
        state.spread=0;for(auto& b:next)state.spread+=(b.p-state.centre).length()/64;
        for(int i=0;i<64;++i)for(int j=i+1;j<64;++j)if((next[i].p-next[j].p).length()<radius)neighbours+=2;
        state.density=neighbours/(64*63);
    }
};
}
