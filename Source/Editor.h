#pragma once
#include <functional>

namespace instrument {
const juce::Colour cream(0xffddd6bd), quiet(0xff929887), amber(0xffe2ae62), green(0xff9ddaa0), black(0xff101411);
const std::array<const char*,9> names{"FLOCK","SEPARATE","ALIGN","GOAL","AVOID","FOLLOW","WANDER","SCATTER","ECHO"};
const std::array<const char*,9> descriptions{"Gather reflections","Open the cloud","Smooth trajectories","Pull toward target","Dodge occupied space","Carry the trail","Loosen the path","Explode the cloud","Capture + excite echoes"};
const std::array<juce::Colour,9> tint{juce::Colour(0xff94b99a),juce::Colour(0xffc6a170),juce::Colour(0xff96bac4),juce::Colour(0xffd1bf77),juce::Colour(0xffce8e79),juce::Colour(0xffa6a2bb),juce::Colour(0xffa4b39c),juce::Colour(0xffe29164),juce::Colour(0xffe1bb70)};
inline juce::String parameter(int stage){return stage<7?ruleIds[stage]:(stage==7?"scatter":"echoDrive");}
inline juce::String enabled(int stage){return stage<7?juce::String(ruleIds[stage])+"On":(stage==7?"scatterOn":"echoOn");}
inline void type(juce::Graphics& g,const juce::String& text,juce::Rectangle<int> box,float size,juce::Colour colour,juce::Justification align=juce::Justification::centredLeft){g.setColour(colour);g.setFont(juce::Font(juce::FontOptions("Segoe UI",size,juce::Font::plain)));g.drawText(text,box,align);}
inline void digital(juce::Graphics& g,const juce::String& text,juce::Rectangle<int> box,float size,juce::Colour colour=green,juce::Justification align=juce::Justification::centredLeft){g.setColour(colour);g.setFont(juce::Font(juce::FontOptions("Consolas",size,juce::Font::plain)));g.drawText(text,box,align);}

struct Look final:juce::LookAndFeel_V4 {
    Look(){setColour(juce::Slider::textBoxTextColourId,cream);setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff101611));setColour(juce::Slider::textBoxOutlineColourId,juce::Colour(0xff434b3b));setColour(juce::ComboBox::backgroundColourId,juce::Colour(0xff101611));setColour(juce::ComboBox::textColourId,amber);setColour(juce::ComboBox::outlineColourId,juce::Colour(0xff5e604a));setColour(juce::PopupMenu::backgroundColourId,juce::Colour(0xff242923));setColour(juce::PopupMenu::textColourId,cream);setColour(juce::PopupMenu::highlightedBackgroundColourId,juce::Colour(0xff49523d));setColour(juce::Label::textColourId,cream);}
    void drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float proportion,float start,float end,juce::Slider& slider)override{
        float r=std::min((float)w,(float)h)*.5f-7,cx=x+w*.5f,cy=y+h*.5f,angle=start+proportion*(end-start);auto colour=slider.findColour(juce::Slider::rotarySliderFillColourId);
        for(int i=0;i<=10;++i){float a=start+(end-start)*i/10;g.setColour(quiet.withAlpha(.5f));g.drawLine(cx+std::sin(a)*(r+4),cy-std::cos(a)*(r+4),cx+std::sin(a)*(r+7),cy-std::cos(a)*(r+7),.8f);}
        g.setColour(juce::Colours::black.withAlpha(.65f));g.fillEllipse(cx-r-1,cy-r+3,r*2+2,r*2+2);
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff77786b),cx,cy-r,juce::Colour(0xff222920),cx,cy+r,false));g.fillEllipse(cx-r,cy-r,r*2,r*2);
        g.setColour(juce::Colour(0xff101811));g.drawEllipse(cx-r,cy-r,r*2,r*2,1);
        for(int i=0;i<36;++i){float a=i*juce::MathConstants<float>::twoPi/36;g.setColour(juce::Colours::black.withAlpha(.42f));g.drawLine(cx+std::sin(a)*(r-3),cy-std::cos(a)*(r-3),cx+std::sin(a)*(r-1),cy-std::cos(a)*(r-1),.65f);}
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff5b6057),cx-r*.4f,cy-r*.7f,juce::Colour(0xff252924),cx+r*.5f,cy+r,true));g.fillEllipse(cx-r+4,cy-r+4,(r-4)*2,(r-4)*2);
        g.setColour(juce::Colours::white.withAlpha(.09f));g.drawEllipse(cx-r+4,cy-r+4,(r-4)*2,(r-4)*2,.7f);
        juce::Path arc;arc.addCentredArc(cx,cy,r+3,r+3,0,start,angle,true);g.setColour(colour.withAlpha(.9f));g.strokePath(arc,juce::PathStrokeType(1.6f));
        g.setColour(cream);g.drawLine(cx+std::sin(angle)*r*.35f,cy-std::cos(angle)*r*.35f,cx+std::sin(angle)*(r-5),cy-std::cos(angle)*(r-5),2);
    }
    void drawButtonBackground(juce::Graphics& g,juce::Button& button,const juce::Colour&,bool hover,bool down)override{
        auto b=button.getLocalBounds().toFloat().reduced(.7f);bool lit=button.getToggleState();juce::Colour c=lit?amber:juce::Colour(0xff3b4436);if(hover)c=c.brighter(.15f);if(down)c=c.darker(.2f);g.setColour(juce::Colours::black.withAlpha(.55f));g.fillRoundedRectangle(b.translated(0,1),3);g.setGradientFill(juce::ColourGradient(c.brighter(.12f),0,b.getY(),c.darker(.2f),0,b.getBottom(),false));g.fillRoundedRectangle(b,3);g.setColour(lit?amber.brighter(.2f):juce::Colour(0xff656b58));g.drawRoundedRectangle(b,3,.8f);
    }
    void drawButtonText(juce::Graphics& g,juce::TextButton& b,bool,bool)override{type(g,b.getButtonText(),b.getLocalBounds().reduced(3),12,b.getToggleState()?black:cream,juce::Justification::centred);}
    juce::Font getComboBoxFont(juce::ComboBox&)override{return juce::Font(juce::FontOptions("Consolas",13,juce::Font::plain));}
    juce::Label* createSliderTextBox(juce::Slider& slider)override{auto* box=juce::LookAndFeel_V4::createSliderTextBox(slider);box->setFont(juce::Font(juce::FontOptions("Consolas",11,juce::Font::plain)));return box;}
};

