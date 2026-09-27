#include <juce_audio_utils/juce_audio_utils.h>
#include "Swarm.h"
#include "BoidDelay.h"
#include <memory>

namespace {
const std::array<const char*,7> ruleIds{"flock","separation","alignment","goal","avoid","follow","wander"};
const std::array<const char*,7> ruleNames{"Flock","Separation","Alignment","Goal","Avoid","Follow leader","Wander"};
const std::array<const char*,12> mainIds{"mix","decay","size","damping","energy","transient","motion","width","radius","goalX","goalY","obstacleX"};
const std::array<const char*,12> mainNames{"Mix","Decay / seconds","Space size","Damping","Swarm energy","Transient response","Motion","Stereo width","Neighbour radius","Goal X","Goal Y","Obstacle X"};
const std::array<const char*,6> delayIds{"delayTime","delaySpread","delayFeedback","delayLevel","delaySend","reverbLevel"};
const std::array<const char*,6> delayNames{"Delay time / ms","Tap spread","Delay feedback","Delay level","Delay > reverb","Reverb level"};
const std::array<const char*,7> presetNames{"Still Air","Murmuration","Panic","Dead Mall","Migration","Fireflies","Collapse"};
const juce::Colour bg(0xff11191e),panel(0xff1a252c),ink(0xffe4ecee),muted(0xff91a5ae),mint(0xff7ee0be);
const std::array<juce::Colour,7> colours{juce::Colour(0xff7ee0be),juce::Colour(0xffffba79),juce::Colour(0xff7fc9ff),juce::Colour(0xffe7da91),juce::Colour(0xffff7d87),juce::Colour(0xffc8a1ff),juce::Colour(0xffa4b5bc)};
}

