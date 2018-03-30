#pragma once

#include "IopCommon.h"

extern void PCRead(FILE* fp, u32 addr, u32 size, u32& ret);
extern void PCOpen(std::string filePath, u32 accessMode, u32& ret);
extern void PClSeek(FILE* fp, u32 offset, u32 origin, u32& ret);
extern void PCClose(FILE* fp, u32& ret);
extern void HandleBranch(u32& branchPC);