struct Dial:juce::Component {
    juce::Slider slider;juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    Dial(FlockProcessor& p,const juce::String& id,const juce::String& name,juce::Colour colour){
        addAndMakeVisible(slider);addAndMakeVisible(label);label.setText(name,juce::dontSendNotification);label.setFont(juce::Font(juce::FontOptions("Segoe UI",12,juce::Font::plain)));label.setJustificationType(juce::Justification::centred);label.setColour(juce::Label::textColourId,cream);
        slider.setName(name);slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);slider.setRotaryParameters(juce::MathConstants<float>::pi*1.2f,juce::MathConstants<float>::pi*2.8f,true);slider.setTextBoxStyle(juce::Slider::TextBoxBelow,false,66,19);slider.setColour(juce::Slider::rotarySliderFillColourId,colour);slider.setMouseDragSensitivity(180);
        attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,id,slider);
        if(auto* parameter=p.parameters.getParameter(id))slider.setDoubleClickReturnValue(true,parameter->convertFrom0to1(parameter->getDefaultValue()));
        slider.textFromValueFunction=[id](double v){if(id=="delayTime")return juce::String(juce::roundToInt(v))+" ms";if(id=="decay")return juce::String(v,1)+" s";if(id=="size")return juce::String(v,2)+"x";return juce::String(juce::roundToInt(v*100))+"%";};
        slider.valueFromTextFunction=[id](const juce::String& s){double v=s.getDoubleValue();return id=="delayTime"||id=="decay"||id=="size"?v:v/100;};
        slider.updateText();slider.setTooltip(name+" / drag to turn, double-click to reset; click the value to type.");
    }
    void resized()override{slider.setBounds(0,0,getWidth(),96);label.setBounds(0,99,getWidth(),19);}
};

