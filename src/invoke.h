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
    constexpr UINT64 GET_VEHICLE_CLASS_FROM_NAME           = 0xDEDF1C8BD47C2200;
    constexpr UINT64 GET_VEHICLE_ESTIMATED_MAX_SPEED       = 0x53AF99BAA671CA47; // SDK: _GET_VEHICLE_SPEED; m/s, includes upgrades

    // garage: vehicle setup (read from a saved car, applied to the one we spawn)
    constexpr UINT64 SET_VEHICLE_MOD_KIT                   = 0x1F2AA07F00B3217A;
    constexpr UINT64 GET_VEHICLE_MOD                       = 0x772960298DA26FDB;
    constexpr UINT64 GET_VEHICLE_MOD_VARIATION             = 0xB3924ECD70E095DC;
    constexpr UINT64 SET_VEHICLE_MOD                       = 0x6AF0636DDEDCB6DD;
    constexpr UINT64 IS_TOGGLE_MOD_ON                      = 0x84B233A8C8FC8AE7;
    constexpr UINT64 TOGGLE_VEHICLE_MOD                    = 0x2A1F4F37F95BAD08;
    constexpr UINT64 GET_VEHICLE_COLOURS                   = 0xA19435F193E081AC;
    constexpr UINT64 SET_VEHICLE_COLOURS                   = 0x4F1D4BE3A7F24601;
    constexpr UINT64 GET_VEHICLE_EXTRA_COLOURS             = 0x3BC4245933A166F7;
    constexpr UINT64 SET_VEHICLE_EXTRA_COLOURS             = 0x2036F561ADD12E33;
    constexpr UINT64 GET_VEHICLE_CUSTOM_PRIMARY_COLOUR     = 0xB64CF2CCA9D95F52;
    constexpr UINT64 SET_VEHICLE_CUSTOM_PRIMARY_COLOUR     = 0x7141766F91D15BEA;
    constexpr UINT64 GET_VEHICLE_CUSTOM_SECONDARY_COLOUR   = 0x8389CD56CA8072DC;
    constexpr UINT64 SET_VEHICLE_CUSTOM_SECONDARY_COLOUR   = 0x36CED73BFED89754;
    constexpr UINT64 GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM  = 0xF095C0405307B21B;
    constexpr UINT64 GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM = 0x910A32E7AAD2656C;
    constexpr UINT64 GET_VEHICLE_MOD_COLOR_1               = 0xE8D65CA700C9A693;
    constexpr UINT64 SET_VEHICLE_MOD_COLOR_1               = 0x43FEB945EE7F85B8;
    constexpr UINT64 GET_VEHICLE_MOD_COLOR_2               = 0x81592BE4E3878728;
    constexpr UINT64 SET_VEHICLE_MOD_COLOR_2               = 0x816562BADFDEC83E;
    constexpr UINT64 GET_VEHICLE_COLOUR_COMBINATION        = 0x6A842D197F845D56;
    constexpr UINT64 SET_VEHICLE_COLOUR_COMBINATION        = 0x33E8CD3322E2FE31;
    constexpr UINT64 GET_VEHICLE_WHEEL_TYPE                = 0xB3ED1BFB4BE636DC;
    constexpr UINT64 SET_VEHICLE_WHEEL_TYPE                = 0x487EB21CC7295BA1;
    constexpr UINT64 DOES_EXTRA_EXIST                      = 0x1262D55792428154;
    constexpr UINT64 IS_VEHICLE_EXTRA_TURNED_ON            = 0xD2E6822DBFD6C8BD;
    constexpr UINT64 SET_VEHICLE_EXTRA                     = 0x7EE3A3C5E4A40CC9; // toggle: 0 = on, 1 = off
    constexpr UINT64 GET_VEHICLE_LIVERY                    = 0x2BB9230590DA5E8A;
    constexpr UINT64 SET_VEHICLE_LIVERY                    = 0x60BF608F1B8CD1B6;
    constexpr UINT64 GET_VEHICLE_WINDOW_TINT               = 0x0EE21293DAD47C95;
    constexpr UINT64 SET_VEHICLE_WINDOW_TINT               = 0x57C51E6BAD752696;
    constexpr UINT64 IS_VEHICLE_NEON_LIGHT_ENABLED         = 0x8C4B92553E4766A5; // SDK: _IS_VEHICLE_NEON_LIGHT_ENABLED
    constexpr UINT64 SET_VEHICLE_NEON_LIGHT_ENABLED        = 0x2AA720E4287BF269; // SDK: _SET_VEHICLE_NEON_LIGHT_ENABLED
    constexpr UINT64 GET_VEHICLE_NEON_LIGHTS_COLOUR        = 0x7619EEE8C886757F; // SDK: _GET_VEHICLE_NEON_LIGHTS_COLOUR
    constexpr UINT64 SET_VEHICLE_NEON_LIGHTS_COLOUR        = 0x8E0A582209A62695; // SDK: _SET_VEHICLE_NEON_LIGHTS_COLOUR
    constexpr UINT64 GET_VEHICLE_TYRE_SMOKE_COLOR          = 0xB635392A4938B3C3;
    constexpr UINT64 SET_VEHICLE_TYRE_SMOKE_COLOR          = 0xB5BA80F839791C0F;
    constexpr UINT64 GET_VEHICLE_NUMBER_PLATE_TEXT         = 0x7CE1CCB9B293020E;
    constexpr UINT64 SET_VEHICLE_NUMBER_PLATE_TEXT         = 0x95A88F0B409CDA47;
    constexpr UINT64 GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX   = 0xF11BC2DD9A3E7195;
    constexpr UINT64 SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX   = 0x9088EB5A43FFB0A1;
    constexpr UINT64 GET_VEHICLE_TYRES_CAN_BURST           = 0x678B9BB8C3F58FEB;
    constexpr UINT64 SET_VEHICLE_TYRES_CAN_BURST           = 0xEB9DC3C7D8596C46;
    constexpr UINT64 IS_THIS_MODEL_A_CAR                   = 0x7F6DB52EEFC96DF8;
    constexpr UINT64 IS_THIS_MODEL_A_BIKE                  = 0xB50C0B0CEDC6CE84;
    constexpr UINT64 IS_THIS_MODEL_A_QUADBIKE              = 0x39DAC362EE65FA28;
    constexpr UINT64 IS_THIS_MODEL_A_BICYCLE               = 0xBF94DD42F63BDED2;
    constexpr UINT64 IS_MODEL_IN_CDIMAGE                   = 0x35B9E0803292B641;
    constexpr UINT64 IS_MODEL_A_VEHICLE                    = 0x19AAC8F07BFEC53E;

    // garage: calling a saved car
    constexpr UINT64 REQUEST_MODEL                         = 0x963D27A58DF860AC;
    constexpr UINT64 HAS_MODEL_LOADED                      = 0x98A4EB5D89A0C952;
    constexpr UINT64 SET_MODEL_AS_NO_LONGER_NEEDED         = 0xE532F5D78798DAAB;
    constexpr UINT64 CREATE_VEHICLE                        = 0xAF35D0D2583051B0;
    constexpr UINT64 SET_VEHICLE_ON_GROUND_PROPERLY        = 0x49733E92263139D1;
    constexpr UINT64 GET_NTH_CLOSEST_VEHICLE_NODE_WITH_HEADING = 0x80CA6A8B6C094CC4;
    constexpr UINT64 IS_SPHERE_VISIBLE                     = 0xE33D59DA70B58FDF;
    constexpr UINT64 IS_ENTITY_ON_SCREEN                   = 0xE659E47AF827484B;
    constexpr UINT64 SET_ENTITY_AS_MISSION_ENTITY          = 0xAD738C3085FE7E11;
    constexpr UINT64 SET_VEHICLE_AS_NO_LONGER_NEEDED       = 0x629BFA74418D6239;
    constexpr UINT64 DELETE_VEHICLE                        = 0xEA386986E786A54F;
    constexpr UINT64 SET_VEHICLE_DOORS_LOCKED              = 0xB664292EAECF7FA6; // 1 = unlocked
    constexpr UINT64 SET_VEHICLE_ENGINE_ON                 = 0x2497C4717C8B881E;
    constexpr UINT64 SET_VEHICLE_DIRT_LEVEL                = 0x79D3B596FE44EE8B;
    constexpr UINT64 SET_VEHICLE_HAS_BEEN_OWNED_BY_PLAYER  = 0x2B5F9D2AF1F1722D;
    constexpr UINT64 SET_VEHICLE_NEEDS_TO_BE_HOTWIRED      = 0xFBA550EA44404EE6;
    constexpr UINT64 SET_VEHICLE_IS_STOLEN                 = 0x67B2C79AA7FF5738;
    constexpr UINT64 IS_VEHICLE_DRIVEABLE                  = 0x4C241E39B23DF959;
    constexpr UINT64 IS_PED_IN_VEHICLE                     = 0xA3EE4A07279BB9DB;
    constexpr UINT64 IS_VEHICLE_SEAT_FREE                  = 0x22AC59A870E6A669;
    constexpr UINT64 GET_VEHICLE_NUMBER_OF_PASSENGERS      = 0x24CB2137731FFE89;
    constexpr UINT64 IS_ANY_VEHICLE_NEAR_POINT             = 0x61E1DD6125A3EEE6;
    constexpr UINT64 IS_ENTITY_IN_WATER                    = 0xCFB0A0D8EDD145A3;

    // garage: car photos (scripted camera)
    constexpr UINT64 CREATE_CAM                            = 0xC3981DCE61D9E13F;
    constexpr UINT64 SET_CAM_COORD                         = 0x4D41783FB745E42E;
    constexpr UINT64 SET_CAM_FOV                           = 0xB13C14F66A00D047;
    constexpr UINT64 POINT_CAM_AT_ENTITY                   = 0x5640BFF86B16E8DC;
    constexpr UINT64 SET_CAM_ACTIVE                        = 0x026FB97D0A425F84;
    constexpr UINT64 RENDER_SCRIPT_CAMS                    = 0x07E5B515DB0636FC;
    constexpr UINT64 DESTROY_CAM                           = 0x865908C81A2C22E9;
    constexpr UINT64 HIDE_HUD_AND_RADAR_THIS_FRAME         = 0x719FF505F097FD20;

    // garage: plate editor (the game's on-screen keyboard)
    constexpr UINT64 DISPLAY_ONSCREEN_KEYBOARD             = 0x00DC833F2568DBF6;
    constexpr UINT64 UPDATE_ONSCREEN_KEYBOARD              = 0x0CF2B696BBF945AE; // 0 editing, 1 done, 2 cancelled, 3 inactive
    constexpr UINT64 GET_ONSCREEN_KEYBOARD_RESULT          = 0x8362B09B91893647;

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
    constexpr UINT64 ADD_BLIP_FOR_ENTITY                   = 0x5CDE92C702A8FCE7;
    constexpr UINT64 SET_BLIP_SPRITE                       = 0xDF735600A4696DAF;
    constexpr UINT64 SET_BLIP_COLOUR                       = 0x03D7FB09E75D6B7E;
    constexpr UINT64 SET_BLIP_DISPLAY                      = 0x9029B2F3DA924928; // 0 = hidden, 2 = map + radar
    constexpr UINT64 REMOVE_BLIP                           = 0x86A652570E5F25DD;
    constexpr UINT64 BEGIN_TEXT_COMMAND_SET_BLIP_NAME      = 0xF9113A30DE5C6670;
    constexpr UINT64 END_TEXT_COMMAND_SET_BLIP_NAME        = 0xBC38B49BCB83BC9B;

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
