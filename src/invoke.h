#pragma once
// Minimal native invoker on top of the ScriptHookV SDK exports (nativeInit/nativePush64/nativeCall).
// Calls natives by hash so we don't depend on a particular natives.h naming scheme.
// Every hash below is checked against the SDK's natives.h.

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
    // player
    constexpr UINT64 PLAYER_PED_ID                         = 0xD80958FC74E988A6;
    constexpr UINT64 PLAYER_ID                             = 0x4F8644AF03D0E0D6;
    constexpr UINT64 IS_PLAYER_FREE_AIMING                 = 0x2E397FD2ECD37C87;
    constexpr UINT64 IS_PLAYER_CONTROL_ON                  = 0x49C32D60007AFA47;

    // entity
    constexpr UINT64 DOES_ENTITY_EXIST                     = 0x7239B21A38F536BA;
    constexpr UINT64 GET_ENTITY_COORDS                     = 0x3FEF770D40960D5A;
    constexpr UINT64 GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS = 0x1899F328B0E12848;
    constexpr UINT64 GET_ENTITY_MODEL                      = 0x9F47B058362C84B5;
    constexpr UINT64 GET_ENTITY_HEADING                    = 0xE83D4F9BA2A38914;
    constexpr UINT64 GET_ENTITY_HEALTH                     = 0xEEF059FAD016D209;
    constexpr UINT64 GET_ENTITY_MAX_HEALTH                 = 0x15D757606D170C3C;
    constexpr UINT64 GET_ENTITY_SPEED                      = 0xD5037BA82E12416F; // m/s
    constexpr UINT64 IS_ENTITY_DEAD                        = 0x5F9532F3B5CC2551;
    constexpr UINT64 HAS_ENTITY_CLEAR_LOS_TO_ENTITY        = 0xFCDFF7B72D23A1AC;
    constexpr UINT64 HAS_ENTITY_BEEN_DAMAGED_BY_ENTITY     = 0xC86D67D52A707CF8;
    constexpr UINT64 SET_ENTITY_COORDS_NO_OFFSET           = 0x239A3351AC1DA385;
    constexpr UINT64 GET_MODEL_DIMENSIONS                  = 0x03E8D3D5F549087A;
    constexpr UINT64 GET_HASH_KEY                          = 0xD24D37CC275948CC;
    constexpr UINT64 IS_MODEL_VALID                        = 0xC0296A2EDF545E92;

    // ped
    constexpr UINT64 IS_PED_HUMAN                          = 0xB980061DA992779D;
    constexpr UINT64 GET_PED_TYPE                          = 0xFF059E1E4C01E63C;
    constexpr UINT64 GET_PED_ARMOUR                        = 0x9483AF821605B1D8;
    constexpr UINT64 GET_PED_BONE_COORDS                   = 0x17C07FC640E86B4E;
    constexpr UINT64 IS_PED_IN_COMBAT                      = 0x4859F1FC66A6278E;
    constexpr UINT64 IS_PED_IN_ANY_VEHICLE                 = 0x997ABD671D25CA0B;
    constexpr UINT64 GET_VEHICLE_PED_IS_USING              = 0x6094AD011A2EA87D;
    constexpr UINT64 GET_VEHICLE_PED_IS_IN                 = 0x9A9112A0FE9A4713;
    constexpr UINT64 IS_PED_SHOOTING                       = 0x34616828CD07F1A1;
    constexpr UINT64 IS_PED_IN_MELEE_COMBAT                = 0x4E209B2C1EAD5159;
    constexpr UINT64 GET_PED_SOURCE_OF_DEATH               = 0x93C8B64DEB84728C; // SDK: _GET_PED_KILLER
    constexpr UINT64 CLEAR_PED_TASKS_IMMEDIATELY           = 0xAAA34F8A7CB32098;
    constexpr UINT64 GET_RELATIONSHIP_BETWEEN_PEDS         = 0xEBA5AD3A0EAF7121;
    constexpr UINT64 GET_SELECTED_PED_WEAPON               = 0x0A6DB4965674D243;
    constexpr UINT64 GET_WEAPONTYPE_GROUP                  = 0xC3287EE3050FB74C;
    constexpr UINT64 GIVE_DELAYED_WEAPON_TO_PED            = 0xB282DC6EBD803C75;
    constexpr UINT64 GET_WEAPONTYPE_MODEL                  = 0xF46CDC33180FDA94;

    // vehicle
    constexpr UINT64 GET_DISPLAY_NAME_FROM_VEHICLE_MODEL   = 0xB215AAC32D25D019;
    constexpr UINT64 GET_PED_IN_VEHICLE_SEAT               = 0xBB40DD2270B65366;
    constexpr UINT64 GET_VEHICLE_DOOR_LOCK_STATUS          = 0x25BC98A59C2EA962;
    constexpr UINT64 GET_VEHICLE_CLASS                     = 0x29439776AAA00A62;
    constexpr UINT64 GET_VEHICLE_ESTIMATED_MAX_SPEED       = 0x53AF99BAA671CA47; // SDK: _GET_VEHICLE_SPEED; m/s, includes upgrades

    // camera
    constexpr UINT64 GET_GAMEPLAY_CAM_COORD                = 0x14D6F5678D8F1B37;
    constexpr UINT64 GET_GAMEPLAY_CAM_ROT                  = 0x837765A25378F0BB;
    constexpr UINT64 GET_FINAL_RENDERED_CAM_COORD          = 0xA200EB1EE790F448; // SDK: _GET_GAMEPLAY_CAM_COORDS
    constexpr UINT64 GET_FINAL_RENDERED_CAM_ROT            = 0x5B4E4C817FCC2DFB; // SDK: _GET_GAMEPLAY_CAM_ROT
    constexpr UINT64 GET_GAMEPLAY_CAM_RELATIVE_HEADING     = 0x743607648ADD4587;
    constexpr UINT64 GET_GAMEPLAY_CAM_RELATIVE_PITCH       = 0x3A6867B4845BEDA2;
    constexpr UINT64 SET_GAMEPLAY_CAM_RELATIVE_HEADING     = 0xB4EC2312F4E5B1F1;
    constexpr UINT64 SET_GAMEPLAY_CAM_RELATIVE_PITCH       = 0x6D0858B8EDFD2B7D;
    constexpr UINT64 IS_GAMEPLAY_CAM_RENDERING             = 0x39B5D1B10383F0C8;

    // world / misc
    constexpr UINT64 GET_GROUND_Z_FOR_3D_COORD             = 0xC906A7DAB05C8D2B;
    constexpr UINT64 GET_WATER_HEIGHT                      = 0xF6829842C06AE524;
    constexpr UINT64 GET_GAME_TIMER                        = 0x9CD27B0045628463;
    constexpr UINT64 IS_PAUSE_MENU_ACTIVE                  = 0xB0034A223497FFCB;
    constexpr UINT64 DISABLE_CONTROL_ACTION                = 0xFE99B66D079CF6BC;
    constexpr UINT64 ADD_TO_CLOCK_TIME                     = 0xD716F30D8C8980E2;
    constexpr UINT64 GET_CLOCK_HOURS                       = 0x25223CA6B4D20B7F;
    constexpr UINT64 GET_CLOCK_MINUTES                     = 0x13D2B8ADD79640F2;

    // blips
    constexpr UINT64 GET_BLIP_INFO_ID_ITERATOR             = 0x186E5D252FA50E7D;
    constexpr UINT64 GET_FIRST_BLIP_INFO_ID                = 0x1BEDE233E6CD2A1F;
    constexpr UINT64 GET_NEXT_BLIP_INFO_ID                 = 0x14F96AA50D6FBEA7;
    constexpr UINT64 GET_BLIP_INFO_ID_TYPE                 = 0xBE9B0959FFD0779B;
    constexpr UINT64 GET_BLIP_INFO_ID_COORD                = 0xFA7C7F0AADF25D09;
    constexpr UINT64 DOES_BLIP_EXIST                       = 0xA6DB27D19ECBB7DA;

    // drawing
    constexpr UINT64 GET_SCREEN_COORD_FROM_WORLD_COORD     = 0x34E82F05DF2974F5;
    constexpr UINT64 GET_ASPECT_RATIO                      = 0xF1307EF624A80D87;
    constexpr UINT64 DRAW_RECT                             = 0x3A618A217E5154F0;
    constexpr UINT64 DRAW_LINE                             = 0x6B7256074AE34680;
    constexpr UINT64 DRAW_SPRITE                           = 0xE7FFAE5EBF23D890;
    constexpr UINT64 REQUEST_STREAMED_TEXTURE_DICT         = 0xDFA2EF8E04127DD5;
    constexpr UINT64 HAS_STREAMED_TEXTURE_DICT_LOADED      = 0x0145F696AAAAD2E4;

    // text
    constexpr UINT64 GET_FILENAME_FOR_AUDIO_CONVERSATION   = 0x7B5280EBA9840C72; // a.k.a. GET_LABEL_TEXT
    constexpr UINT64 SET_TEXT_FONT                         = 0x66E0276CC5F6B9DA;
    constexpr UINT64 SET_TEXT_SCALE                        = 0x07C837F9A01C34C9;
    constexpr UINT64 SET_TEXT_COLOUR                       = 0xBE6B23FFA53FB442;
    constexpr UINT64 SET_TEXT_CENTRE                       = 0xC02F4DBFB51D988B;
    constexpr UINT64 SET_TEXT_OUTLINE                      = 0x2513DFB0FB8400FE;
    constexpr UINT64 BEGIN_TEXT_COMMAND_DISPLAY_TEXT       = 0x25FBB336DF1804CB;
    constexpr UINT64 ADD_TEXT_COMPONENT_SUBSTRING_PLAYER_NAME = 0x6C188BE134E074AA;
    constexpr UINT64 END_TEXT_COMMAND_DISPLAY_TEXT         = 0xCD015E5BB0D96A57;
    constexpr UINT64 BEGIN_TEXT_COMMAND_GET_WIDTH          = 0x54CE8AC98E120CAB; // SDK: _SET_TEXT_ENTRY_FOR_WIDTH
    constexpr UINT64 END_TEXT_COMMAND_GET_WIDTH            = 0x85F061DA64ED2F67; // SDK: _GET_TEXT_SCREEN_WIDTH
    constexpr UINT64 SET_NOTIFICATION_TEXT_ENTRY           = 0x202709F4C58A0424;
    constexpr UINT64 DRAW_NOTIFICATION                     = 0x2ED7843F8F801023;
}
