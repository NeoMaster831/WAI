#include "pch.h"
#include <iostream>
#include <TlHelp32.h>
#include <vector>
#include <cmath>

#define FLY_SPEED 1.0f
#define SWAP(a,b) {float t; t=a; a=b; b=t;}

struct Vector3 {
    float x, y, z;
};

void lockLocation(float lockX, float lockY, float lockZ, float* lockAddrX, float* lockAddrY, float* lockAddrZ) {
    *lockAddrX = lockX;
    *lockAddrY = lockY;
    *lockAddrZ = lockZ;
}

void lockUpdate(float *toX, float *toY, float *toZ, float* lockAddrX, float* lockAddrY, float* lockAddrZ) {
    *toX = *lockAddrX;
    *toY = *lockAddrY;
    *toZ = *lockAddrZ;
}

double radian(float angle) {
    const double PI = 3.14159265358979323846;
    return angle * PI / 180.;
}

float pytha(float dist1, float dist2) {
    return sqrt((dist1 * dist1) + (dist2 * dist2));
}

Vector3 getAxis(float xr, float yr) { // 1.41x 까지 부스트 가능

    Vector3 toReturn;

    int xrx = (int)xr % 90;
    toReturn.x = FLY_SPEED;
    if (xrx < 45) {
        toReturn.z = tan(radian(xrx)) * FLY_SPEED;
        SWAP(toReturn.z, toReturn.x);
    }
    else toReturn.z = tan(radian(90 - xrx)) * FLY_SPEED;

    if (xr >= 0 && xr < 90) {
        SWAP(toReturn.x, toReturn.z);
        toReturn.x = -toReturn.x;
    }
    if (xr >= 90 && xr < 180) {
        // nop;
    }
    if (xr >= 180 && xr < 270) {
        SWAP(toReturn.x, toReturn.z);
        toReturn.z = -toReturn.z;
    }
    if (xr >= 270 && xr < 360) {
        toReturn.z = -toReturn.z;
        toReturn.x = -toReturn.x;
    }

    float yrA = abs(yr);
    float dist_py = pytha(toReturn.x, toReturn.z);

    toReturn.y = tan(radian(yrA)) * dist_py;

    if (toReturn.y > FLY_SPEED) toReturn.y = FLY_SPEED;

    if (yr <= 0) {
        toReturn.y = -toReturn.y;
    }

    return toReturn;
}

DWORD WINAPI thr_main(HMODULE hModule) {

    // 1. Create Console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    std::cout << "Can you hear me?" << std::endl;
    std::cout << "OK I see. NUM1 = Health Hack, NUM2 = Ammo Hack, NUM3 = Fly Hack." << std::endl;
    std::cout << "Also END = Escape." << std::endl;

    // 2. Get Module Base
    uintptr_t mBase = (uintptr_t)GetModuleHandle(L"ac_client.exe");

    // 3. Loop it
    bool health = false; bool ammo = false; bool fly = false;

    float lockXLoc = 0;
    float lockYLoc = 0;
    float lockZLoc = 0;

    while (true) {

        uintptr_t* playerAddr = (uintptr_t*)(mBase + 0x17B0B8);
        int* healthAddr = (int*)(*playerAddr + 0xEC); // this is health.
        int* ammoAddr = (int*)(*(uintptr_t*)(*(uintptr_t*)(*playerAddr + 0x364) + 0x14));

        float* playerZAddr = (float*)(*playerAddr + 0x28);
        float* playerXAddr = (float*)(*playerAddr + 0x2C);
        float* playerYAddr = (float*)(*playerAddr + 0x30);
        float* playerXRotation = (float*)(*playerAddr + 0x34);
        float* playerYRotation = (float*)(*playerAddr + 0x38);

        Vector3 axis = getAxis(*playerXRotation, *playerYRotation);

        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            health = !health;
            std::cout << "Log... Health cheat: " << health << std::endl;
            if (!health) *healthAddr = 100;
        }
        if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
            ammo = !ammo;
            std::cout << "Log... Ammo cheat: " << ammo << std::endl;
            if (!ammo) *ammoAddr = 20;
        }
        if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
            fly = !fly;
            std::cout << "Log... Fly Cheat: " << fly << std::endl;
            lockUpdate(&lockXLoc, &lockYLoc, &lockZLoc, playerXAddr, playerYAddr, playerZAddr);
        }

        if (fly) { // Lock their location cuz the physics.
            lockLocation(lockXLoc, lockYLoc, lockZLoc, playerXAddr, playerYAddr, playerZAddr);
        }
        
        // 4 fly hack.
        if (GetAsyncKeyState(0x57) & 1 && fly) { // if player presses W key,
            *playerXAddr += axis.x;
            *playerYAddr += axis.y;
            *playerZAddr += axis.z;
            lockUpdate(&lockXLoc, &lockYLoc, &lockZLoc, playerXAddr, playerYAddr, playerZAddr);

        } if (GetAsyncKeyState(0x41) & 1 && fly) { // A
            *playerXAddr -= axis.z;
            *playerYAddr -= axis.y;
            *playerZAddr -= axis.x;
            lockUpdate(&lockXLoc, &lockYLoc, &lockZLoc, playerXAddr, playerYAddr, playerZAddr);

        } if (GetAsyncKeyState(0x53) & 1 && fly) { // S
            *playerXAddr -= axis.x;
            *playerYAddr -= axis.y;
            *playerZAddr -= axis.z;
            lockUpdate(&lockXLoc, &lockYLoc, &lockZLoc, playerXAddr, playerYAddr, playerZAddr);

        } if (GetAsyncKeyState(0x44) & 1 && fly) { // D
            *playerXAddr += axis.z;
            *playerYAddr += axis.y;
            *playerZAddr += axis.x;
            lockUpdate(&lockXLoc, &lockYLoc, &lockZLoc, playerXAddr, playerYAddr, playerZAddr);

        } if (GetAsyncKeyState(VK_SPACE) & 1 && fly) { // if player presses space key,
            *playerYAddr += FLY_SPEED;
            lockUpdate(&lockXLoc, &lockYLoc, &lockZLoc, playerXAddr, playerYAddr, playerZAddr);
        } if (GetAsyncKeyState(VK_SHIFT) & 1 && fly) { // if player presses shift key,
            *playerYAddr -= FLY_SPEED;
            lockUpdate(&lockXLoc, &lockYLoc, &lockZLoc, playerXAddr, playerYAddr, playerZAddr);
        }

        if (GetAsyncKeyState(VK_END) & 1) {
            break;
        }

        if (!playerAddr) continue;

        if (health) {
            *healthAddr = 3170;
        }

        if (ammo) {
            *ammoAddr = 3170;
        }
    }
    

    if (f) fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{

    HANDLE hThread = 0;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)thr_main, hModule, 0, NULL);
        if (hThread) CloseHandle(hThread);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

