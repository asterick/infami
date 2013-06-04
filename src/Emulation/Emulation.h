#ifndef __EMULATION_H
#define __EMULATION_H

struct TimingSettings
{
	int SystemClock;
	int FrameRate;

	int CPUDivider;
	int PPUDivider;
	int BlankingLines;
	int FrameCount;
};

const TimingSettings NTSCTiming = { 21477270, 60, 12, 4, 20,  89490 };
const TimingSettings PALTiming  = { 26601171, 50, 16, 5, 70, 110840 };

#include "Controller.h"

#include "SquareAPU.h"
#include "TriangleAPU.h"
#include "NoiseAPU.h"
#include "DeltaModAPU.h"
#include "APU.h"

#include "PPU.h"

#include "R6502.h"

#include "Famicom.h"
#include "ROM.h"

#include <stdio.h>
#include <memory.h>

#endif