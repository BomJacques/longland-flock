#include "Engine.h"
#include <iostream>
#include <vector>
#include <chrono>

// Schroeder backward integration, fitting -5 to -25 dB (T20 extrapolated RT60).
int main() {
    constexpr int sr=48000, count=sr*16;
    double shortest=100,longest=0;
    for(float diffusion:{.6f,.8f,.95f}) {
        flock::Reverb engine;engine.prepare(sr);
        // Settle coefficients and delay taps before the impulse.
        for(int n=0;n<sr;++n)engine.tick(0,0,1,6,.2f,1,0,0,diffusion);
        std::vector<double> energy(count);
        for(int n=0;n<count;++n){auto w=engine.tick(n==0?.1f:0,0,1,6,.2f,1,0,0,diffusion);energy[n]=w[0]*w[0]+w[1]*w[1];}
        for(int n=count-2;n>=0;--n)energy[n]+=energy[n+1];
        double total=energy[0],sx=0,sy=0,sxx=0,sxy=0;int points=0;
        for(int n=0;n<count;++n){double db=10*std::log10(std::max(1e-30,energy[n]/total));if(db<-5&&db>-25){double t=(double)n/sr;sx+=t;sy+=db;sxx+=t*t;sxy+=t*db;++points;}}
        double slope=(points*sxy-sx*sy)/(points*sxx-sx*sx);
        std::cout<<"Diffusion "<<diffusion<<", nominal RT60 6 s: measured broadband T20 RT60 "<<-60/slope<<" s\n";
        if(!std::isfinite(slope)||slope>=0)return 1;
        double decay=-60/slope;shortest=std::min(shortest,decay);longest=std::max(longest,decay);
    }
    if(shortest<3||longest>8||longest/shortest>1.15)return 2;
    std::cout<<"Decay remains consistent across diffusion settings\n";
}
