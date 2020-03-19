/**************************************************************                 
                                                                                
                             Paradise Cracked                                         
                                                                                
                       (c) by MiST Land 2000                                    
        ---------------------------------------------------                     
   Description: Sound recorder
                
   Author: Klimov Alexander a.k.a. DisPell
***************************************************************/

#ifndef __SNDRECORDER_H__
#define __SNDRECORDER_H__

#include "SndParser.h"
#include "sound.h"

class SndRecorder : protected SndParser{
public:

    SndRecorder(ANTLRTokenBuffer* p_buff) : SndParser(p_buff), 
											last_srcdesc(NULL) {};
    ~SndRecorder() {};

    void Run();

private:
    sound::srcdsc* last_srcdesc;

	void BeginSnd(const char* name);
	void EndSnd  ();

    void GiveReport  ()                    const;
    void ThrowUnknown(const char* unknown) const;

	// record parametres
	void RecLOOP			    ();
	void RecUSE3D		        ();
	void RecSTREAMED	        ();
	void RecMP3 				();
	void RecPERMANENT 			();
	void RecLOWSRCQUALITY       ();
	void RecMEDSRCQUALITY       ();
	void RecHIGHSRCQUALITY      ();
	void RecLOWPRIORITY         ();
	void RecMEDPRIORITY         ();
	void RecHIGHPRIORITY        ();
	void RecCTRLPAN			    (int value);
	void RecCTRLFREQ	        (unsigned int value);
	void RecCTRLVOLUME		    (int value);
	void RecMAXDISTANCE	        (float value);
	void RecMINDISTANCE		    (float value);
	void RecINSIDECONEANGLE     (unsigned int value);
	void RecOUTSIDECONEANGLE    (unsigned int value);
    void RecCONEOUTSIDEVOLUME   (int value);
    void RecROLLOFF             (float value);
	void RecDOPPLERFACTOR       (float value);
    void RecMODE                (unsigned int value);
    void RecNAME        		(const char* name); 
	void RecLCOORDSYSTEM        ();
	void RecRCOORDSYSTEM        ();
	void RecPRIORITY			(float value);
    void RecMEMORYSTATIC        ();

	void RecSOURCEDIRECT        (int value);
	void RecSOURCEDIRECTHF		(int value);
	void RecSOURCEROOM			(int value);
	void RecSOURCEROOMHF		(int value);
	void RecSOURCEROLLOFF		(float value);
	void RecSOURCEOUTSIDE		(int value);
	void RecSOURCEABSORPTION	(float value);
	void RecSOURCEFLAGS			(unsigned int value);
	void RecSOURCEOBSTRUCTION	(int value);
	void RecSOURCEOBSTRUCTIONLF	(float value);
	void RecSOURCEOCCLUSION		(int value);
	void RecSOURCEOCCLUSIONLF	(float value);
	void RecSOURCEOCCLUSIONROOM	(float value);
	void RecPOSX				(float value);
	void RecPOSY				(float value);
 	void RecPOSZ				(float value);
	void RecSOURCEAFFECTDIRECTHF();
	void RecSOURCEAFFECTROOM	();
	void RecSOURCEAFFECTROOMHF	();

	void RecROOM				(int value);
	void RecROOMHF				(int value);
	void RecROOMROLLOFF			(float value);
	void RecDECAYTIME			(float value);
	void RecDECAYHFRATIO		(float value);
	void RecREFLECTIONS			(int value);
	void RecREFLECTIONSDELAY	(float value);
	void RecREVERB				(int value);
	void RecREVERBDELAY			(float value);
	void RecENVIROMENT			(unsigned int value);
	void RecENVIROMENTSIZE		(float value);
	void RecENVIROMENTDIFFUSION	(float value);
	void RecAIRABSOPTION		(float value);
	void RecFLAGS				(unsigned int value);
	void RecSCALETIME			();
	void RecSCALEREFLECTIONS	();
	void RecSCALEREFLECTIONSDELAY();
	void RecSCALEREVERB			();
	void RecSCALEREVERBDELAY	();
	void RecCLIPDECAYHF			();
};

