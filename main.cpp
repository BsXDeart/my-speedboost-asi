#include <windows.h>

// ตัวแปรเก็บสถานะ เปิด/ปิด ESP Name
bool g_bESPEnabled = false;
bool g_bKeyHandled = false;

// ฟังก์ชันคำนวณเวกเตอร์ความเร็วสำหรับ GTA SA (Speed Boost)
void ApplySpeedBoost() {
    uintptr_t pedPtr = *reinterpret_cast<uintptr_t*>(0xB6F5F0);
    if (!pedPtr) return;

    uintptr_t vehiclePtr = *reinterpret_cast<uintptr_t*>(pedPtr + 0x58C);
    if (!vehiclePtr) return;

    if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
        float* matrix = *reinterpret_cast<float**>(vehiclePtr + 0x14);
        if (!matrix) return;

        float forwardX = matrix[4];
        float forwardY = matrix[5];
        float forwardZ = matrix[6];

        float* speedX = reinterpret_cast<float*>(vehiclePtr + 0x44);
        float* speedY = reinterpret_cast<float*>(vehiclePtr + 0x48);
        float* speedZ = reinterpret_cast<float*>(vehiclePtr + 0x4C);

        float boostAmount = 0.015f;
        *speedX += forwardX * boostAmount;
        *speedY += forwardY * boostAmount;
        *speedZ += forwardZ * boostAmount;
    }
}

// ฟังก์ชันควบคุมการเรนเดอร์ ESP Name (เปิด/ปิด Tag ชื่อผู้เล่น)
void ProcessESPName() {
    // เช็คการกด Ctrl + L
    if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) && (GetAsyncKeyState('L') & 0x8000)) {
        if (!g_bKeyHandled) {
            g_bESPEnabled = !g_bESPEnabled; // สลับสถานะ เปิด/ปิด
            g_bKeyHandled = true;
        }
    } else {
        g_bKeyHandled = false;
    }

    // เขียน Memory บังคับให้ SAMP วาด NameTag ทะลุกำแพง / มองเห็นตลอดเวลา
    // (SAMP NameTag Draw Flag Memory Address: 0x609CD)
    DWORD dwOldProtect;
    VirtualProtect(reinterpret_cast<LPVOID>(0x609CD), 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
    
    if (g_bESPEnabled) {
        // NOP ให้เห็นชื่อ/ID/สีชื่อตลอดเวลา
        *reinterpret_cast<BYTE*>(0x609CD) = 0x90;
        *reinterpret_cast<BYTE*>(0x609CE) = 0x90;
    } else {
        // คืนค่าเดิมของ SAMP
        *reinterpret_cast<BYTE*>(0x609CD) = 0x75;
        *reinterpret_cast<BYTE*>(0x609CE) = 0x36;
    }
    
    VirtualProtect(reinterpret_cast<LPVOID>(0x609CD), 4, dwOldProtect, &dwOldProtect);
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    while (true) {
        ApplySpeedBoost();
        ProcessESPName();
        Sleep(10);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(NULL, 0, MainThread, NULL, 0, NULL);
    }
    return TRUE;
}
