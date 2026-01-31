# libstroopwafel

**This library is still WIP, may not work as expected or have breaking changes in the near future**

# libstroopwafel
This library provides a convenient wrapper for interacting with the `stroopwafel` custom firmware's IPC functionalities. It requires `stroopwafel` to be running.  
Requires [wut](https://github.com/devkitPro/wut) for building.
Install via `make install`.

## Usage
Make sure to add `-lstroopwafel` to `LIBS` and `$(WUT_ROOT)/usr` to `LIBDIRS` in your makefile.

After that you can simply include `<stroopwafel/stroopwafel.h>` to get access to the `stroopwafel` functions after calling `Stroopwafel_InitLibrary()`.

Available functions:
- `Stroopwafel_GetStatusStr(StroopwafelStatus status)`: Returns a string representation of the status.
- `Stroopwafel_InitLibrary()`: Initializes the library.
- `Stroopwafel_DeInitLibrary()`: Deinitializes the library.
- `Stroopwafel_GetAPIVersion(uint32_t *outVersion)`: Retrieves the API version of the running stroopwafel.
- `Stroopwafel_SetFwPath(const char* path)`: Sets the firmware image path.
- `Stroopwafel_WriteMemory(uint32_t num_writes, const StroopwafelWrite *writes)`: Writes data to the IOS memory.
- `Stroopwafel_Execute(uint32_t target_addr, const void *config, uint32_t config_len, void *output, uint32_t output_len)`: Executes code at a target address in IOS.
- `Stroopwafel_MapMemory(const StroopwafelMapMemory *info)`: Maps memory pages in IOS.
