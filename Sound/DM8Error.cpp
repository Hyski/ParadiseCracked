#include "precomp.h"
#include "DM8Error.h"
#include <dmerror.h>
#include "DX8Sound.h"

#include <sstream>

cc_SoundError::cc_SoundError(HRESULT hr, const char *message)
:	cc_Unexpected(cc_nobreak())
{
	std::ostringstream sstr;
	sstr << message << " : " << cc_DM8Error::getDescription(hr) << std::ends;
	setMessage(sstr.str().c_str());
}

const char *cc_DM8Error::getDescription(HRESULT hr)
{
	switch(hr)
	{
		case CLASS_E_NOAGGREGATION: 
			return "Aggregation is not supported. The LPUNKNOWN parameter should be set to NULL.";
		case DMUS_E_ALL_TOOLS_FAILED: 
			return "The graph object was unable to load all tools from the IStream object data, perhaps because of errors in the stream or because the tools are incorrectly registered on the client.";
		case DMUS_E_ALL_TRACKS_FAILED: 
			return "The segment object was unable to load all tracks from the IStream object data, perhaps because of errors in the stream or because the tracks are incorrectly registered on the client.";
		case DMUS_E_ALREADY_ACTIVATED: 
			return "The port has been activated, and the parameter cannot be changed.";
		case DMUS_E_ALREADY_DOWNLOADED: 
			return "The buffer has already been downloaded.";
		case DMUS_E_ALREADY_EXISTS: 
			return "The tool is already contained in the graph. You must create a new instance.";
		case DMUS_E_ALREADY_INITED: 
			return "The object has already been initialized.";
		case DMUS_E_ALREADY_LOADED :
			return "A DLS collection is already open. ";
		case DMUS_E_ALREADY_SENT :
			return "The message has already been sent. ";
		case DMUS_E_ALREADYCLOSED :
			return "The port is not open. ";
		case DMUS_E_ALREADYOPEN :
			return "The port is already open. ";
		case DMUS_E_AUDIOPATH_INACTIVE :
			return "The audiopath is inactive, perhaps because the performance has been closed down. ";
		case DMUS_E_AUDIOPATH_NOBUFFER :
			return "The audiopath could not be created because a requested buffer could not be created. ";
		case DMUS_E_AUDIOPATH_NOGLOBALFXBUFFER :
			return "An attempt was made to create an audiopath that sends to a nonexistent global effects buffer. ";
		case DMUS_E_AUDIOPATH_NOPORT :
			return "The audiopath could not be used for playback because it lacked port assignments. ";
		case DMUS_E_AUDIOPATHS_IN_USE :
			return "The performance has set up audiopaths, so performance channels cannot be allocated. ";
		case DMUS_E_AUDIOPATHS_NOT_VALID :
			return "Performance channels have been set up by using IDirectMusicPerformance8::AssignPChannel, so the performance cannot support audiopaths. ";
		case DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE :
			return "A script routine written in AudioVBScript failed because a function outside the script failed to complete. ";
		case DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR :
			return "A script routine written in AudioVBScript failed because an invalid operation occurred—for example, adding an integer to an object, or attempting to call a routine that does not exist. ";
		case DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR :
			return "A script routine written in AudioVBScript could not be read because it contained a statement not allowed by the language. ";
		case DMUS_E_BADARTICULATION :
			return "Invalid articulation chunk in DLS collection. ";
		case DMUS_E_BADINSTRUMENT :
			return "Invalid instrument chunk in DLS collection. ";
		case DMUS_E_BADOFFSETTABLE :
			return "The offset table has errors. ";
		case DMUS_E_BADWAVE :
			return "Corrupt wave header. ";
		case DMUS_E_BADWAVELINK :
			return "The wave-link chunk in DLS collection points to invalid wave. ";
		case DMUS_E_BUFFER_EMPTY :
			return "There is no data in the buffer. ";
		case DMUS_E_BUFFER_FULL :
			return "The specified number of bytes exceeds the maximum buffer size. ";
		case DMUS_E_BUFFERNOTAVAILABLE :
			return "The buffer is not available for download. ";
		case DMUS_E_BUFFERNOTSET :
			return "No buffer was prepared for the data. ";
		case DMUS_E_CANNOT_CONVERT :
			return "The requested conversion between music and MIDI values could not be made. This usually occurs when the provided DMUS_CHORD_KEY structure has an invalid chord or scale pattern. ";
		case DMUS_E_CANNOT_FREE :
			return "The message could not be freed, either because it was not allocated or because it has already been freed. ";
		case DMUS_E_CANNOT_OPEN_PORT :
			return "The default system port could not be opened. ";
		case DMUS_E_CANNOTREAD :
			return "An error occurred when trying to read from the IStream object. ";
		case DMUS_E_CANNOTSEEK :
			return "The IStream object does not support Seek. ";
		case DMUS_E_CANNOTWRITE :
			return "The IStream object does not support Write. ";
		case DMUS_E_CHUNKNOTFOUND :
			return "A chunk with the specified header could not be found. ";
		case DMUS_E_DESCEND_CHUNK_FAIL :
			return "An attempt to descend into a chunk failed. ";
		case DMUS_E_DEVICE_IN_USE :
			return "The device is in use, possibly by a non-DirectMusic client, and cannot be opened again. ";
		case DMUS_E_DMUSIC_RELEASED :
			return "The operation cannot be performed because the final instance of the DirectMusic object was released. Ports cannot be used after final release of the DirectMusic object. ";
		case DMUS_E_DRIVER_FAILED :
			return "An unexpected error was returned from a device driver, indicating possible failure of the driver or hardware. ";
		case DMUS_E_DSOUND_ALREADY_SET :
			return "A DirectSound object has already been set. ";
		case DMUS_E_DSOUND_NOT_SET :
			return "The port could not be created because no DirectSound object has been specified. ";
		case DMUS_E_GET_UNSUPPORTED :
			return "Getting the parameter is not supported. ";
		case DMUS_E_INSUFFICIENTBUFFER :
			return "The buffer is not large enough for the requested operation. ";
		case DMUS_E_INVALID_BAND :
			return "The file does not contain a valid band. ";
		case DMUS_E_INVALID_CONTAINER_OBJECT :
			return "The file does not contain a valid container object. ";
		case DMUS_E_INVALID_DOWNLOADID :
			return "An invalid download identifier was used in the process of creating a download buffer. ";
		case DMUS_E_INVALID_EVENT :
			return "The event either is not a valid MIDI message or makes use of running status and cannot be packed into the buffer. ";
		case DMUS_E_INVALID_LYRICSTRACK :
			return "The file contains an invalid lyrics track. ";
		case DMUS_E_INVALID_PARAMCONTROLTRACK :
			return "The file contains an invalid parameter control track. ";
		case DMUS_E_INVALID_SCRIPTTRACK :
			return "The file contains an invalid script track. ";
		case DMUS_E_INVALID_SEGMENTTRIGGERTRACK :
			return "The file contains an invalid segment trigger track. ";
		case DMUS_E_INVALID_TOOL_HDR :
			return "The IStream object's data contains an invalid tool header and cannot be read by the graph object. ";
		case DMUS_E_INVALID_TRACK_HDR :
			return "The IStream object's data contains an invalid track header and cannot be read by the segment object. ";
		case DMUS_E_INVALIDBUFFER :
			return "An invalid DirectSound buffer was handed to a port. ";
		case DMUS_E_INVALIDCHUNK :
			return "Invalid data was found in a RIFF file chunk. ";
		case DMUS_E_INVALIDFILE :
			return "Not a valid file. ";
		case DMUS_E_INVALIDOFFSET :
			return "Wave chunks in the DLS collection file are at incorrect offsets. ";
		case DMUS_E_INVALIDPATCH :
			return "No instrument in the collection matches the patch number. ";
		case DMUS_E_INVALIDPOS :
			return "Error reading wave data from a DLS collection. Indicates bad file. ";
		case DMUS_E_LOADER_BADPATH :
			return "The file path is invalid. ";
		case DMUS_E_LOADER_FAILEDCREATE :
			return "The object could not be found or created. ";
		case DMUS_E_LOADER_FAILEDOPEN :
			return "File open failed because the file does not exist or is locked. ";
		case DMUS_E_LOADER_FORMATNOTSUPPORTED :
			return "The object cannot be loaded because the data format is not supported. ";
		case DMUS_E_LOADER_NOCLASSID :
			return "No class identifier was supplied in the object description. ";
		case DMUS_E_LOADER_NOFILENAME :
			return "No file name was supplied in the object description. ";
		case DMUS_E_LOADER_OBJECTNOTFOUND :
			return "The object was not found. ";
		case DMUS_E_NO_AUDIOPATH :
			return "An attempt was made to play on a nonexistent audiopath. ";
		case DMUS_E_NO_AUDIOPATH_CONFIG :
			return "The object does not contain an embedded audiopath configuration. ";
		case DMUS_E_NO_MASTER_CLOCK :
			return "There is no master clock in the performance. Be sure to call the IDirectMusicPerformance8::Init method. ";
		case DMUS_E_NOARTICULATION :
			return "Articulation missing from an instrument in the DLS collection. ";
		case DMUS_E_NOSYNTHSINK :
			return "No sink is connected to the synthesizer. ";
		case DMUS_E_NOT_DOWNLOADED_TO_PORT :
			return "The object cannot be unloaded because it is not present on the port. ";
		case DMUS_E_NOT_FOUND :
			return "The requested item is not contained by the object. ";
		case DMUS_E_NOT_INIT :
			return "A required object is not initialized or failed to initialize. ";
		case DMUS_E_NOT_LOADED :
			return "An attempt to use this object failed because it was not loaded. ";
		case DMUS_E_NOTADLSCOL :
			return "The object being loaded is not a valid DLS collection. ";
		case DMUS_E_NOTMONO :
			return "The wave chunk has more than one interleaved channel. DLS format requires mono. ";
		case DMUS_E_NOTPCM :
			return "Wave data is not in PCM format. ";
		case DMUS_E_OUT_OF_RANGE :
			return "The requested time is outside the range of the segment. ";
		case DMUS_E_PORT_NOT_CAPTURE :
			return "The port is not a capture port. ";
		case DMUS_E_PORT_NOT_RENDER :
			return "The port is not an output port. ";
		case DMUS_E_PORTS_OPEN :
			return "The requested operation cannot be performed while there are instantiated ports in any process in the system. ";
		case DMUS_E_SCRIPT_CANTLOAD_OLEAUT32 :
			return "Loading of Oleaut32.dll failed. ActiveX scripting languages require use of oleaut32.dll. On platforms where this file is not present, only the AudioVBScript language can be used. ";
		case DMUS_E_SCRIPT_CONTENT_READONLY :
			return "Script variables for content referenced or embedded in a script cannot be set. ";
		case DMUS_E_SCRIPT_ERROR_IN_SCRIPT :
			return "An error was encountered while parsing or executing the script. ";
		case DMUS_E_SCRIPT_INVALID_FILE :
			return "The script file is invalid. ";
		case DMUS_E_SCRIPT_LANGUAGE_INCOMPATIBLE :
			return "The ActiveX scripting engine for the script's language is not compatible with DirectMusic. ";
		case DMUS_E_SCRIPT_LOADSCRIPT_ERROR :
			return "The script that was loaded contains an error. ";
		case DMUS_E_SCRIPT_NOT_A_REFERENCE :
			return "An attempt was made to set a script's variable by reference to a value that is not an object type. ";
		case DMUS_E_SCRIPT_ROUTINE_NOT_FOUND :
			return "The script does not contain a routine with the specified name. ";
		case DMUS_E_SCRIPT_UNSUPPORTED_VARTYPE :
			return "A variant was used that had a type not supported by DirectMusic. ";
		case DMUS_E_SCRIPT_VALUE_NOT_SUPPORTED :
			return "An attempt was made to set a script's variable by value to an object that does not support a default value property. ";
		case DMUS_E_SCRIPT_VARIABLE_NOT_FOUND :
			return "The script does not contain a variable with the specified name. ";
		case DMUS_E_SEGMENT_INIT_FAILED :
			return "Segment initialization failed, probably because of a critical memory situation. ";
		case DMUS_E_SET_UNSUPPORTED :
			return "Setting the parameter is not supported. ";
		case DMUS_E_SYNTHACTIVE :
			return "The synthesizer has been activated, and the parameter cannot be changed. ";
		case DMUS_E_SYNTHINACTIVE :
			return "The synthesizer has not been activated and cannot process data. ";
		case DMUS_E_SYNTHNOTCONFIGURED :
			return "The synthesizer is not properly configured or opened. ";
		case DMUS_E_TIME_PAST :
			return "The time requested is in the past. ";
		case DMUS_E_TOOL_HDR_NOT_FIRST_CK :
			return "The stream object's data does not have a tool header as the first chunk and, therefore, cannot be read by the graph object. ";
		case DMUS_E_TRACK_HDR_NOT_FIRST_CK :
			return "The stream object's data does not have a track header as the first chunk and, therefore, cannot be read by the segment object. ";
		case DMUS_E_TRACK_NO_CLOCKTIME_SUPPORT :
			return "The track does not support clock-time playback or parameter retrieval. ";
		case DMUS_E_TRACK_NOT_FOUND :
			return "There is no track of the requested type. ";
		case DMUS_E_TYPE_DISABLED :
			return "A track parameter is unavailable because it has been disabled. ";
		case DMUS_E_TYPE_UNSUPPORTED :
			return "Parameter is unsupported on this track. ";
		case DMUS_E_UNKNOWN_PROPERTY :
			return "The property set or item is not implemented by this port. ";
		case DMUS_E_UNKNOWNDOWNLOAD :
			return "The synthesizer does not support this type of download. ";
		case DMUS_E_UNSUPPORTED_STREAM :
			return "The IStream object does not contain data supported by the loading object. ";
		case DMUS_E_WAVEFORMATNOTSUPPORTED :
			return "Invalid buffer format was handed to the synthesizer sink. ";
		case DMUS_S_DOWN_OCTAVE :
			return "The note has been lowered by one or more octaves to fit within the range of MIDI values. ";
		case DMUS_S_END :
			return "The operation succeeded and reached the end of the data. ";
		case DMUS_S_FREE :
			return "The allocated memory should be freed. ";
		case DMUS_S_GARBAGE_COLLECTED :
			return "The requested operation was not performed because the object has been released. ";
		case DMUS_S_LAST_TOOL :
			return "There are no more tools in the graph. ";
		case DMUS_S_NOBUFFERCONTROL :
			return "Although the audio output from the port is routed to the same device as the given DirectSound buffer, buffer controls such as pan and volume do not affect the output. ";
		case DMUS_S_OVER_CHORD :
			return "No MIDI values have been calculated because the music value has the note at a position higher than the top note of the chord. ";
		case DMUS_S_PARTIALDOWNLOAD :
			return "Some instruments could not be downloaded to the port. ";
		case DMUS_S_PARTIALLOAD :
			return "The object could only load partially. This can happen if some components, such as embedded tracks and tools, are not registered properly. It can also happen if some content is missing; for example, if a segment uses a DLS collection that is not in the loader's current search directory. ";
		case DMUS_S_REQUEUE :
			return "The message should be passed to the next tool. ";
		case DMUS_S_STRING_TRUNCATED :
			return "The method succeeded, but the returned string was truncated. ";
		case DMUS_S_UP_OCTAVE :
			return "The note has been raised by one or more octaves to fit within the range of MIDI values. ";
		case E_FAIL :
			return "The method did not succeed. ";
		case E_INVALIDARG :
			return "Invalid argument. Often, this error results from failing to initialize the dwSize member of a structure before passing it to the method. ";
		case E_NOINTERFACE :
			return "No object interface is available. ";
		case E_NOTIMPL :
			return "The method is not implemented. This value might be returned if a driver does not support a feature necessary for the operation. ";
		case E_OUTOFMEMORY :
			return "Insufficient memory to complete the task. ";
		case E_POINTER :
			return "An invalid pointer, usually NULL, was passed as a parameter. ";
		case REGDB_E_CLASSNOTREG :
			return "The object class is not registered. ";
		case S_FALSE :
			return "The method succeeded, but there was nothing to do. ";
		case S_OK :
			return "The operation was completed successfully.";
		default:
			return "Unknown error";
	}
}

void cc_DM8Error::safe_call(HRESULT hr, const char *msg)
{
	if (FAILED(hr)) throw SND_EXCEPTION(std::string(msg)+getDescription(hr));
}

std::string cc_DM8Error::getMusicTime()
{
	MUSIC_TIME time = cc_DX8Sound::m_Sound->getTime();
	char buffer[16];
	sprintf(buffer,"%010u",time);
	return std::string(buffer);
}

std::string cc_DM8Error::unsigned2String(unsigned n)
{
	char buffer[16];
	sprintf(buffer,"%u",n);
	return std::string(buffer);
}

std::string cc_DM8Error::hexstr(const void *ptr)
{
	char buffer[16];
	sprintf(buffer,"{%08X} ",ptr);
	return std::string(buffer);
}