class FlockEditor;
class FlockProcessor final : public juce::AudioProcessor {
public:
    static juce::AudioProcessorValueTreeState::ParameterLayout layout(){
        juce::AudioProcessorValueTreeState::ParameterLayout p;
        auto add=[&](const juce::String& id,const juce::String& name,float lo,float hi,float def){p.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{id,1},name,juce::NormalisableRange<float>(lo,hi,.001f),def));};
        add("mix","Mix",0,1,.35f);add("decay","Decay",.2f,20,3);add("size","Size",.5f,3,1);add("damping","Damping",0,1,.5f);
        add("energy","Swarm energy",0,3,1);add("transient","Transient response",0,2,1);add("motion","Motion",0,1,.35f);add("width","Width",0,2,1);
        add("radius","Neighbour radius",.15f,.8f,.45f);add("goalX","Goal X",-.85f,.85f,.3f);add("goalY","Goal Y",-.85f,.85f,.2f);add("obstacleX","Obstacle X",-.85f,.85f,-.35f);add("obstacleY","Obstacle Y",-.85f,.85f,-.1f);
        const float defaults[]{.7f,.45f,.55f,.25f,.6f,.3f,.15f};
        for(int i=0;i<7;++i){add(ruleIds[i],ruleNames[i],0,1,defaults[i]);p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{juce::String(ruleIds[i])+"On",1},juce::String(ruleNames[i])+" enabled",true));}
        p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"freeze",1},"Freeze ecosystem",false));
        add("delayTime","Delay time",20,1500,300);add("delaySpread","Delay spread",0,1,.45f);add("delayFeedback","Delay feedback",0,.85f,.35f);add("delayLevel","Delay level",0,1,.5f);add("delaySend","Delay reverb send",0,1,.35f);add("reverbLevel","Reverb level",0,1,1);add("scatter","Scatter",0,1,.85f);p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"scatterOn",1},"Scatter enabled",false));add("echoDrive","Echo brain drive",0,1,.75f);p.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"echoOn",1},"Echo enabled",true));p.add(std::make_unique<juce::AudioParameterInt>(juce::ParameterID{"brainOrder",1},"Brain order",0,flock::brainPermutationCount-1,0));return p;
    }
    juce::AudioProcessorValueTreeState parameters;
    flock::Reverb engine;flock::Swarm swarm;flock::BoidDelay boidDelay;
    // A single producer (audio) and consumer (editor). Full queue drops pictures, never audio.
    juce::AbstractFifo queue{8};std::array<flock::Snapshot,8> pictures;
    std::array<std::atomic<float>*,39> raw{};
    float delayLevel=.5f,delaySend=.35f,reverbLevel=1,scatterStrength=0,echoDrive=.75f;
    float rate=48000,env=0,previous=0,bright=0,fast=0,slow=0,phase=0,mix=.35f;
    float analysisCoeff=.001f,slowCoeff=.0001f,mixCoeff=.001f;
    float controlClock=0,peak=0,mapWidth=1,mapDiffusion=.8f,mapDelay=1,mapMotion=0,mapDamping=.5f;
    std::array<float,7> smoothWeights{};int pictureCount=0;
    FlockProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),parameters(*this,nullptr,"FlockState",layout()){
        for(int i=0;i<12;++i)raw[i]=parameters.getRawParameterValue(mainIds[i]);
        raw[12]=parameters.getRawParameterValue("obstacleY");raw[13]=parameters.getRawParameterValue("freeze");
        for(int i=0;i<7;++i){raw[14+i]=parameters.getRawParameterValue(ruleIds[i]);raw[21+i]=parameters.getRawParameterValue(juce::String(ruleIds[i])+"On");}
        for(int i=0;i<6;++i)raw[28+i]=parameters.getRawParameterValue(delayIds[i]);
        const char* extra[]{"scatter","scatterOn","echoDrive","echoOn","brainOrder"};for(int i=0;i<5;++i)raw[34+i]=parameters.getRawParameterValue(extra[i]);
        static_assert(std::atomic<float>::is_always_lock_free,"Audio parameters must be lock-free");
    }
    float value(int i)const{float v=raw[i]->load(std::memory_order_relaxed);return std::isfinite(v)?v:0;}
    void set(const juce::String& id,float v){if(auto* p=parameters.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();}}
    void preset(int n){
        // Every sound parameter is specified so presets do not inherit hidden prior settings.
        const float rules[7][7]={{.5f,.4f,.7f,.2f,.3f,.1f,.03f},{.85f,.4f,.8f,.35f,.25f,.4f,.15f},{.4f,.85f,.3f,.15f,1,.1f,.4f},{.5f,.6f,.7f,.2f,.2f,.2f,.08f},{.6f,.45f,.8f,.2f,.25f,1,.1f},{.25f,.8f,.2f,.1f,.3f,.1f,.8f},{1,.65f,.3f,.8f,.6f,.1f,.1f}};
        const float settings[7][8]={{.3f,2,1,.45f,.5f,.2f,.05f,.8f},{.4f,4,1.4f,.4f,1,.7f,.4f,1.2f},{.45f,5,1.3f,.3f,2,2,.8f,1.7f},{.5f,9,2.4f,.9f,.6f,.3f,.2f,1.6f},{.4f,5,1.7f,.45f,1.2f,.5f,.5f,1.3f},{.4f,3,.8f,.35f,1.4f,.8f,.6f,1.6f},{.5f,6,1.5f,.6f,1.4f,1.4f,.6f,1.2f}};
        n=juce::jlimit(0,6,n);for(int i=0;i<8;++i)set(mainIds[i],settings[n][i]);for(int i=0;i<7;++i){set(ruleIds[i],rules[n][i]);set(juce::String(ruleIds[i])+"On",1);}
        set("radius",.45f);set("goalX",.3f);set("goalY",.2f);set("obstacleX",-.35f);set("obstacleY",-.1f);set("freeze",0);
        const float echoes[7][6]={{120,.15f,.12f,.15f,.2f,1},{320,.4f,.4f,.5f,.4f,1},{220,.9f,.5f,.65f,.5f,1},{550,.65f,.5f,.35f,.65f,1},{450,.35f,.6f,.6f,.4f,.8f},{180,.8f,.3f,.65f,.25f,.75f},{380,.7f,.55f,.5f,.5f,1}};for(int i=0;i<6;++i)set(delayIds[i],echoes[n][i]);set("brainOrder",0);set("scatter",.85f);set("scatterOn",n==2||n==6?1.f:0.f);set("echoOn",1);set("echoDrive",n==0?.25f:.75f);
    }
    flock::BrainOrder brainOrder()const{return flock::decodeOrder((int)value(38));}
    void reorderStage(int stage,int destination){auto order=brainOrder();int from=0,to=0;for(int i=0;i<9;++i){if(order[i]==stage)from=i;if(order[i]==destination)to=i;}set("brainOrder",(float)flock::encodeOrder(flock::moveStage(order,from,to)));}
    bool stageEnabled(int stage)const{return value(stage<7?21+stage:(stage==7?35:37))>.5f;}
    const juce::String getName()const override{return "Longland Flock";}
    void prepareToPlay(double sr,int)override{
        rate=(float)sr;engine.prepare(sr);swarm.reset();env=previous=bright=fast=slow=phase=peak=controlClock=0;pictureCount=0;
        // Do not reset the FIFO here: an editor may currently be consuming it.
        analysisCoeff=1-std::exp(-1/(rate*.008f));slowCoeff=1-std::exp(-1/(rate*.12f));mixCoeff=1-std::exp(-1/(rate*.025f));
        boidDelay.prepare(sr,swarm.state,value(28),value(29),value(30));delayLevel=value(31)*value(37);scatterStrength=value(34)*value(35);echoDrive=value(36);delaySend=value(32);reverbLevel=value(33);
        mix=value(0);mapWidth=value(7);mapDamping=value(3);mapDiffusion=.8f;mapDelay=1;mapMotion=0;
        for(int i=0;i<7;++i)smoothWeights[i]=value(14+i)*value(21+i);
    }
    void releaseResources()override{}
    bool isBusesLayoutSupported(const BusesLayout& b)const override{return b.getMainInputChannelSet()==b.getMainOutputChannelSet()&&(b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo()||b.getMainOutputChannelSet()==juce::AudioChannelSet::mono());}
    void updateControl(){
        for(int i=0;i<7;++i)smoothWeights[i]+=.15f*(value(14+i)*value(21+i)-smoothWeights[i]);
        float brightness=flock::clamp(bright/(fast+.001f)*.45f,0,1);
        scatterStrength+=.15f*(value(34)*value(35)-scatterStrength);echoDrive+=.15f*(value(36)-echoDrive);auto order=brainOrder();boidDelay.transferActivity(swarm.state);
        swarm.update(smoothWeights,std::sqrt(std::max(0.f,env)),peak,brightness,value(5),value(4),{value(9),value(10),0},{value(11),value(12),0},value(13)>.5f,value(8),&order,scatterStrength,echoDrive,value(37)>.5f);peak=0;
        auto& s=swarm.state;boidDelay.setTargets(s,value(28),value(29),value(30),true);boidDelay.describe(s);
        mapWidth+=.04f*(flock::clamp(value(7)*(.65f+s.spread),0,2)-mapWidth);
        mapDiffusion+=.04f*(.6f+.35f*s.density-mapDiffusion);
        mapDelay+=.04f*(1+.16f*s.centre.y-mapDelay);
        mapMotion+=.04f*(value(6)*flock::clamp(s.speed/.63f,0,1)-mapMotion);
        mapDamping+=.04f*(flock::clamp(value(3)+s.centre.z*.12f,0,1)-mapDamping);
        // Trajectory is derived from the moving flock, not an independent modulation clock.
        phase+=.08f*(s.centre.x*5+s.centre.y*3+s.centre.z*2-phase);
        s.width=mapWidth;s.diffusion=mapDiffusion;s.delayScale=mapDelay;s.modulation=mapMotion;
        if(++pictureCount>=3){pictureCount=0;int a,b,c,d;queue.prepareToWrite(1,a,b,c,d);if(b){pictures[(size_t)a]=s;queue.finishedWrite(1);}}
    }
    void processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&)override{
        juce::ScopedNoDenormals guard;
        for(int channel=getTotalNumInputChannels();channel<b.getNumChannels();++channel)b.clear(channel,0,b.getNumSamples());
        if(b.getNumChannels()==0)return;
        for(int s=0;s<b.getNumSamples();++s){
            float l=b.getSample(0,s),r=b.getSample(b.getNumChannels()>1?1:0,s);
            l=std::isfinite(l)?flock::clamp(l,-8,8):0;r=std::isfinite(r)?flock::clamp(r,-8,8):0;
            float magnitude=.5f*(std::abs(l)+std::abs(r)),mono=.5f*(l+r);
            env+=analysisCoeff*(.5f*(l*l+r*r)-env);fast+=analysisCoeff*(magnitude-fast);slow+=slowCoeff*(magnitude-slow);
            bright+=analysisCoeff*(std::abs(mono-previous)-bright);previous=mono;peak=std::max(peak,std::max(0.f,fast-slow));
            controlClock+=100;if(controlClock>=rate){controlClock-=rate;updateControl();}
            auto echo=boidDelay.tick(l,r);delayLevel+=mixCoeff*(value(31)*value(37)-delayLevel);delaySend+=mixCoeff*(value(32)-delaySend);reverbLevel+=mixCoeff*(value(33)-reverbLevel);
            auto wet=engine.tick(l+echo[0]*delayLevel*delaySend,r+echo[1]*delayLevel*delaySend,value(2)*mapDelay,value(1),mapDamping,mapWidth,mapMotion,phase,mapDiffusion);
            for(int ch=0;ch<2;++ch)wet[ch]=reverbLevel*wet[ch]+delayLevel*echo[ch];
            mix+=mixCoeff*(value(0)-mix);
            auto safe=[](float x){return std::isfinite(x)?x/(1+std::max(0.f,std::abs(x)-.9f)):0.f;};
            if(b.getNumChannels()==1)b.setSample(0,s,(1-mix)*l+mix*safe(.5f*(wet[0]+wet[1])));
            else{b.setSample(0,s,(1-mix)*l+mix*safe(wet[0]));b.setSample(1,s,(1-mix)*r+mix*safe(wet[1]));}
        }
    }
    bool readPicture(flock::Snapshot& out){bool got=false;while(queue.getNumReady()>0){int a,b,c,d;queue.prepareToRead(1,a,b,c,d);if(!b)break;out=pictures[(size_t)a];queue.finishedRead(1);got=true;}return got;}
    juce::AudioProcessorEditor* createEditor()override;
    bool hasEditor()const override{return true;} bool acceptsMidi()const override{return false;}bool producesMidi()const override{return false;}
    double getTailLengthSeconds()const override{return 160;}
    int getNumPrograms()override{return 1;}int getCurrentProgram()override{return 0;}void setCurrentProgram(int)override{}
    const juce::String getProgramName(int)override{return "Custom";}void changeProgramName(int,const juce::String&)override{}
    void getStateInformation(juce::MemoryBlock& b)override{auto xml=parameters.copyState().createXml();copyXmlToBinary(*xml,b);}
    void setStateInformation(const void* data,int size)override{auto xml=getXmlFromBinary(data,size);if(!xml||!xml->hasTagName(parameters.state.getType()))return;auto state=juce::ValueTree::fromXml(*xml);const char* ids[]{"delayTime","delaySpread","delayFeedback","delayLevel","delaySend","reverbLevel","scatter","scatterOn","echoDrive","echoOn","brainOrder"};const float defaults[]{300,.45f,.35f,0,.35f,1,.85f,0,.75f,1,0};for(int i=0;i<11;++i){bool found=false;for(auto child:state)if(child["id"].toString()==ids[i])found=true;if(!found){juce::ValueTree child("PARAM");child.setProperty("id",ids[i],nullptr);child.setProperty("value",defaults[i],nullptr);state.addChild(child,-1,nullptr);}}parameters.replaceState(state);}
};

#include "Editor.h"
juce::AudioProcessorEditor* FlockProcessor::createEditor(){return new FlockEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new FlockProcessor;}

