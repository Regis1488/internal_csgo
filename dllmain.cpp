#include <iostream>
#include <Windows.h>
#include "Header.h"
#include "Vector.h"

world GameWorld;
Offsets offset;
entity LocalPlayer;

const int SCREEN_WIDTH = GetSystemMetrics(SM_CXSCREEN); const int xhairx = SCREEN_WIDTH / 2;
const int SCREEN_HEIGHT = GetSystemMetrics(SM_CYSCREEN); const int xhairy = SCREEN_HEIGHT / 2;

constexpr Vector3 CalculateAngle(
	const Vector3& localPosition,
	const Vector3& enemyPosition,
	const Vector3& viewAngles) noexcept
{
	return ((enemyPosition - localPosition).ToAngle() - viewAngles);
}

template<typename T> T RPM(uintptr_t adress) {
	try { return *(T*)adress;}
	catch (...) { return T(); }
}

uintptr_t getLocalPlayer() {
	return RPM<uintptr_t>(GameWorld.GameModule + offset.dwLocalPlayer);
}

uintptr_t getPlayer(int index) {
	return RPM<uintptr_t>(GameWorld.GameModule + offset.dwEntityList + index * 0x10);
}
int getTeam(uintptr_t player) {
	return RPM<int>(player + offset.m_iTeamNum);
}

int getCrosshairId(uintptr_t player){
	return RPM<int>(player + offset.m_iCrosshairId);
}
int getFlags(uintptr_t player) 
{
	return RPM<uintptr_t>(player + offset.m_fFlags);
}
Vector3 PlayerLocation(uintptr_t player)
{
	return RPM<Vector3>(player + offset.m_vecOrigin);
}

Vector3 get_head(uintptr_t player)
{
	struct boneMatrix_t {
		byte pad3[12];
		float x;
		byte pad2[12];
		float y;
		byte pad1[12];
		float z;
	};
	uintptr_t boneBase = RPM<uintptr_t>(player + offset.m_dwBoneMatrix);
	boneMatrix_t BoneMatrix = RPM<boneMatrix_t>(boneBase + sizeof(sizeof(BoneMatrix) * 8));
	return Vector3(BoneMatrix.x, BoneMatrix.y, BoneMatrix.z);
}

struct view_matrix_t {
	float matrix[16];
}vm;
struct Vector3 WorldToScreen(const struct Vector3 pos, struct view_matrix_t matrix) {
	struct Vector3 out;
	float _x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
	float _y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
	out.z = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];

	_x *= 1.f / out.z;
	_y *= 1.f / out.z;
	out.z = SCREEN_WIDTH * .5f;
	out.y = SCREEN_HEIGHT * .5f;

	return out;
}
float pythag(int x1, int y1, int x2, int y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

int FindClosestEnemy() {
	float Finish;
	int ClosestEntity = 1;
	Vector3 Calc = { 0, 0, 0 };
	float Closest = FLT_MAX;
	int localTeam = getTeam(GetLocalPlayer());
	for (int i = 1; i < 64; i++) { //Loops through all the entitys in the index 1-64.
		DWORD Entity = GetPlayer(i);
		int EnmTeam = getTeam(Entity); if (EnmTeam == localTeam) continue;
		int EnmHealth = GetPlayerHealth(Entity); if (EnmHealth < 1 || EnmHealth > 100) continue;
		int Dormant = DormantCheck(Entity); if (Dormant) continue;
		Vector3 headBone = WorldToScreen(get_head(Entity), vm);
		Finish = pythag(headBone.x, headBone.y, xhairx, xhairy);
		if (Finish < Closest) {
			Closest = Finish;
			ClosestEntity = i;
		}
	}

	return ClosestEntity;
}

void DrawLine(float StartX, float StartY, float EndX, float EndY) { //This function is optional for debugging.
	int a, b = 0;
	HPEN hOPen;
	HPEN hNPen = CreatePen(PS_SOLID, 2, 0x0000FF /*red*/);
	hOPen = (HPEN)SelectObject(hdc, hNPen);
	MoveToEx(hdc, StartX, StartY, NULL); //start of line
	a = LineTo(hdc, EndX, EndY); //end of line
	DeleteObject(SelectObject(hdc, hOPen));
}

void FindClosestEnemyThread() {
	while (1) {
		closest = FindClosestEnemy();
	}
}

DWORD WINAPI MainThread(HMODULE hModule)
{
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);

	std::cout << "Hello";

	GameWorld.GameModule = (DWORD)GetModuleHandle("client.dll");

	while (true)
	{
		if (GetAsyncKeyState(VK_END) & 1)
		{
			
		}
		int CrosshairID = getCrosshairId(getLocalPlayer());
		int CrosshairTeam = getTeam(getPlayer(CrosshairID - 1));
		int LocalTeam = getTeam(getLocalPlayer());
		int ground = getFlags(getLocalPlayer());
		DWORD GlowObjectManager = *(DWORD*)(GameWorld.GameModule + offset.dwGlowObjectManager);
		if (CrosshairID > 0 && CrosshairID < 32 && LocalTeam != CrosshairTeam)
		{
		//	if (GetAsyncKeyState(VK_MENU))
		//	{

			
			mouse_event(MOUSEEVENTF_LEFTDOWN, NULL, NULL, 0, 0);
			mouse_event(MOUSEEVENTF_LEFTUP, NULL, NULL, 0, 0);
			Sleep(100);
			//	}

		}
		if (GetAsyncKeyState(VK_SPACE))
		{
			LocalPlayer.EntityPtr = *(DWORD*)(GameWorld.GameModule + offset.dwLocalPlayer);

			if (LocalPlayer.EntityPtr != NULL) {
				LocalPlayer.health = *(int*)(LocalPlayer.EntityPtr + offset.m_iHealth);
				LocalPlayer.flags = *(int*)(LocalPlayer.EntityPtr + offset.m_fFlags);
				if (LocalPlayer.health > 0 && LocalPlayer.flags == 257) {

					LocalPlayer.ForceJump(GameWorld.GameModule);
				}
			}
		}
		for (int i = 1; i < 32; i++)
		{
			DWORD entity = *(DWORD*)((GameWorld.GameModule + offset.dwEntityList) + i * 0x10);
			if (entity == NULL) continue;

			int glowIndex = *(int*) (entity + offset.m_iGlowIndex); 
			int entityTeam = *(int*)(entity + offset.m_iTeamNum);

			if (entityTeam == LocalTeam)
			{
				*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x8)) = 0.f; //R
				*(float*)((GlowObjectManager + glowIndex * 0x38 + 0xC)) = 1.f;
				*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x10)) = 0.f;
				*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x14)) = 1.7f;
			}
			else {
				*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x8)) = 1.f; //R
				*(float*)((GlowObjectManager + glowIndex * 0x38 + 0xC)) = 0.f;
				*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x10)) = 0.f;
				*(float*)((GlowObjectManager + glowIndex * 0x38 + 0x14)) = 1.7f;
			}
			*(bool*)((GlowObjectManager + glowIndex * 0x38 + 0x28)) = true;
			*(bool*)((GlowObjectManager + glowIndex * 0x38 + 0x29)) = false;
		}

	}

	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
}