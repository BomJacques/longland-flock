#pragma once
#include <fstream>
#include <iomanip>

// Offline-only synthetic audition material. No file I/O is used by the plugin.
inline bool writeWave(const juce::File& file,const juce::AudioBuffer<float>& buffer) {
    std::ofstream out(file.getFullPathName().toStdString(),std::ios::binary|std::ios::trunc);
    auto little=[&](uint32_t x,int bytes){for(int n=0;n<bytes;++n)out.put((char)((x>>(8*n))&255));};
    uint32_t bytes=(uint32_t)buffer.getNumSamples()*6;
    out.write("RIFF",4);little(36+bytes,4);out.write("WAVEfmt ",8);little(16,4);little(1,2);little(2,2);little(48000,4);little(48000*6,4);little(6,2);little(24,2);out.write("data",4);little(bytes,4);
    for(int n=0;n<buffer.getNumSamples();++n)for(int ch=0;ch<2;++ch){int32_t sample=(int32_t)std::lround(flock::clamp(buffer.getSample(ch,n),-1,1)*8388607.f);little((uint32_t)sample,3);}
    return out.good();
}

inline int renderAuditions(const juce::File& folder) {
    if(!folder.createDirectory())return 20;
    constexpr int sr=48000,block=128,count=sr*14;
    std::ofstream report(folder.getChildFile("Listening-guide.md").getFullPathName().toStdString());
    report<<"# Longland Flock 0.4 listening examples\n\n48 kHz / 24-bit stereo. Each file contains six seconds of synthetic input, then eight seconds for the tail. The dry and processed files use identical sources without loudness normalisation. Preset files use their own Mix setting. Boid delay only files disable reverb and use 65% global wet mix, 100% delay level and 50% feedback. Compare at the same playback level.\n\nDrums: compare scattering and discrete reflections in Panic with coherent movement in Murmuration. Pad: compare the darker long tail in Dead Mall with the flowing Migration setting. Scatter then Echo and Echo then Scatter use identical settings and sources, changing only the order of those two modules. These files are for human listening; passing signal checks is not a perceptual sound-quality verdict.\n\n| File | Peak dBFS | Tail RMS dBFS (6-14 s) | Maximum spread | Maximum transient pulse |\n|---|---:|---:|---:|---:|\n";
    for(int source=0;source<2;++source){
        juce::AudioBuffer<float> dry(2,count);dry.clear();uint32_t rng=42;
        for(int n=0;n<sr*6;++n){float t=(float)n/sr;rng=rng*1664525u+1013904223u;float noise=(float)(rng>>8)/8388608.f-1;float l=0,r=0;
            if(source==0){float beat=std::fmod(t,.5f),hat=std::fmod(t,.25f),snare=std::fmod(t+.5f,1.f);
                float kick=.48f*std::sin(6.2831853f*(48*beat+2.5f*(1-std::exp(-beat*30))))*std::exp(-beat*20);
                float clap=.22f*noise*std::exp(-snare*35),hiss=.07f*noise*std::exp(-hat*95);l=kick+clap+hiss;r=kick+clap-hiss*.7f;
            }else{float env=std::min(1.f,t/1.2f)*std::min(1.f,(6-t)/1.2f);for(float hz:{130.8128f,164.8138f,195.9977f,261.6256f}){l+=.075f*(std::sin(6.2831853f*hz*t)+.15f*std::sin(12.5663706f*hz*t))*env;r+=.075f*(std::sin(6.2831853f*hz*1.001f*t)+.15f*std::sin(12.5663706f*hz*1.001f*t))*env;}}
            dry.setSample(0,n,l);dry.setSample(1,n,r);
        }
        for(int variant=0;variant<6;++variant){
            int preset=source==0?(variant==1?1:2):(variant==1?3:4);
            juce::String name=source==0?"Drums":"Pad";name+=" - ";name+=variant==0?"Dry":(variant==3?"Boid delay only":(variant==4?"Scatter then Echo":(variant==5?"Echo then Scatter":presetNames[preset])));name+=".wav";
            juce::AudioBuffer<float> output(2,count),chunk(2,block);FlockProcessor processor;processor.preset(preset);if(variant>=3){processor.set("reverbLevel",0);processor.set("delayLevel",1);processor.set("delaySend",0);processor.set("mix",.65f);processor.set("delayTime",source==0?320:450);processor.set("delayFeedback",.5f);processor.set("delaySpread",.5f);if(variant>=4){processor.set("scatterOn",1);processor.set("scatter",1);processor.set("echoDrive",1);if(variant==5)processor.reorderStage(8,7);}}processor.prepareToPlay(sr,block);juce::MidiBuffer midi;
            float maxSpread=0,maxPulse=0;double peak=0,tailEnergy=0;
            for(int base=0;base<count;base+=block){for(int ch=0;ch<2;++ch)chunk.copyFrom(ch,0,dry,ch,base,block);if(variant)processor.processBlock(chunk,midi);
                maxSpread=std::max(maxSpread,processor.swarm.state.spread);maxPulse=std::max(maxPulse,processor.swarm.state.pulse);
                for(int ch=0;ch<2;++ch){output.copyFrom(ch,base,chunk,ch,0,block);for(int i=0;i<block;++i){float x=chunk.getSample(ch,i);if(!std::isfinite(x)||std::abs(x)>1)return 21;peak=std::max(peak,(double)std::abs(x));if(base+i>=sr*6)tailEnergy+=x*x;}}
            }
            if(!writeWave(folder.getChildFile(name),output))return 22;
            auto db=[](double x){return 20*std::log10(std::max(1e-9,x));};
            report<<"| ["<<name<<"]("<<name.replace(" ","%20")<<") | "<<std::fixed<<std::setprecision(2)<<db(peak)<<" | "<<db(std::sqrt(tailEnergy/(sr*8*2)))<<" | "<<maxSpread<<" | "<<maxPulse<<" |\n";
            std::cout<<"Rendered "<<name<<"\n";
        }
    }
    return report.good()?0:23;
}


