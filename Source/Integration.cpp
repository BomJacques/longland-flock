#include "Plugin.cpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include "Audition.h"

int main(int argc,char** argv){
    std::cout.setf(std::ios::unitbuf);
    juce::ScopedJuceInitialiser_GUI gui;
    if(argc==3&&juce::String(argv[1])=="--audition")return renderAuditions(juce::File(juce::String::fromUTF8(argv[2])));
    FlockProcessor p;juce::MidiBuffer midi;uint32_t rng=193;
    double audioSeconds=0;auto start=std::chrono::steady_clock::now();
    for(double sr:{44100.,48000.,96000.})for(int block:{32,128,512,1024}){
        for(int preset=0;preset<7;++preset){
            p.preset(preset);p.prepareToPlay(sr,block);juce::AudioBuffer<float> b(2,block);
            for(int base=0;base<(int)sr;base+=block){
                if(base>sr*.5){p.set("decay",20);p.set("size",3);p.set("motion",1);p.set("width",2);p.set("delayTime",1500);p.set("delaySpread",1);p.set("delayFeedback",.85f);p.set("delayLevel",1);p.set("delaySend",1);}
                for(int i=0;i<block;++i){int n=base+i;rng=rng*1664525u+1013904223u;float noise=(float)(rng>>8)/8388608.f-1;
                    float t=(float)(n/sr);float x=0;
                    switch(preset){case 0:x=0;break;case 1:x=n==0?1.f:0.f;break;case 2:x=noise;break;case 3:x=.8f*std::sin(t*6.2831853f*55);break;case 4:x=.8f*std::sin(t*6.2831853f*220);break;case 5:x=noise*std::exp(-std::fmod(t,.2f)*50);break;case 6:x=.5f*(std::sin(t*1382)+std::sin(t*1741));break;}
                    b.setSample(0,i,x);b.setSample(1,i,x*.7f);
                }
                p.processBlock(b,midi);
                for(int ch=0;ch<2;++ch)for(int i=0;i<block;++i)if(!std::isfinite(b.getSample(ch,i))||std::abs(b.getSample(ch,i))>1.01f){std::cerr<<"Output safety failure\n";return 1;}
            }
            for(const auto& bird:p.swarm.state.birds)if(!std::isfinite(bird.p.x)||bird.p.length()>1.001f)return 2;
            audioSeconds+=sr==0?0:std::ceil(sr/block)*block/sr;
        }
        std::cout<<"Stress passes: "<<sr<<" Hz / "<<block<<" samples / 7 stimuli and presets\n";
    }
    auto elapsed=std::chrono::duration<double>(std::chrono::steady_clock::now()-start).count();
    std::cout<<"Processed "<<audioSeconds<<" audio seconds in "<<elapsed<<" seconds ("<<100*elapsed/audioSeconds<<"% of one core, aggregate test including checks)\n";
    p.preset(4);p.set("goalX",-.72f);p.set("obstacleY",.61f);p.set("freeze",1);p.set("avoidOn",0);p.set("scatterOn",1);p.reorderStage(8,0);
    juce::MemoryBlock state;p.getStateInformation(state);FlockProcessor restored;restored.setStateInformation(state.getData(),(int)state.getSize());
    for(int i=0;i<39;++i)if(std::abs(p.value(i)-restored.value(i))>.0001f)return 3;
    std::cout<<"All sound parameters and target positions restore\n";
    auto legacy=p.parameters.copyState();for(int i=legacy.getNumChildren()-1;i>=0;--i){auto id=legacy.getChild(i)["id"].toString();for(auto key:delayIds)if(id==key){legacy.removeChild(i,nullptr);break;}}juce::MemoryBlock legacyBytes;auto legacyXml=legacy.createXml();juce::AudioProcessor::copyXmlToBinary(*legacyXml,legacyBytes);restored.set("delayLevel",1);restored.setStateInformation(legacyBytes.getData(),(int)legacyBytes.getSize());if(restored.value(31)!=0||restored.value(33)!=1)return 16;
    std::cout<<"Legacy state restores with echo level off\n";
    restored.prepareToPlay(48000,128);juce::AudioBuffer<float> b(2,128);b.clear();b.setSample(0,0,std::numeric_limits<float>::quiet_NaN());b.setSample(1,0,std::numeric_limits<float>::infinity());restored.processBlock(b,midi);
    for(int c=0;c<2;++c)for(int i=0;i<128;++i)if(!std::isfinite(b.getSample(c,i)))return 4;
    std::cout<<"Non-finite input is contained\n";
    // Identical output regardless of host partitioning.
    auto render=[](int block){FlockProcessor q;q.prepareToPlay(48000,block);juce::MidiBuffer m;juce::AudioBuffer<float> buffer(2,block);std::vector<float> out;out.reserve(48000);for(int n=0;n<48000;){int count=std::min(block,48000-n);buffer.setSize(2,count,false,false,true);for(int i=0;i<count;++i){float x=.3f*std::sin((n+i)*.024f);buffer.setSample(0,i,x);buffer.setSample(1,i,x);}q.processBlock(buffer,m);for(int i=0;i<count;++i)out.push_back(buffer.getSample(0,i));n+=count;}return out;};
    if(render(32)!=render(511))return 5;
    std::cout<<"Audio/control scheduling is buffer-size invariant\n";
    auto orderedAudio=[](bool echoFirst){FlockProcessor q;q.preset(2);q.set("mix",1);q.set("reverbLevel",0);q.set("delayLevel",1);q.set("echoDrive",1);q.set("scatter",1);q.set("delaySpread",1);if(echoFirst)q.reorderStage(8,7);q.prepareToPlay(48000,128);juce::AudioBuffer<float> buffer(2,128);juce::MidiBuffer m;std::vector<float> out;out.reserve(192000);for(int n=0;n<192000;n+=128){for(int i=0;i<128;++i){float t=(n+i)/48000.f;float x=.4f*std::sin(t*1000)*std::exp(-std::fmod(t,.25f)*30);buffer.setSample(0,i,x);buffer.setSample(1,i,x);}q.processBlock(buffer,m);for(int i=0;i<128;++i)out.push_back(buffer.getSample(0,i));}return out;};auto orderA=orderedAudio(false),orderB=orderedAudio(true);double difference=0,reference=0;for(size_t i=0;i<orderA.size();++i){difference+=(orderA[i]-orderB[i])*(orderA[i]-orderB[i]);reference+=orderA[i]*orderA[i];}if(reference<1e-6||difference/reference<.001)return 18;std::cout<<"Moving Echo across Scatter changes rendered audio (relative difference "<<std::sqrt(difference/reference)<<")\n";
    p.set("mix",0);p.prepareToPlay(48000,128);for(int i=0;i<128;++i){b.setSample(0,i,1);b.setSample(1,i,-1);}p.processBlock(b,midi);for(int i=0;i<128;++i)if(b.getSample(0,i)!=1||b.getSample(1,i)!=-1)return 8;
    std::cout<<"Dry-only unity gain passes\n";
    FlockProcessor echoes;echoes.set("mix",1);echoes.set("reverbLevel",0);echoes.set("delayLevel",1);echoes.set("delayTime",100);echoes.set("delaySpread",0);echoes.set("delayFeedback",0);echoes.set("freeze",1);echoes.prepareToPlay(48000,128);float premature=0,echoPeak=0;for(int base=0;base<9600;base+=128){b.clear();if(base==0){b.setSample(0,0,.1f);b.setSample(1,0,.1f);}echoes.processBlock(b,midi);for(int i=0;i<128;++i){if(base+i<4799)premature=std::max(premature,std::abs(b.getSample(0,i)));if(std::abs(base+i-4800)<2)echoPeak=std::max(echoPeak,std::abs(b.getSample(0,i)));}}if(premature>1e-6f||echoPeak<.01f)return 17;std::cout<<"Processor delay-only route produces its echo at 100 ms\n";
    FlockProcessor mono;auto monoLayout=mono.getBusesLayout();monoLayout.inputBuses.set(0,juce::AudioChannelSet::mono());monoLayout.outputBuses.set(0,juce::AudioChannelSet::mono());if(!mono.setBusesLayout(monoLayout))return 9;mono.prepareToPlay(44100,128);juce::AudioBuffer<float> monoBuffer(1,128);monoBuffer.clear();monoBuffer.setSample(0,0,1);mono.processBlock(monoBuffer,midi);for(int i=0;i<128;++i)if(!std::isfinite(monoBuffer.getSample(0,i)))return 10;
    for(int i=0;i<4;++i){std::unique_ptr<juce::AudioProcessorEditor> reopened(mono.createEditorAndMakeActive());if(!reopened)return 15;static_cast<FlockEditor*>(reopened.get())->timerCallback();mono.editorBeingDeleted(reopened.get());}
    std::cout<<"Mono layout and editor reopen pass\n";
    p.preset(1);p.prepareToPlay(48000,128);
    std::unique_ptr<juce::AudioProcessorEditor> editor(p.createEditorAndMakeActive());
    instrument::BrainRow* echoRow=nullptr;auto* surface=editor->getChildComponent(0);for(int i=0;i<surface->getNumChildComponents();++i)if(auto* row=dynamic_cast<instrument::BrainRow*>(surface->getChildComponent(i)))if(row->stage==8)echoRow=row;if(!echoRow)return 19;int savedOrder=(int)p.value(38);echoRow->stepUp(8);if((int)p.value(38)==savedOrder)return 20;instrument::BrainRow* firstRow=nullptr;for(int i=0;i<surface->getNumChildComponents();++i)if(auto* row=dynamic_cast<instrument::BrainRow*>(surface->getChildComponent(i)))if(row->stage==0)firstRow=row;if(!firstRow)return 24;echoRow->drag(8,firstRow->localPointToGlobal(juce::Point<int>(100,20)),true);if(p.brainOrder()[0]!=8)return 25;echoRow->erase(8);if(p.stageEnabled(8)||echoRow->isVisible())return 21;p.set("echoOn",1);p.set("brainOrder",(float)savedOrder);static_cast<FlockEditor*>(editor.get())->timerCallback();if(!echoRow->isVisible())return 22;editor->setSize(992,656);if(surface->getTransform().getScaleFactor()>.81f)return 23;editor->setSize(1240,820);std::cout<<"Brain UI reorder, remove/re-add and responsive scaling pass\n";
    for(int n=0;n<1000;++n){for(int i=0;i<128;++i){float x=.3f*std::sin((n*128+i)*.025f);b.setSample(0,i,x);b.setSample(1,i,x);}p.processBlock(b,midi);if(n%15==0)static_cast<FlockEditor*>(editor.get())->timerCallback();}
    // Supply a fresh snapshot, then explicitly run the editor's timer delivery.
    for(int n=0;n<20;++n){b.clear();p.processBlock(b,midi);}static_cast<FlockEditor*>(editor.get())->timerCallback();
    if(argc>1){juce::File file(juce::String::fromUTF8(argv[1]));auto stream=file.createOutputStream();if(!stream)return 6;stream->setPosition(0);stream->truncate();auto image=editor->createComponentSnapshot(editor->getLocalBounds(),true,1);juce::PNGImageFormat png;if(!png.writeImageToStream(image,*stream))return 7;std::cout<<"Editor rendered to "<<file.getFullPathName()<<"\n";
        p.set("scatterOn",1);static_cast<FlockEditor*>(editor.get())->timerCallback();auto scatterFile=file.getSiblingFile(file.getFileNameWithoutExtension()+"-scatter.png");auto scatterStream=scatterFile.createOutputStream();if(!scatterStream)return 26;scatterStream->setPosition(0);scatterStream->truncate();png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds(),true,1),*scatterStream);editor->setSize(992,656);auto compactFile=file.getSiblingFile(file.getFileNameWithoutExtension()+"-compact.png");auto compactStream=compactFile.createOutputStream();if(!compactStream)return 27;compactStream->setPosition(0);compactStream->truncate();png.writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds(),true,1),*compactStream);editor->setSize(1240,820);}
    if(argc>2){juce::VST3PluginFormat format;juce::OwnedArray<juce::PluginDescription> descriptions;format.findAllTypesForFile(descriptions,juce::String::fromUTF8(argv[2]));if(descriptions.isEmpty()){std::cerr<<"VST3 discovery failed\n";return 11;}juce::String error;auto plugin=format.createInstanceFromDescription(*descriptions[0],48000,128,error);if(!plugin){std::cerr<<error;return 12;}plugin->prepareToPlay(48000,128);for(int n=0;n<300;++n){b.clear();if(n==0)b.setSample(0,0,1);plugin->processBlock(b,midi);for(int c=0;c<2;++c)for(int i=0;i<128;++i)if(!std::isfinite(b.getSample(c,i)))return 13;}juce::MemoryBlock saved;plugin->getStateInformation(saved);plugin->setStateInformation(saved.getData(),(int)saved.getSize());{std::unique_ptr<juce::AudioProcessorEditor> hostEditor(plugin->createEditorAndMakeActive());if(!hostEditor)return 14;plugin->editorBeingDeleted(hostEditor.get());}plugin->releaseResources();std::cout<<"Actual VST3 binary: discovery, load, audio, state and editor pass\n";}
    p.editorBeingDeleted(editor.get());editor.reset();std::cout<<"Integration checks PASS\n";
}