//
//
//
inline void SndRecorder::RecNAME(const char* name)
{
//  recording the name of the description:
//	filename must not contain bracets("-symbol),
//  so cut the first symbol if it is bracet and the last if it is too
	name++;

	last_srcdesc->_name = name;

	int last_elem=last_srcdesc->_name.length()-1;

	if(last_elem >= 0)
		last_srcdesc->_name[last_elem]=0;
}

//
//
//
inline void SndRecorder::RecLOOP()
{
    last_srcdesc->_Flags|=sound::LOOPED;
}

//
//
//
inline void SndRecorder::RecUSE3D()
{
    last_srcdesc->_Flags|=sound::DISABLE_3D;
}

//
//
//
inline void SndRecorder::RecSTREAMED()
{
    last_srcdesc->_Flags|=sound::STREAMED;
}

//
//
//
inline void SndRecorder::RecMP3()
{
    last_srcdesc->_Flags|=sound::MPEG3FILE;
}

//
//
//
inline void SndRecorder::RecPERMANENT()
{
    last_srcdesc->_Flags|=sound::PERMANENT;
}

//
//
//
inline void SndRecorder::RecLOWPRIORITY()
{
    last_srcdesc->_Flags|=sound::LOWPRIORITY;
}

//
//
//
inline void SndRecorder::RecMEDPRIORITY()
{
    last_srcdesc->_Flags|=sound::MEDPRIORITY;
}

//
//
//
inline void SndRecorder::RecHIGHPRIORITY()
{
    last_srcdesc->_Flags|=sound::HIGHPRIORITY;
}

//
//
//
inline void SndRecorder::RecLOWSRCQUALITY()
{
    last_srcdesc->_quality.changed=true;
	last_srcdesc->_quality.value=sound::LOWQUALITY;
}

//
//
//
inline void SndRecorder::RecMEDSRCQUALITY()
{
    last_srcdesc->_quality.changed=true;
	last_srcdesc->_quality.value=sound::MEDQUALITY;
}

//
//
//
inline void SndRecorder::RecHIGHSRCQUALITY()
{
    last_srcdesc->_quality.changed=true;
	last_srcdesc->_quality.value=sound::HIGHQUALITY;
}

//
//
//
inline void SndRecorder::RecCTRLPAN(int value)
{
    last_srcdesc->_pan_value.changed=true;
    last_srcdesc->_pan_value.value=value;
}

//
//
//
inline void SndRecorder::RecCTRLFREQ(unsigned int value)
{
    last_srcdesc->_freq_value.changed=true;
    last_srcdesc->_freq_value.value=value;
}

//
//
//
inline void SndRecorder::RecCTRLVOLUME(int value)
{
    last_srcdesc->_vol_value.changed=true;
    last_srcdesc->_vol_value.value=value;
}

//
//
//
inline void SndRecorder::RecMAXDISTANCE(float value)
{
    last_srcdesc->_max_dist.changed=true;
    last_srcdesc->_max_dist.value=value;
}

//
//
//
inline void SndRecorder::RecMINDISTANCE(float value)
{
    last_srcdesc->_min_dist.changed=true;
    last_srcdesc->_min_dist.value=value;
}

//
//
//
inline void SndRecorder::RecINSIDECONEANGLE(unsigned int value)
{
    last_srcdesc->_inside_cone_angle.changed=true;
    last_srcdesc->_inside_cone_angle.value=value;
}

//
//
//
inline void SndRecorder::RecOUTSIDECONEANGLE(unsigned int value)
{
    last_srcdesc->_outside_cone_angle.changed=true;
    last_srcdesc->_outside_cone_angle.value=value;
}

//
//
//
inline void SndRecorder::RecCONEOUTSIDEVOLUME(int value)
{
    last_srcdesc->_cone_outside_volume.changed=true;
    last_srcdesc->_cone_outside_volume.value=value;
}

