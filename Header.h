#pragma once
#include <Windows.h>
DWORD dwForceJump = 0x5278DDC;
struct world {
	DWORD GameModule = (DWORD)GetModuleHandle("client.dll");
	DWORD GameModuleEngine = (DWORD)GetModuleHandle("engine.dll");
};

struct entity {
public:
	int health;
	int flags;
	DWORD EntityPtr;

	void ForceJump(DWORD GameModule)
	{
		*(int*)(GameModule + dwForceJump) = 2;
	}


};
struct Offsets {
public:
DWORD dwLocalPlayer = 0xDB35DC;
DWORD m_iHealth = 0x100;
DWORD m_fFlags = 0x104;
DWORD dwEntityList = 0x4DCEEAC;
DWORD m_iCrosshairId = 0x11838;
DWORD m_iTeamNum = 0xF4;
DWORD dwGlowObjectManager = 0x5317308;
DWORD m_iGlowIndex = 0x10488;
DWORD m_vecOrigin = 0x138;
DWORD m_dwBoneMatrix = 0x26A8;
};