struct BrainRow:juce::Component {
    FlockProcessor& p;int stage;bool selected=false,dragging=false;float activity=0;
    juce::Slider strength;juce::TextButton up{"^"},down{"v"},view{"VIEW"},remove{"x"};
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    std::function<void(int)> select,stepUp,stepDown,erase;
    std::function<void(int,juce::Point<int>,bool)> drag;
    int number=1;
    BrainRow(FlockProcessor& processor,int id):p(processor),stage(id){
        addAndMakeVisible(strength);strength.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);strength.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);strength.setColour(juce::Slider::rotarySliderFillColourId,tint[id]);strength.setMouseDragSensitivity(160);strength.setName(juce::String(names[id])+" strength");attachment=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,parameter(id),strength);if(auto* param=p.parameters.getParameter(parameter(id)))strength.setDoubleClickReturnValue(true,param->convertFrom0to1(param->getDefaultValue()));
        for(auto* b:{&up,&down,&view,&remove})addAndMakeVisible(*b);
        up.setTooltip("Move this module earlier in the Brain");down.setTooltip("Move this module later in the Brain");remove.setTooltip("Remove module from active Brain; add it again with + MODULE");view.setTooltip("Inspect this rule on the radar");
        up.onClick=[this]{if(stepUp)stepUp(stage);};down.onClick=[this]{if(stepDown)stepDown(stage);};remove.onClick=[this]{if(erase)erase(stage);};view.onClick=[this]{if(select)select(stage);};
        setTooltipText();
    }
    void setTooltipText(){strength.setTooltip(stage==8?"Echo brain drive: predicts tap positions at this chain stage and returns audible echo activity to boid energy.":"Rule strength. Later active rules reshape earlier steering; drag the grip to change order.");}
    void resized()override{strength.setBounds(284,0,47,43);up.setBounds(386,9,25,25);down.setBounds(416,9,25,25);view.setBounds(450,9,51,25);remove.setBounds(getWidth()-37,9,25,25);}
    void paint(juce::Graphics& g)override{
        auto box=getLocalBounds().toFloat().reduced(.5f);auto c=selected?juce::Colour(0xff3a3e31):juce::Colour(0xff292d28);g.setGradientFill(juce::ColourGradient(c.brighter(.06f),0,0,c.darker(.1f),0,(float)getHeight(),false));g.fillRoundedRectangle(box,4);g.setColour(selected?tint[stage].withAlpha(.75f):juce::Colour(0xff48513f));g.drawRoundedRectangle(box,4,.8f);g.setColour(tint[stage]);g.fillRoundedRectangle(1,6,3,(float)getHeight()-12,1);
        for(int col=0;col<2;++col)for(int r=0;r<3;++r){g.setColour(quiet.withAlpha(.65f));g.fillEllipse(12.f+col*4,14.f+r*6,1.5f,1.5f);}
        digital(g,juce::String(number).paddedLeft('0',2),{29,8,31,26},14,tint[stage]);type(g,names[stage],{65,2,169,22},15,cream);type(g,descriptions[stage],{65,23,196,16},11,quiet);
        for(int i=0;i<7;++i){g.setColour(i<activity*7?tint[stage]:juce::Colour(0xff151d16));g.fillRect(254,31-i*3,15,2);}
        digital(g,juce::String(juce::roundToInt((float)strength.getValue()*100))+"%",{332,8,48,27},12,tint[stage]);
    }
    void mouseDown(const juce::MouseEvent&)override{dragging=false;if(select)select(stage);}
    void mouseDrag(const juce::MouseEvent& e)override{if(e.getDistanceFromDragStart()>4){dragging=true;if(drag)drag(stage,e.getScreenPosition(),false);}}
    void mouseUp(const juce::MouseEvent& e)override{if(dragging&&drag)drag(stage,e.getScreenPosition(),true);dragging=false;}
};
}

