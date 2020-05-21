// CMake generated file. Do Not Edit.

#pragma once

namespace pangolin {

void RegisterTestVideoFactory();
void RegisterImagesVideoFactory();
void RegisterImagesVideoOutputFactory();
void RegisterSplitVideoFactory();
void RegisterPvnVideoFactory();
void RegisterPangoVideoFactory();
void RegisterPangoVideoOutputFactory();
void RegisterDebayerVideoFactory();
void RegisterShiftVideoFactory();
void RegisterMirrorVideoFactory();
void RegisterUnpackVideoFactory();
void RegisterJoinVideoFactory();
void RegisterMergeVideoFactory();
void RegisterJsonVideoFactory();
void RegisterThreadVideoFactory();
void RegisterUvcMediaFoundationVideoFactory();

inline bool LoadBuiltInVideoDrivers()
{
    RegisterTestVideoFactory();
    RegisterImagesVideoFactory();
    RegisterImagesVideoOutputFactory();
    RegisterSplitVideoFactory();
    RegisterPvnVideoFactory();
    RegisterPangoVideoFactory();
    RegisterPangoVideoOutputFactory();
    RegisterDebayerVideoFactory();
    RegisterShiftVideoFactory();
    RegisterMirrorVideoFactory();
    RegisterUnpackVideoFactory();
    RegisterJoinVideoFactory();
    RegisterMergeVideoFactory();
    RegisterJsonVideoFactory();
    RegisterThreadVideoFactory();
    RegisterUvcMediaFoundationVideoFactory();
    return true;
}

} // pangolin
