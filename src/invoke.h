#pragma once
// Minimal native invoker on top of the ScriptHookV SDK exports (nativeInit/nativePush64/nativeCall).
// Calls natives by hash so we don't depend on a particular natives.h naming scheme.

#include <windows.h>
#include <cstring>
#include <type_traits>
#include "types.h"
#include "main.h"

// Script vectors are 3 floats padded to 8 bytes each.
struct Vec3 {
    float x; DWORD _px;
    float y; DWORD _py;
    float z; DWORD _pz;
};

template <typename T>
inline void PushArg(T v) {
    static_assert(sizeof(T) <= sizeof(UINT64), "native arg too large");
    UINT64 raw = 0;
    std::memcpy(&raw, &v, sizeof(T));
    nativePush64(raw);
}

template <typename R = void, typename... Args>
inline R Invoke(UINT64 hash, Args... args) {
    nativeInit(hash);
    (PushArg(args), ...);
    if constexpr (std::is_void_v<R>) {
        nativeCall();
    } else {
        return *reinterpret_cast<R*>(nativeCall());
    }
}

namespace N {
    constexpr UINT64 PLAYER_PED_ID                         = 0xD80958FC74E988A6;
    constexpr UINT64 GET_ENTITY_COORDS                     = 0x3FEF770D40960D5A;
    constexpr UINT64 GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS = 0x1899F328B0E12848;
    constexpr UINT64 GET_ENTITY_MODEL                      = 0x9F47B058362C84B5;
    constexpr UINT64 GET_ENTITY_HEALTH                     = 0xEEF059FAD016D209;
    constexpr UINT64 GET_ENTITY_MAX_HEALTH                 = 0x15D757606D170C3C;
    constexpr UINT64 IS_ENTITY_DEAD                        = 0x5F9532F3B5CC2551;
    constexpr UINT64 GET_MODEL_DIMENSIONS                  = 0x03E8D3D5F549087A;
    constexpr UINT64 GET_PED_TYPE                          = 0xFF059E1E4C01E63C;
    constexpr UINT64 IS_PED_IN_COMBAT                      = 0x4859F1FC66A6278E;
    constexpr UINT64 GET_RELATIONSHIP_BETWEEN_PEDS         = 0xEBA5AD3A0EAF7121;
    constexpr UINT64 GET_DISPLAY_NAME_FROM_VEHICLE_MODEL   = 0xB215AAC32D25D019;
    constexpr UINT64 GET_FILENAME_FOR_AUDIO_CONVERSATION   = 0x7B5280EBA9840C72; // a.k.a. GET_LABEL_TEXT
    constexpr UINT64 GET_SCREEN_COORD_FROM_WORLD_COORD     = 0x34E82F05DF2974F5;
    constexpr UINT64 GET_ASPECT_RATIO                      = 0xF1307EF624A80D87;
    constexpr UINT64 DRAW_RECT                             = 0x3A618A217E5154F0;
    constexpr UINT64 DRAW_LINE                             = 0x6B7256074AE34680;
    constexpr UINT64 SET_TEXT_FONT                         = 0x66E0276CC5F6B9DA;
    constexpr UINT64 SET_TEXT_SCALE                        = 0x07C837F9A01C34C9;
    constexpr UINT64 SET_TEXT_COLOUR                       = 0xBE6B23FFBBD77D82;
    constexpr UINT64 SET_TEXT_CENTRE                       = 0xC02F4DBFB51D988B;
    constexpr UINT64 SET_TEXT_OUTLINE                      = 0x2513DFB0FB8400FE;
    constexpr UINT64 BEGIN_TEXT_COMMAND_DISPLAY_TEXT       = 0x25FBB336DF1804CB;
    constexpr UINT64 ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME = 0x6C188BE134E074AA;
    constexpr UINT64 END_TEXT_COMMAND_DISPLAY_TEXT         = 0xCD015E5BB0D96A57;
}