//
//
//
inline void SndRecorder::RecMODE(unsigned int value)
{
    last_srcdesc->_mode.changed=true;
    last_srcdesc->_mode.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEDIRECT(int value) 
{
    last_srcdesc->_source_direct.changed=true;
    last_srcdesc->_source_direct.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEDIRECTHF(int value)
{
    last_srcdesc->_source_directHF.changed=true;
    last_srcdesc->_source_directHF.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEROOM(int value)
{
    last_srcdesc->_source_room.changed=true;
    last_srcdesc->_source_room.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEROOMHF(int value)
{
    last_srcdesc->_source_roomHF.changed=true;
    last_srcdesc->_source_roomHF.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEROLLOFF(float value)
{
    last_srcdesc->_source_rolloff.changed=true;
    last_srcdesc->_source_rolloff.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEOUTSIDE(int value)
{
    last_srcdesc->_source_outside.changed=true;
    last_srcdesc->_source_outside.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEABSORPTION(float value)
{
    last_srcdesc->_source_absorption.changed=true;
    last_srcdesc->_source_absorption.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEFLAGS(unsigned int value)
{
    last_srcdesc->_source_flags.changed=true;
    last_srcdesc->_source_flags.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEOBSTRUCTION(int value)
{
    last_srcdesc->_source_obstruction.changed=true;
    last_srcdesc->_source_obstruction.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEOBSTRUCTIONLF(float value)
{
    last_srcdesc->_source_obstructionLF.changed=true;
    last_srcdesc->_source_obstructionLF.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEOCCLUSION(int value)
{
    last_srcdesc->_source_occlusion.changed=true;
    last_srcdesc->_source_occlusion.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEOCCLUSIONLF(float value)
{
    last_srcdesc->_source_obstructionLF.changed=true;
    last_srcdesc->_source_obstructionLF.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEOCCLUSIONROOM(float value)
{
    last_srcdesc->_source_occlusion_room.changed=true;
    last_srcdesc->_source_occlusion_room.value=value;
}

//
//
//
inline void SndRecorder::RecPOSX(float value)
{
    last_srcdesc->_posX.changed=true;
    last_srcdesc->_posX.value=value;
}

//
//
//
inline void SndRecorder::RecPOSY(float value)
{
    last_srcdesc->_posY.changed=true;
    last_srcdesc->_posY.value=value;
}

//
//
//
inline void SndRecorder::RecPOSZ(float value)
{
    last_srcdesc->_posZ.changed=true;
    last_srcdesc->_posZ.value=value;
}

//
//
//
inline void SndRecorder::RecSOURCEAFFECTDIRECTHF()
{
    last_srcdesc->_Flags|=sound::S_AFFECT_DIRHF;
}

//
//
//
inline void SndRecorder::RecSOURCEAFFECTROOM()
{
    last_srcdesc->_Flags|=sound::S_AFFECT_ROOM;
}

//
//
//
inline void SndRecorder::RecSOURCEAFFECTROOMHF()
{
    last_srcdesc->_Flags|=sound::S_AFFECT_ROOMHF;
}

//
//
//
inline void SndRecorder::RecLCOORDSYSTEM()
{
    sound::dev->SetCoordinateSystem(sound::LEFT_HANDED);
}

//
//
//
inline void SndRecorder::RecRCOORDSYSTEM()
{
    sound::dev->SetCoordinateSystem(sound::RIGHT_HANDED);
}

//
//
//
inline void SndRecorder::RecPRIORITY(float value)
{
    last_srcdesc->_priority.changed=true;
    last_srcdesc->_priority.value=value;
}

//
//
//
inline void SndRecorder::RecMEMORYSTATIC()
{
    last_srcdesc->_Flags|=sound::STATIC;
}

//////////////////////////////////////////////////////////////////////////
//                          Listener settings                           //
//////////////////////////////////////////////////////////////////////////

using sound::ListenerDefs;

//
//
//
inline void SndRecorder::RecROLLOFF(float value)
{
	ListenerDefs._rolloff.changed=true;
	ListenerDefs._rolloff.value=value;
}

//
//
//
inline void SndRecorder::RecDOPPLERFACTOR(float value)
{
	ListenerDefs._doppler_factor.changed=true;
	ListenerDefs._doppler_factor.value=value;
}

//
//
//
inline void SndRecorder::RecROOM(int value)
{
    ListenerDefs._room.changed=true;
    ListenerDefs._room.value  =value;
}

//
//
//
inline void SndRecorder::RecROOMHF(int value)
{
    ListenerDefs._roomHF.changed=true;
    ListenerDefs._roomHF.value  =value;
}

//
//
//
inline void SndRecorder::RecROOMROLLOFF(float value)
{
    ListenerDefs._room_rolloff.changed=true;
    ListenerDefs._room_rolloff.value  =value;
}

//
//
//
inline void SndRecorder::RecDECAYTIME(float value)
{
    ListenerDefs._decay_time.changed=true;
    ListenerDefs._decay_time.value  =value;
}

//
//
//
inline void SndRecorder::RecDECAYHFRATIO(float value)
{
    ListenerDefs._decay_HFRatio.changed=true;
    ListenerDefs._decay_HFRatio.value  =value;
}

//
//
//
inline void SndRecorder::RecREFLECTIONS(int value)
{
    ListenerDefs._reflections.changed=true;
    ListenerDefs._reflections.value  =value;
}

//
//
//
inline void SndRecorder::RecREFLECTIONSDELAY(float value)
{
    ListenerDefs._reflections_delay.changed=true;
    ListenerDefs._reflections_delay.value  =value;
}

//
//
//
inline void SndRecorder::RecREVERB(int value)
{
    ListenerDefs._reverb.changed=true;
    ListenerDefs._reverb.value  =value;
}

//
//
//
inline void SndRecorder::RecREVERBDELAY(float value)
{
    ListenerDefs._reverb_delay.changed=true;
    ListenerDefs._reverb_delay.value  =value;
}

//
//
//
inline void SndRecorder::RecENVIROMENT(unsigned int value)
{
    ListenerDefs._environment.changed=true;
    ListenerDefs._environment.value  =value;
}

//
//
//
inline void SndRecorder::RecENVIROMENTSIZE(float value)
{
    ListenerDefs._environment_size.changed=true;
    ListenerDefs._environment_size.value  =value;
}

//
//
//
inline void SndRecorder::RecENVIROMENTDIFFUSION(float value)
{
    ListenerDefs._environment_diffusion.changed=true;
    ListenerDefs._environment_diffusion.value  =value;
}

//
//
//
inline void SndRecorder::RecAIRABSOPTION(float value)
{
    ListenerDefs._air_absorption.changed=true;
    ListenerDefs._air_absorption.value  =value;
}

//
//
//
inline void SndRecorder::RecFLAGS(unsigned int value)
{
    ListenerDefs._flags.changed=true;
    ListenerDefs._flags.value  =value;
}

//
//
//
inline void SndRecorder::RecSCALETIME()
{
    ListenerDefs._lflags|=sound::SCALE_DECAY_TIME;
}

//
//
//
inline void SndRecorder::RecSCALEREFLECTIONS()
{
    ListenerDefs._lflags|=sound::SCALE_REFLECTIONS;
}

//
//
//
inline void SndRecorder::RecSCALEREFLECTIONSDELAY()
{
    ListenerDefs._lflags|=sound::SCALE_REFLECTIONS_DELAY;
}

//
//
//
inline void SndRecorder::RecSCALEREVERB()
{
    ListenerDefs._lflags|=sound::SCALE_REVERB;
}

//
//
//
inline void SndRecorder::RecSCALEREVERBDELAY()
{
    ListenerDefs._lflags|=sound::SCALE_REVERB_DELAY;
}

//
//
//
inline void SndRecorder::RecCLIPDECAYHF()
{
    ListenerDefs._lflags|=sound::CLIP_DECAY_HF;
}

#endif __SNDRECORDER_H__