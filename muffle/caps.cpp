#include "precomp.h"
#include "caps.h"
#include "dsound.h"

namespace
{
	struct FlagsMap
	{
		unsigned flag;
		const char *desc;
	};

	const FlagsMap fmap[] =
	{
		{DSCAPS_CERTIFIED,"DSCAPS_CERTIFIED"},
		{DSCAPS_CONTINUOUSRATE,"DSCAPS_CONTINUOUSRATE"},
		{DSCAPS_EMULDRIVER,"DSCAPS_EMULDRIVER"},
		{DSCAPS_PRIMARY16BIT,"DSCAPS_PRIMARY16BIT"},
		{DSCAPS_PRIMARY8BIT,"DSCAPS_PRIMARY8BIT"},
		{DSCAPS_PRIMARYMONO,"DSCAPS_PRIMARYMONO"},
		{DSCAPS_PRIMARYSTEREO,"DSCAPS_PRIMARYSTEREO"},
		{DSCAPS_SECONDARY16BIT,"DSCAPS_SECONDARY16BIT"},
		{DSCAPS_SECONDARY8BIT,"DSCAPS_SECONDARY8BIT"},
		{DSCAPS_SECONDARYMONO,"DSCAPS_SECONDARYMONO"},
		{DSCAPS_SECONDARYSTEREO,"DSCAPS_SECONDARYSTEREO"},
	};

//=====================================================================================//
//                            std::string getFlagsString()                             //
//=====================================================================================//
	void dumpFlagsString(unsigned flags)
	{
		bool needcnv = false;
		for(unsigned i = 0; i < sizeof(fmap)/sizeof(FlagsMap); ++i)
		{
			if(fmap[i].flag & flags)
			{
				if(needcnv)
				{
					KERNEL_LOG("|");
				}
				else
				{
					needcnv = true;
				}
				KERNEL_LOG(fmap[i].desc);
			}
		}
	}
}

//=====================================================================================//
//                                   void dumpCaps()                                   //
//=====================================================================================//
void dumpCaps(const DirectSound &dsnd)
{
	DSCAPS caps;
	caps.dwSize = sizeof(caps);
	dsnd.dsound()->GetCaps(&caps);

	KERNEL_LOG("Возможности устройства:\n");
	KERNEL_LOG("\tFlags = ");
	dumpFlagsString(caps.dwFlags);
	KERNEL_LOG("\n");

	KERNEL_LOG("\tMinSecondarySampleRate = " << caps.dwMinSecondarySampleRate << "\n");
	KERNEL_LOG("\tMaxSecondarySampleRate = " << caps.dwMaxSecondarySampleRate << "\n");
	KERNEL_LOG("\tPrimaryBuffers = " << caps.dwPrimaryBuffers << "\n");
	KERNEL_LOG("\tMaxHwMixingAllBuffers = " << caps.dwMaxHwMixingAllBuffers << "\n");
	KERNEL_LOG("\tMaxHwMixingStaticBuffers = " << caps.dwMaxHwMixingStaticBuffers << "\n");
	KERNEL_LOG("\tMaxHwMixingStreamingBuffers = " << caps.dwMaxHwMixingStreamingBuffers << "\n");
	KERNEL_LOG("\tFreeHwMixingAllBuffers = " << caps.dwFreeHwMixingAllBuffers << "\n");
	KERNEL_LOG("\tFreeHwMixingStaticBuffers = " << caps.dwFreeHwMixingStaticBuffers << "\n");
	KERNEL_LOG("\tFreeHwMixingStreamingBuffers = " << caps.dwFreeHwMixingStreamingBuffers << "\n");
	KERNEL_LOG("\tMaxHw3DAllBuffers = " << caps.dwMaxHw3DAllBuffers << "\n");
	KERNEL_LOG("\tMaxHw3DStaticBuffers = " << caps.dwMaxHw3DStaticBuffers << "\n");
	KERNEL_LOG("\tMaxHw3DStreamingBuffers = " << caps.dwMaxHw3DStreamingBuffers << "\n");
	KERNEL_LOG("\tFreeHw3DAllBuffers = " << caps.dwFreeHw3DAllBuffers << "\n");
	KERNEL_LOG("\tFreeHw3DStaticBuffers = " << caps.dwFreeHw3DStaticBuffers << "\n");
	KERNEL_LOG("\tFreeHw3DStreamingBuffers = " << caps.dwFreeHw3DStreamingBuffers << "\n");
	KERNEL_LOG("\tTotalHwMemBytes = " << caps.dwTotalHwMemBytes << "\n");
	KERNEL_LOG("\tFreeHwMemBytes = " << caps.dwFreeHwMemBytes << "\n");
	KERNEL_LOG("\tMaxContigFreeHwMemBytes = " << caps.dwMaxContigFreeHwMemBytes << "\n");
	KERNEL_LOG("\tUnlockTransferRateHwBuffers = " << caps.dwUnlockTransferRateHwBuffers << "\n");
	KERNEL_LOG("\tPlayCpuOverheadSwBuffers = " << caps.dwPlayCpuOverheadSwBuffers << "\n");
}