#include "stroopwafel_ipc.h"
#include "logger.h"
#include "stroopwafel/commands.h"
#include "stroopwafel/stroopwafel.h"
#include <coreinit/ios.h>
#include <cstring>
#include <malloc.h>
#include <stdint.h>

int stroopwafelHandle = -1;
int stroopwafelInitDone = 0;

namespace {
    // Helper to send simple IOCTL commands with one input and one output buffer.
    // This is similar to doSimpleCustomIPCCommand from original libmocha,
    // but adapted for /dev/stroopwafel and general IOCTL commands.
    StroopwafelStatus doStroopwafelIPC(const uint32_t command, const void *buffer_in, uint32_t length_in, void *buffer_io, uint32_t length_io, int *actual_len) {
        if (!stroopwafelInitDone || stroopwafelHandle < 0) {
            return STROOPWAFEL_RESULT_LIB_UNINITIALIZED;
        }

        int res = IOS_Ioctl(stroopwafelHandle, command, (void*)buffer_in, length_in, buffer_io, length_io);
        if (res < 0) {
            DEBUG_FUNCTION_LINE_ERR("IOS_Ioctl failed with res: %d", res);
            return STROOPWAFEL_RESULT_UNKNOWN_ERROR;
        }
        if (actual_len) {
            *actual_len = res;
        }
        return STROOPWAFEL_RESULT_SUCCESS;
    }
} // namespace

const char *Stroopwafel_GetStatusStr(StroopwafelStatus status) {
    switch (status) {
        case STROOPWAFEL_RESULT_SUCCESS:
            return "STROOPWAFEL_RESULT_SUCCESS";
        case STROOPWAFEL_RESULT_INVALID_ARGUMENT:
            return "STROOPWAFEL_RESULT_INVALID_ARGUMENT";
        case STROOPWAFEL_RESULT_MAX_CLIENT:
            return "STROOPWAFEL_RESULT_MAX_CLIENT";
        case STROOPWAFEL_RESULT_OUT_OF_MEMORY:
            return "STROOPWAFEL_RESULT_OUT_OF_MEMORY";
        case STROOPWAFEL_RESULT_ALREADY_EXISTS:
            return "STROOPWAFEL_RESULT_ALREADY_EXISTS";
        case STROOPWAFEL_RESULT_ADD_DEVOPTAB_FAILED:
            return "STROOPWAFEL_RESULT_ADD_DEVOPTAB_FAILED";
        case STROOPWAFEL_RESULT_NOT_FOUND:
            return "STROOPWAFEL_RESULT_NOT_FOUND";
        case STROOPWAFEL_RESULT_UNSUPPORTED_API_VERSION:
            return "STROOPWAFEL_RESULT_UNSUPPORTED_API_VERSION";
        case STROOPWAFEL_RESULT_UNSUPPORTED_COMMAND:
            return "STROOPWAFEL_RESULT_UNSUPPORTED_COMMAND";
        case STROOPWAFEL_RESULT_UNSUPPORTED_CFW:
            return "STROOPWAFEL_RESULT_UNSUPPORTED_CFW";
        case STROOPWAFEL_RESULT_LIB_UNINITIALIZED:
            return "STROOPWAFEL_RESULT_LIB_UNINITIALIZED";
        case STROOPWAFEL_RESULT_UNKNOWN_ERROR:
            return "STROOPWAFEL_RESULT_UNKNOWN_ERROR";
    }
    return "STROOPWAFEL_RESULT_UNKNOWN_ERROR";
}


StroopwafelStatus Stroopwafel_InitLibrary() {
    if (stroopwafelInitDone) {
        return STROOPWAFEL_RESULT_SUCCESS;
    }

    if (stroopwafelHandle < 0) {
        int handle = IOS_Open((char *) ("/dev/stroopwafel"), static_cast<IOSOpenMode>(0));
        if (handle < 0) {
            DEBUG_FUNCTION_LINE_ERR("Failed to open /dev/stroopwafel: %d", handle);
            return STROOPWAFEL_RESULT_UNSUPPORTED_CFW;
        }
        stroopwafelHandle = handle;
    }

    stroopwafelInitDone = 1;
    
    // Check API version for compatibility
    uint32_t version = 0;
    StroopwafelStatus status = Stroopwafel_GetAPIVersion(&version);
    if (status != STROOPWAFEL_RESULT_SUCCESS) {
        IOS_Close(stroopwafelHandle);
        stroopwafelHandle = -1;
        return status;
    }

    if (version > STROOPWAFEL_API_VERSION || version>>24 != STROOPWAFEL_API_VERSION>>24) {
        IOS_Close(stroopwafelHandle);
        stroopwafelHandle = -1;
        DEBUG_FUNCTION_LINE_ERR("Unsupported API Version: 0x%08X, expected 0x%08X", version, STROOPWAFEL_API_VERSION);
        return STROOPWAFEL_RESULT_UNSUPPORTED_API_VERSION;
    }


    return STROOPWAFEL_RESULT_SUCCESS;
}

StroopwafelStatus Stroopwafel_DeInitLibrary() {
    stroopwafelInitDone = 0;

    if (stroopwafelHandle >= 0) {
        IOS_Close(stroopwafelHandle);
        stroopwafelHandle = -1;
    }

    return STROOPWAFEL_RESULT_SUCCESS;
}

StroopwafelStatus Stroopwafel_GetAPIVersion(uint32_t *version) {
    if (!version) {
        return STROOPWAFEL_RESULT_INVALID_ARGUMENT;
    }
    
    ALIGN_0x40 uint32_t io_buffer[0x4]; // Buffer for output, ensure 0x40 aligned
    int actual_len = 0;
    StroopwafelStatus status = doStroopwafelIPC(STROOPWAFEL_IOCTL_GET_API_VERSION, nullptr, 0, io_buffer, sizeof(uint32_t), &actual_len);

    if (status == STROOPWAFEL_RESULT_SUCCESS && actual_len == sizeof(uint32_t)) {
        *version = io_buffer[0];
        return STROOPWAFEL_RESULT_SUCCESS;
    }

    return status;
}

StroopwafelStatus Stroopwafel_SetFwPath(const char* path) {
    if (!path) {
        return STROOPWAFEL_RESULT_INVALID_ARGUMENT;
    }

    // Ensure path fits within the buffer, including null terminator
    size_t path_len = strlen(path);
    if (path_len >= 256) { // Max path length is 255 + null terminator
        return STROOPWAFEL_RESULT_INVALID_ARGUMENT;
    }

    // Create a 0x40 aligned buffer for the path
    ALIGN_0x40 char aligned_path[256];
    strncpy(aligned_path, path, sizeof(aligned_path) - 1);
    aligned_path[sizeof(aligned_path) - 1] = '\0';

    return doStroopwafelIPC(STROOPWAFEL_IOCTL_SET_FW_PATH, aligned_path, strlen(aligned_path) + 1, nullptr, 0, nullptr);
}
