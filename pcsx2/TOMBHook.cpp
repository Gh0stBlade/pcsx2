#include "TOMBHook.h"

#include "PrecompiledHeader.h"
#include "IopCommon.h"
#include "App.h" // For host irx injection hack
#include "R3000A.h"

#include <stdio.h>

using namespace R3000A;

enum GP
{
	TOMB4 = 0xA51E4,
	TOMB5 = 0xA05B4
};

/*
 * Emulate SIO
 */

void PCRead(FILE* fp, u32 addr, u32 size, u32& ret)
{
	ret = fread(iopVirtMemW<char*>(addr), size, 1, fp);
}

void PCOpen(std::string filePath, u32 accessMode, u32& ret)
{
	FILE* fileHandle = NULL;

	filePath = "sio\\" + filePath;

	/*
	* HACK - Removes "\\" from string, causes file open issues with CUTSEQ.JIZ
	*/

	if (filePath[0] == '\\')
	{
		filePath.erase(0, 1);
	}

	switch (accessMode)
	{
	case 0://Read only
		fileHandle = fopen(filePath.c_str(), "rb");
		break;
	case 1://Write Only
		fileHandle = fopen(filePath.c_str(), "wb");
		break;
	case 2://Read & Write
		fileHandle = fopen(filePath.c_str(), "rb+");
		break;
	default:
		Console.WriteLn("Error, unknown accessMode: %d\n", accessMode);
		break;
	}

	ret = (u32)fileHandle;

	if (ret == 0)
		ret = -1;
}

void PClSeek(FILE* fp, u32 offset, u32 origin, u32& ret)
{
	fseek(fp, offset, origin);
	ret = (u32)ftell(fp);
}

void PCClose(FILE* fp, u32& ret)
{
	ret = (u32)fclose(fp);
}

bool HandleTOMB5(u32& branchPC)
{
	if (branchPC == 0x784F4 || branchPC == 0x68AC0)
	{
		PCOpen(iopMemReadString(psxRegs.GPR.n.a0), psxRegs.GPR.n.a1, psxRegs.GPR.n.v0);
		psxRegs.pc = psxRegs.GPR.n.ra;
		return true;
	}
	else if (/*branchPC == 0x78530*/ || branchPC == 0x68AE0)
	{
		PClSeek((FILE*)psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.v0);
		psxRegs.pc = psxRegs.GPR.n.ra;
		return true;
	}
	else if (branchPC == 0x78660 || branchPC == 0x68B04)
	{
		PCClose((FILE*)psxRegs.GPR.n.a0, psxRegs.GPR.n.v0);
		psxRegs.pc = psxRegs.GPR.n.ra;
		return true;
	}
	else if (branchPC == 0x785A8 || branchPC == 0x6BB00)
	{
		PCRead((FILE*)psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.v0);
		psxRegs.pc = psxRegs.GPR.n.ra;
		return true;
	}

	return false;
}

bool HandleTOMB4(u32& branchPC)
{
	if (branchPC == 0x8622C || branchPC == 0x6ED70)
	{
		PCOpen(iopMemReadString(psxRegs.GPR.n.a0), psxRegs.GPR.n.a1, psxRegs.GPR.n.v0);
		psxRegs.pc = psxRegs.GPR.n.ra;
		return true;
	}
	else if (/*branchPC == 0x6EDA0 ||*/ branchPC == 0x6EDA0)//FIXME must match 0x78530
	{
		PClSeek((FILE*)psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.v0);
		psxRegs.pc = psxRegs.GPR.n.ra;
		return true;
	}
	else if (branchPC == 0x86398 || branchPC == 0x6EDC4)
	{
		PCClose((FILE*)psxRegs.GPR.n.a0, psxRegs.GPR.n.v0);
		psxRegs.pc = psxRegs.GPR.n.ra;
		return true;
	}
	else if (branchPC == 0x862E0 || branchPC == 0x71CD4)

	{
		PCRead((FILE*)psxRegs.GPR.n.a0, psxRegs.GPR.n.a1, psxRegs.GPR.n.a2, psxRegs.GPR.n.v0);
		psxRegs.pc = psxRegs.GPR.n.ra;
		return true;
	}

	return false;
}

void HandleBranch(u32& branchPC)
{
	switch (psxRegs.GPR.n.gp)
	{
	case GP::TOMB4:
	{
		if (HandleTOMB4(branchPC))
			return;
	}
	break;
	case GP::TOMB5:
		if (HandleTOMB5(branchPC))
			return;
		break;
	}

	psxRegs.pc = branchPC;
}