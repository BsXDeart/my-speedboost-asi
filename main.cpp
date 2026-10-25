#include <windows.h>

// ฟังก์ชันคำนวณเวกเตอร์ความเร็วสำหรับ GTA SA
void ApplySpeedBoost() {
    uintptr_t pedPtr = *reinterpret_cast<uintptr_t*>(0xB6F5F0);
    if (!pedPtr) return;

    // เช็คว่าผู้เล่นอยู่ในรถหรือไม่ (Offset 0x58C)
    uintptr_t vehiclePtr = *reinterpret_cast<uintptr_t*>(pedPtr + 0x58C);
    if (!vehiclePtr) return;

    // เช็คการกดปุ่ม Alt ซ้าย (VK_LMENU)
    if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
        // ดึง Forward Matrix ทิศทางด้านหน้าของรถ (Offset 0x14)
        float* matrix = *reinterpret_cast<float**>(vehiclePtr + 0x14);
        if (!matrix) return;

        float forwardX = matrix[4];
        float forwardY = matrix[5];
        float forwardZ = matrix[6];

        // ดึงความเร็วปัจจุบันของรถ (Offset 0x44)
        float* speedX = reinterpret_cast<float*>(vehiclePtr + 0x44);
        float* speedY = reinterpret_cast<float*>(vehiclePtr + 0x48);
        float* speedZ = reinterpret_cast<float*>(vehiclePtr + 0x4C);

        // เพิ่มความเร็วพุ่งไปข้างหน้า
        float boostAmount = 0.015f;
        *speedX += forwardX * boostAmount;
        *speedY += forwardY * boostAmount;
        *speedZ += forwardZ * boostAmount;
    }
}

DWORD WINAPI MainThread(LPVOID lpParam) {
    while (true) {
        ApplySpeedBoost();
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
