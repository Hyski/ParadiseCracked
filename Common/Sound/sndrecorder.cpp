#include "tokens.h"
#include "sndrecorder.h"

#include <iostream>

void SndRecorder::Run()
{
    init();
    start();
}

void SndRecorder::BeginSnd(const char* name)
{
    DSSndDesc desc;
    (*Map_of_Descs)[name]=desc;
    last_desc=&(*Map_of_Descs)[name];
}

void SndRecorder::EndSnd()
{
}

void SndRecorder::GiveReport() const
{
    std::map<std::string, DSSndDesc>::const_iterator p;

    for(p = (*Map_of_Descs).begin();p != (*Map_of_Descs).end();++p)
    {
        std::cout<<"name        :"<<p->first          <<"\n"
                 <<"loop        :"<<p->second.looped  <<"\n"
                 <<"use3d       :"<<p->second.Use3D   <<"\n"
                 <<"pan         :"<<p->second.PanValue<<"\n"
                 <<"maxdistance :"<<p->second.MaxDist <<"\n"
                 <<"mindistance :"<<p->second.MinDist <<"\n\n\n";
    }
}

void SndRecorder::RecLOOP(const char* ans)
{
    *ans=='y' ? last_desc->looped=true : last_desc->looped=false;
}

void SndRecorder::RecUSE3D(const char* ans)
{
    *ans=='y' ? last_desc->Use3D=true : last_desc->Use3D=false;
}

void SndRecorder::RecCTRLPAN(unsigned int value)
{
    last_desc->PanValue=value;
}

void SndRecorder::RecCTRLFREQ(unsigned int value)
{
    last_desc->FreqValue=value;
}

void SndRecorder::RecCTRLVOLUME(unsigned int value)
{
    last_desc->VolValue=value;
}

void SndRecorder::RecMAXDISTANCE(float value)
{
    last_desc->MaxDist=value;
}

void SndRecorder::RecMINDISTANCE(float value)
{
    last_desc->MinDist=value;
}

void SndRecorder::RecINSIDECONEANGLE(float value)
{
}

void SndRecorder::RecOUTSIDECONEANGLE(float value)
{
}

void SndRecorder::RecROLLOFF(float value)
{
}

void SndRecorder::RecDOPPLERFACTOR(float value)
{
}