class FlockEditor final:public juce::AudioProcessorEditor,private juce::Timer {
    using BA=juce::AudioProcessorValueTreeState::ButtonAttachment;
    struct Canvas:juce::Component {FlockEditor& owner;explicit Canvas(FlockEditor& o):owner(o){}void paint(juce::Graphics& g)override{owner.paintSurface(g);}void mouseDown(const juce::MouseEvent& e)override{owner.radarDown(e);}void mouseDrag(const juce::MouseEvent& e)override{owner.radarDrag(e);}void mouseUp(const juce::MouseEvent&)override{owner.endTargetDrag();}};
    FlockProcessor& p;instrument::Look look;Canvas canvas{*this};
    juce::Image chassis;std::array<std::unique_ptr<instrument::Dial>,15> dials;
    std::array<std::unique_ptr<instrument::BrainRow>,9> rows;
    juce::ComboBox presets;juce::TextButton previous{"<"},next{">"},freeze{"FREEZE"},add{"+ MODULE"};
    juce::TextButton trails{"TRAIL"},vectors{"VECTOR"},rules{"RULE"},targets{"TARGET"},tapView{"TAPS"};
    std::unique_ptr<BA> freezeAttachment;juce::TooltipWindow tooltips{this,450};
    flock::Snapshot picture;std::array<std::array<flock::Vec,64>,30> history{};
    std::array<int,9> visible{};int visibleCount=0,selectedBoid=0,selectedStage=8,historyHead=0,historySize=0,dragTarget=0,dropStage=-1,lastOrder=-1,lastMask=-1;
    float sweep=0;const juce::Point<float> radarCentre{267,304};constexpr static float radarRadius=171;
    juce::Point<float> project(flock::Vec v)const{return{radarCentre.x+v.x*radarRadius*.95f,radarCentre.y-v.y*radarRadius*.95f};}
    void smallArrow(juce::Graphics& g,flock::Vec at,flock::Vec force,juce::Colour colour,float scale){auto a=project(at),b=project(at+force*scale);if(a.getDistanceFrom(b)>1){g.setColour(colour);g.drawArrow({a,b},1,4,5);}}
    void plate(juce::Graphics& g,juce::Rectangle<float> box,juce::Colour colour){g.setColour(juce::Colours::black.withAlpha(.8f));g.fillRoundedRectangle(box.translated(0,2),6);g.setGradientFill(juce::ColourGradient(colour.brighter(.07f),box.getX(),box.getY(),colour.darker(.15f),box.getX(),box.getBottom(),false));g.fillRoundedRectangle(box,5);g.setColour(juce::Colour(0xff626855).withAlpha(.45f));g.drawRoundedRectangle(box.reduced(.5f),5,1);}
    void makeChassis(){
        using namespace instrument;chassis=juce::Image(juce::Image::RGB,2480,1640,true);juce::Graphics g(chassis);g.addTransform(juce::AffineTransform::scale(2.f));
        g.fillAll(juce::Colour(0xff171a15));g.setGradientFill(juce::ColourGradient(juce::Colour(0xff4d3929),0,0,juce::Colour(0xff231b15),1240,0,false));g.fillRect(0,0,18,820);g.fillRect(1222,0,18,820);
        for(int x=2;x<17;x+=3){g.setColour(juce::Colour(0xffa17e50).withAlpha(.12f));g.drawLine((float)x,0,(float)x+2,820,.8f);g.drawLine(1224.f+x,0,1224.f+x-2,820,.8f);}
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff343432),18,0,juce::Colour(0xff1b1e1c),1222,820,false));g.fillRect(18,0,1204,820);
        for(int y=0;y<820;y+=3){g.setColour(juce::Colours::black.withAlpha(.035f));g.drawHorizontalLine(y,18,1222);}
        plate(g,{30,17,1178,70},juce::Colour(0xff2c302c));
        type(g,"L O N G L A N D",{49,24,210,18},13,cream);type(g,"FLOCK",{47,40,226,40},40,cream);type(g,"ECOLOGICAL DELAY / REVERBERATOR",{248,44,365,25},13,quiet);
        for(int x:{40,1198})for(int y:{27,77,804}){g.setColour(juce::Colour(0xff131b14));g.fillEllipse((float)x-3,y-3.f,6,6);g.setColour(juce::Colour(0xff78806a));g.drawLine(x-1.5f,y-1.f,x+1.5f,y+1.f,.7f);}
        plate(g,{30,101,548,421},juce::Colour(0xff141d15));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff152e1d),radarCentre.x,radarCentre.y,juce::Colour(0xff060f0a),radarCentre.x+258,radarCentre.y,true));g.fillRoundedRectangle(39,110,530,403,5);
        for(int y=112;y<511;y+=3){g.setColour(juce::Colours::black.withAlpha(.11f));g.drawHorizontalLine(y,41,567);}
        digital(g,"FLOCK RADAR",{54,120,215,20},15,green);digital(g,"XY / ECHO FIELD",{53,142,200,15},10,green.withAlpha(.5f));
        g.setColour(green.withAlpha(.17f));for(int i=1;i<=4;++i){float r=radarRadius*i/4;g.drawEllipse(radarCentre.x-r,radarCentre.y-r,r*2,r*2,.7f);}
        for(int i=0;i<72;++i){float a=i*juce::MathConstants<float>::twoPi/72;float r1=radarRadius+(i%6==0?4:2),r2=radarRadius+7;g.setColour(green.withAlpha(i%6==0?.45f:.2f));g.drawLine(radarCentre.x+std::sin(a)*r1,radarCentre.y-std::cos(a)*r1,radarCentre.x+std::sin(a)*r2,radarCentre.y-std::cos(a)*r2,.65f);}
        g.setColour(green.withAlpha(.17f));g.drawLine(radarCentre.x-radarRadius,radarCentre.y,radarCentre.x+radarRadius,radarCentre.y,.7f);g.drawLine(radarCentre.x,radarCentre.y-radarRadius,radarCentre.x,radarCentre.y+radarRadius,.7f);
        digital(g,"0",{259,116,18,17},10,green.withAlpha(.6f),juce::Justification::centred);digital(g,"180",{250,474,34,12},9,green.withAlpha(.6f),juce::Justification::centred);
        plate(g,{30,536,548,57},juce::Colour(0xff151d15));
        plate(g,{595,101,613,492},juce::Colour(0xff1d2420));type(g,"BOID BRAIN",{613,111,208,25},19,cream);type(g,"SERIAL / DRAG TO REORDER",{774,116,225,17},11,quiet);
        struct Bank{float x,w;const char* title;juce::Colour c;};
        for(auto b:{Bank{30,284,"01 / SWARM",tint[0]},Bank{326,371,"02 / ECHO",tint[8]},Bank{709,499,"03 / SPACE",tint[2]}}){plate(g,{b.x,608,b.w,169},juce::Colour(0xff292f2a));g.setColour(b.c);g.fillRect(b.x+16,623.f,20.f,3.f);type(g,b.title,{(int)b.x+45,615,(int)b.w-58,20},13,cream);g.setColour(quiet.withAlpha(.2f));g.drawHorizontalLine(641,b.x+16,b.x+b.w-16);}
        type(g,"64 AGENTS  /  64 ECHO TAPS  /  8-LINE SPACE",{42,787,470,19},10,quiet);type(g,"DOUBLE-CLICK RESET   /   CLICK VALUE TO TYPE",{817,787,380,19},10,quiet,juce::Justification::centredRight);
    }
    void layoutRows(){
        auto order=p.brainOrder();visibleCount=0;for(int stage:order){bool enabled=p.stageEnabled(stage);rows[stage]->setVisible(enabled);if(enabled){visible[visibleCount]=stage;auto& r=*rows[stage];r.number=visibleCount+1;r.setBounds(613,149+visibleCount*47,578,43);r.selected=stage==selectedStage;r.view.setToggleState(r.selected,juce::dontSendNotification);++visibleCount;}}
        for(int i=0;i<visibleCount;++i){rows[visible[i]]->up.setEnabled(i>0);rows[visible[i]]->down.setEnabled(i+1<visibleCount);}
        for(int i=4;i<9;++i){dials[i]->setEnabled(p.stageEnabled(8));dials[i]->setAlpha(p.stageEnabled(8)?1.f:.35f);}add.setEnabled(visibleCount<9);lastOrder=(int)p.value(38);lastMask=0;for(int i=0;i<9;++i)if(p.stageEnabled(i))lastMask|=1<<i;canvas.repaint();
    }
    void choose(int stage){selectedStage=stage;rules.setToggleState(true,juce::dontSendNotification);for(auto& row:rows){row->selected=row->stage==stage;row->view.setToggleState(row->selected,juce::dontSendNotification);row->repaint();}canvas.repaint(39,110,530,403);}
    void step(int stage,int delta){for(int i=0;i<visibleCount;++i)if(visible[i]==stage){int target=juce::jlimit(0,visibleCount-1,i+delta);p.reorderStage(stage,visible[target]);layoutRows();break;}}
    void addModule(){juce::PopupMenu menu;for(int i=0;i<9;++i)if(!p.stageEnabled(i))menu.addItem(i+1,instrument::names[i]);juce::Component::SafePointer<FlockEditor> safe(this);menu.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&add),[safe](int result){if(safe&&result>0){safe->p.set(instrument::enabled(result-1),1);safe->choose(result-1);safe->layoutRows();}});}
    void dragRow(int stage,juce::Point<int> screen,bool release){auto point=canvas.getLocalPoint(nullptr,screen);dropStage=-1;for(int i=0;i<visibleCount;++i)if(rows[visible[i]]->getBounds().contains(point)){dropStage=visible[i];break;}if(release){if(dropStage>=0&&dropStage!=stage)p.reorderStage(stage,dropStage);dropStage=-1;layoutRows();}canvas.repaint(595,101,613,492);}
public:
    explicit FlockEditor(FlockProcessor& processor):AudioProcessorEditor(processor),p(processor){
        setLookAndFeel(&look);addAndMakeVisible(canvas);picture=flock::Swarm{}.state;makeChassis();
        for(auto* button:{&previous,&next,&freeze,&add,&trails,&vectors,&rules,&targets,&tapView})canvas.addAndMakeVisible(*button);
        for(auto* button:{&trails,&vectors,&rules,&targets,&tapView}){button->setClickingTogglesState(true);button->setToggleState(button!=&vectors,juce::dontSendNotification);button->onClick=[this]{canvas.repaint();};}
        freeze.setClickingTogglesState(true);freezeAttachment=std::make_unique<BA>(p.parameters,"freeze",freeze);freeze.setTooltip("Hold the ecological state. Echo repeats and reverb continue.");
        canvas.addAndMakeVisible(presets);for(int i=0;i<7;++i)presets.addItem(presetNames[i],i+1);presets.setTextWhenNothingSelected("CUSTOM ECOSYSTEM");presets.onChange=[this]{if(presets.getSelectedId()>0){p.preset(presets.getSelectedId()-1);layoutRows();}};
        previous.onClick=[this]{int id=presets.getSelectedId();presets.setSelectedId(id<=1?7:id-1);};next.onClick=[this]{int id=presets.getSelectedId();presets.setSelectedId(id>=7?1:id+1);};add.onClick=[this]{addModule();};
        const char* ids[]{"energy","transient","motion","radius","delayTime","delaySpread","delayFeedback","delayLevel","delaySend","size","decay","damping","width","reverbLevel","mix"};
        const char* labels[]{"ENERGY","REACTION","MOTION","RADIUS","TIME","SPREAD","FEEDBACK","LEVEL","SEND","SIZE","DECAY","DAMPING","WIDTH","REVERB","MIX"};
        for(int i=0;i<15;++i){dials[i]=std::make_unique<instrument::Dial>(p,ids[i],labels[i],instrument::tint[i<4?0:(i<9?8:2)]);canvas.addAndMakeVisible(*dials[i]);}
        for(int i=0;i<9;++i){rows[i]=std::make_unique<instrument::BrainRow>(p,i);auto& row=*rows[i];canvas.addAndMakeVisible(row);row.select=[this](int s){choose(s);};row.stepUp=[this](int s){step(s,-1);};row.stepDown=[this](int s){step(s,1);};row.erase=[this](int s){p.set(instrument::enabled(s),0);layoutRows();};row.drag=[this](int s,juce::Point<int> point,bool release){dragRow(s,point,release);};}
        previous.setBounds(633,37,28,30);presets.setBounds(669,35,297,34);next.setBounds(974,37,28,30);freeze.setBounds(1034,37,141,30);add.setBounds(1041,113,149,27);
        int x=53;for(auto* b:{&trails,&vectors,&rules,&targets,&tapView}){b->setBounds(x,488,87,23);x+=96;}
        for(int i=0;i<15;++i){int local=i<4?i:(i<9?i-4:i-9),count=i<4?4:(i<9?5:6);float bx=i<4?30.f:(i<9?326.f:709.f),bw=i<4?284.f:(i<9?371.f:499.f);int cell=(int)((bw-26)/count);dials[i]->setBounds((int)bx+13+local*cell,650,cell,119);}
        layoutRows();setResizable(true,true);getConstrainer()->setFixedAspectRatio(1240.0/820);setResizeLimits(992,656,1860,1230);setSize(1240,820);startTimerHz(30);
    }
    ~FlockEditor()override{stopTimer();endTargetDrag();setLookAndFeel(nullptr);}
    void resized()override{float scale=std::min(getWidth()/1240.f,getHeight()/820.f);canvas.setBounds(0,0,1240,820);canvas.setTransform(juce::AffineTransform::scale(scale));}
    void paint(juce::Graphics& g)override{g.fillAll(instrument::black);}
    void paintSurface(juce::Graphics& g){
        using namespace instrument;g.drawImage(chassis,juce::Rectangle<float>(0,0,1240,820));int echoSlot=0;for(int i=0;i<visibleCount;++i)if(visible[i]==8)echoSlot=i+1;digital(g,echoSlot>0?"BRAIN / "+juce::String(echoSlot):"REMOVED",{565,617,113,18},11,echoSlot>0?amber:quiet,juce::Justification::centredRight);
        {juce::Graphics::ScopedSaveState save(g);juce::Path clip;clip.addEllipse(radarCentre.x-radarRadius,radarCentre.y-radarRadius,radarRadius*2,radarRadius*2);g.reduceClipRegion(clip);
            juce::Path beam;beam.startNewSubPath(radarCentre);beam.lineTo(radarCentre.x+std::sin(sweep)*radarRadius,radarCentre.y-std::cos(sweep)*radarRadius);beam.addCentredArc(radarCentre.x,radarCentre.y,radarRadius,radarRadius,0,sweep,sweep+.36f,false);beam.closeSubPath();g.setGradientFill(juce::ColourGradient(green.withAlpha(.005f),radarCentre.x,radarCentre.y,green.withAlpha(.11f),radarCentre.x+radarRadius,radarCentre.y,true));g.fillPath(beam);
            g.setColour(green.withAlpha(.18f));g.drawLine(radarCentre.x,radarCentre.y,radarCentre.x+std::sin(sweep+.36f)*radarRadius,radarCentre.y-std::cos(sweep+.36f)*radarRadius,.8f);
            if(targets.getToggleState()){auto goal=project({p.value(9),p.value(10),0}),obstacle=project({p.value(11),p.value(12),0});g.setColour(amber.withAlpha(.9f));g.drawLine(goal.x-8,goal.y,goal.x+8,goal.y,1);g.drawLine(goal.x,goal.y-8,goal.x,goal.y+8,1);g.drawEllipse(goal.x-5,goal.y-5,10,10,1);digital(g,"G",{(int)goal.x+10,(int)goal.y-8,15,16},11,amber);g.setColour(tint[4].withAlpha(.07f));g.fillEllipse(obstacle.x-42,obstacle.y-42,84,84);g.setColour(tint[4].withAlpha(.65f));g.drawEllipse(obstacle.x-7,obstacle.y-7,14,14,1);digital(g,"A",{(int)obstacle.x+10,(int)obstacle.y-8,15,16},11,tint[4]);}
            if(picture.pulse>.02f){g.setColour(tint[7].withAlpha(picture.pulse*.35f));float r=20+picture.pulse*120;g.drawEllipse(radarCentre.x-r,radarCentre.y-r,r*2,r*2,1);}
            for(int i=0;i<64;++i){const auto& bird=picture.birds[i];auto at=project(bird.p);if(trails.getToggleState())for(int j=1;j<historySize;++j){g.setColour((i==selectedBoid?amber:green).withAlpha(.28f*(1-(float)j/historySize)));g.drawLine({project(history[(historyHead+30-j)%30][i]),project(history[(historyHead+29-j)%30][i])},i==selectedBoid?1.3f:.75f);}
                if(vectors.getToggleState())smallArrow(g,bird.p,bird.v,green.withAlpha(.4f),.35f);
                float glow=.15f+.4f*picture.echoActivity[i];g.setColour(green.withAlpha(glow*.2f));g.fillEllipse(at.x-6,at.y-6,12,12);g.setColour(i==selectedBoid?amber:green.withAlpha(.65f+.3f*picture.echoActivity[i]));float r=i==0?2.8f:1.7f;g.fillEllipse(at.x-r,at.y-r,r*2,r*2);
            }
            const auto& b=picture.birds[selectedBoid];auto selected=project(b.p);g.setColour(amber);g.drawRect(selected.x-6,selected.y-6,12.f,12.f,1.f);
            if(rules.getToggleState()&&p.stageEnabled(selectedStage)){smallArrow(g,b.p,b.forces[selectedStage],tint[selectedStage],.35f);if(selectedStage==1){float r=.18f*radarRadius;g.setColour(tint[1].withAlpha(.35f));g.drawEllipse(selected.x-r,selected.y-r,r*2,r*2,1);}if(selectedStage==0||selectedStage==2){for(int j=0;j<64;++j)if(j!=selectedBoid&&(b.p-picture.birds[j].p).length()<p.value(8)){g.setColour(tint[selectedStage].withAlpha(.15f));g.drawLine({selected,project(picture.birds[j].p)},.7f);}}}
            if(tapView.getToggleState()){auto echo=project(picture.echoPositions[selectedBoid]);g.setColour(amber.withAlpha(.7f));g.drawDashedLine({selected,echo},std::array<float,2>{3,3}.data(),2,.8f);g.drawEllipse(echo.x-4,echo.y-4,8,8,1);}
        }
        digital(g,"LIVE",{494,121,61,18},11,green);g.setColour(green);g.fillEllipse(483,127,4,4);
        const char* stats[]{"AGENTS","ENERGY","SPREAD"};const juce::String values[]{"64",juce::String(juce::roundToInt(picture.energy*100))+"%",juce::String(picture.spread,2)};
        for(int i=0;i<3;++i){int y=176+i*68;g.setColour(green.withAlpha(.09f));g.drawRect(469,y,85,54,1);digital(g,stats[i],{478,y+7,70,14},9,green.withAlpha(.5f));digital(g,values[i],{478,y+23,72,24},20,green);}
        digital(g,"TRACK "+juce::String(selectedBoid+1).paddedLeft('0',2),{471,397,89,17},10,amber);digital(g,names[selectedStage],{471,417,88,16},10,tint[selectedStage]);
        digital(g,"ECHO "+juce::String(selectedBoid+1).paddedLeft('0',2)+"  /  "+juce::String(juce::roundToInt(picture.tapMs[selectedBoid]))+" ms",{44,544,249,18},12,amber);digital(g,"PAN "+juce::String(picture.tapPan[selectedBoid],2)+"   LEVEL "+juce::String(picture.tapLevel[selectedBoid],2),{293,544,271,18},11,green.withAlpha(.8f),juce::Justification::centredRight);
        g.setColour(green.withAlpha(.2f));g.drawHorizontalLine(579,47,560);for(int i=0;i<64;++i){float x=47+513*flock::clamp(picture.tapMs[i]/3000,0,1);g.setColour(i==selectedBoid?amber:green.withAlpha(.4f));g.drawLine(x,570,x,585,i==selectedBoid?1.8f:.8f);}
        if(visibleCount<9)type(g,"+ Add a rule. Move ECHO to change what it hears.",{616,149+visibleCount*47,564,32},11,quiet);
        if(dropStage>=0){auto box=rows[dropStage]->getBounds();g.setColour(amber);g.fillRect(box.getX(),box.getY()-2,box.getWidth(),2);}
    }
    void timerCallback()override{
        if(p.readPicture(picture)){for(int i=0;i<64;++i)history[historyHead][i]=picture.birds[i].p;historyHead=(historyHead+1)%30;historySize=std::min(30,historySize+1);}
        sweep=std::fmod(sweep+.027f,juce::MathConstants<float>::twoPi);int mask=0;for(int i=0;i<9;++i){if(p.stageEnabled(i))mask|=1<<i;rows[i]->activity=flock::clamp(picture.activity[i]/(i==8?1.f:1.5f),0,1);}
        if(lastOrder!=(int)p.value(38)||lastMask!=mask)layoutRows();canvas.repaint(30,101,548,492);canvas.repaint(595,101,613,492);
    }
    void radarDown(const juce::MouseEvent& e){auto pos=e.position;if(pos.y>=568&&pos.y<=590&&pos.x>=47&&pos.x<=560){float best=1e9f;for(int i=0;i<64;++i){float d=std::abs(pos.x-(47+513*picture.tapMs[i]/3000));if(d<best){best=d;selectedBoid=i;}}canvas.repaint();return;}if(pos.getDistanceFrom(radarCentre)>radarRadius)return;
        if(targets.getToggleState()&&pos.getDistanceFrom(project({p.value(9),p.value(10),0}))<14)dragTarget=1;else if(targets.getToggleState()&&pos.getDistanceFrom(project({p.value(11),p.value(12),0}))<14)dragTarget=2;else{float nearest=1e9f;for(int i=0;i<64;++i){float d=pos.getDistanceFrom(project(picture.birds[i].p));if(d<nearest){nearest=d;selectedBoid=i;}}}
        if(dragTarget){p.parameters.getParameter(dragTarget==1?"goalX":"obstacleX")->beginChangeGesture();p.parameters.getParameter(dragTarget==1?"goalY":"obstacleY")->beginChangeGesture();}canvas.repaint();
    }
    void radarDrag(const juce::MouseEvent& e){if(!dragTarget)return;float x=flock::clamp((e.position.x-radarCentre.x)/(radarRadius*.95f),-.85f,.85f),y=flock::clamp((radarCentre.y-e.position.y)/(radarRadius*.95f),-.85f,.85f);auto* a=p.parameters.getParameter(dragTarget==1?"goalX":"obstacleX");auto* b=p.parameters.getParameter(dragTarget==1?"goalY":"obstacleY");a->setValueNotifyingHost(a->convertTo0to1(x));b->setValueNotifyingHost(b->convertTo0to1(y));canvas.repaint(30,101,548,421);}
    void endTargetDrag(){if(dragTarget){p.parameters.getParameter(dragTarget==1?"goalX":"obstacleX")->endChangeGesture();p.parameters.getParameter(dragTarget==1?"goalY":"obstacleY")->endChangeGesture();dragTarget=0;}}
};



