#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <algorithm> // for std::clamp

namespace sdk {

// Forward declarations
struct CBaseEntity;
struct CBaseWeapon;
struct CBaseViewModel;
struct CCSPlayerPawn;
struct CCSPlayerController;

// Vector math
struct Vector2D {
    float x, y;
    Vector2D() : x(0), y(0) {}
    Vector2D(float x_, float y_) : x(x_), y(y_) {}
    Vector2D operator+(const Vector2D& o) const { return {x + o.x, y + o.y}; }
    Vector2D operator-(const Vector2D& o) const { return {x - o.x, y - o.y}; }
    Vector2D operator*(float s) const { return {x * s, y * s}; }
    Vector2D operator/(float s) const { return {x / s, y / s}; }
    float Length() const { return sqrtf(x * x + y * y); }
    Vector2D Normalized() const { float l = Length(); return l > 0 ? *this / l : Vector2D{}; }
    
    // ImGui conversion
    operator ImVec2() const { return ImVec2(x, y); }
    ImVec2 ToImVec2() const { return ImVec2(x, y); }
};

struct Vector3D {
    float x, y, z;
    Vector3D() : x(0), y(0), z(0) {}
    Vector3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    Vector3D operator+(const Vector3D& o) const { return {x + o.x, y + o.y, z + o.z}; }
    Vector3D operator-(const Vector3D& o) const { return {x - o.x, y - o.y, z - o.z}; }
    Vector3D operator*(float s) const { return {x * s, y * s, z * s}; }
    Vector3D operator/(float s) const { return {x / s, y / s, z / s}; }
    
    float Dot(const Vector3D& o) const { return x * o.x + y * o.y + z * o.z; }
    Vector3D Cross(const Vector3D& o) const { return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x}; }
    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float Length2D() const { return sqrtf(x * x + y * y); }
    Vector3D Normalized() const { float l = Length(); return l > 0 ? *this / l : Vector3D{}; }
    float DistTo(const Vector3D& o) const { return (*this - o).Length(); }
    
    // ImGui conversion
    operator ImVec2() const { return ImVec2(x, y); }
    ImVec2 ToImVec2() const { return ImVec2(x, y); }
};

struct Vector4D {
    float x, y, z, w;
};

struct QAngle {
    float pitch, yaw, roll;
    QAngle() : pitch(0), yaw(0), roll(0) {}
    QAngle(float p, float y, float r) : pitch(p), yaw(y), roll(r) {}
    QAngle operator+(const QAngle& o) const { return {pitch + o.pitch, yaw + o.yaw, roll + o.roll}; }
    QAngle operator-(const QAngle& o) const { return {pitch - o.pitch, yaw - o.yaw, roll - o.roll}; }
    QAngle operator*(float s) const { return {pitch * s, yaw * s, roll * s}; }
    void Normalize() {
        while (pitch > 89.0f) pitch -= 180.0f;
        while (pitch < -89.0f) pitch += 180.0f;
        while (yaw > 180.0f) yaw -= 360.0f;
        while (yaw < -180.0f) yaw += 360.0f;
        roll = 0.0f;
    }
    void Clamp() {
        pitch = std::clamp(pitch, -89.0f, 89.0f);
        yaw = std::fmod(yaw + 180.0f, 360.0f) - 180.0f;
        roll = 0.0f;
    }
};

struct Matrix3x4 {
    float m[3][4];
    Vector3D GetOrigin() const { return {m[0][3], m[1][3], m[2][3]}; }
    void SetOrigin(const Vector3D& pos) { m[0][3] = pos.x; m[1][3] = pos.y; m[2][3] = pos.z; }
};

struct Matrix4x4 {
    float m[4][4];
    Vector3D GetOrigin() const { return {m[0][3], m[1][3], m[2][3]}; }
};

inline QAngle VectorAngles(const Vector3D& forward) {
    QAngle angles;
    if (forward.x == 0.0f && forward.y == 0.0f) {
        angles.pitch = forward.z > 0 ? -90.0f : 90.0f;
        angles.yaw = 0.0f;
    } else {
        angles.pitch = atan2f(-forward.z, forward.Length2D()) * 180.0f / 3.14159265f;
        angles.yaw = atan2f(forward.y, forward.x) * 180.0f / 3.14159265f;
    }
    angles.roll = 0.0f;
    return angles;
}

inline Vector3D AngleVectors(const QAngle& angles) {
    float sp = sinf(angles.pitch * 3.14159265f / 180.0f);
    float cp = cosf(angles.pitch * 3.14159265f / 180.0f);
    float sy = sinf(angles.yaw * 3.14159265f / 180.0f);
    float cy = cosf(angles.yaw * 3.14159265f / 180.0f);
    return {cp * cy, cp * sy, -sp};
}

// Game enums
enum class HitGroup : int {
    Generic = 0,
    Head = 1,
    Chest = 2,
    Stomach = 3,
    LeftArm = 4,
    RightArm = 5,
    LeftLeg = 6,
    RightLeg = 7,
    Neck = 8,
    Gear = 10
};

enum class ItemDefinitionIndex : short {
    None = 0,
    Weapon_Deagle = 1,
    Weapon_Elite = 2,
    Weapon_FiveSeven = 3,
    Weapon_Glock = 4,
    Weapon_AK47 = 7,
    Weapon_AUG = 8,
    Weapon_AWP = 9,
    Weapon_FAMAS = 10,
    Weapon_G3SG1 = 11,
    Weapon_GalilAR = 13,
    Weapon_M249 = 14,
    Weapon_M4A1 = 16,
    Weapon_MAC10 = 17,
    Weapon_P90 = 19,
    Weapon_MP5SD = 23,
    Weapon_UMP45 = 24,
    Weapon_XM1014 = 25,
    Weapon_Bizon = 26,
    Weapon_Mag7 = 27,
    Weapon_Negev = 28,
    Weapon_Sawedoff = 29,
    Weapon_Tec9 = 30,
    Weapon_Taser = 31,
    Weapon_HKP2000 = 32,
    Weapon_MP7 = 33,
    Weapon_MP9 = 34,
    Weapon_Nova = 35,
    Weapon_P250 = 36,
    Weapon_SCAR20 = 38,
    Weapon_SG556 = 39,
    Weapon_SSG08 = 40,
    Weapon_Knife = 42,
    Weapon_Flashbang = 43,
    Weapon_HEGrenade = 44,
    Weapon_SmokeGrenade = 45,
    Weapon_Molotov = 46,
    Weapon_Decoy = 47,
    Weapon_IncGrenade = 48,
    Weapon_C4 = 49,
    Weapon_HealthShot = 57,
    Weapon_Knife_T = 59,
    Weapon_M4A1_Silencer = 60,
    Weapon_USP_Silencer = 61,
    Weapon_CZ75A = 63,
    Weapon_Revolver = 64,
    Weapon_Tagrenade = 68,
    Weapon_Fists = 69,
    Weapon_Breachcharge = 70,
    Weapon_Tablet = 72,
    Weapon_Melee = 74,
    Weapon_Axe = 75,
    Weapon_Hammer = 76,
    Weapon_Spanner = 78,
    Weapon_Knife_Ghost = 80,
    Weapon_Firebomb = 81,
    Weapon_Diversion = 82,
    Weapon_FragGrenade = 83,
    Weapon_Snowball = 84,
    Weapon_BumpMine = 85,
    Weapon_Knife_Bayonet = 500,
    Weapon_Knife_CSS = 503,
    Weapon_Knife_Flip = 505,
    Weapon_Knife_Gut = 506,
    Weapon_Knife_Karam = 507,
    Weapon_Knife_M9_Bayonet = 508,
    Weapon_Knife_Tactical = 509,
    Weapon_Knife_Falchion = 512,
    Weapon_Knife_Survival_Bowie = 514,
    Weapon_Knife_Butterfly = 515,
    Weapon_Knife_Push = 516,
    Weapon_Knife_Cord = 517,
    Weapon_Knife_Canis = 518,
    Weapon_Knife_Ursus = 519,
    Weapon_Knife_Gypsy_Jackknife = 520,
    Weapon_Knife_Outdoor = 521,
    Weapon_Knife_Stiletto = 522,
    Weapon_Knife_Widowmaker = 523,
    Weapon_Knife_Skeleton = 525,
    Glove_Studded_Bloodhound = 5027,
    Glove_T_Side = 5028,
    Glove_CT_Side = 5029,
    Glove_Sporty = 5030,
    Glove_Slick = 5031,
    Glove_Leather_Wrap = 5032,
    Glove_Motorcycle = 5033,
    Glove_Specialist = 5034,
    Glove_Hydra = 5035
};

enum class ClassId : int {
    CBaseEntity = 0,
    CCSPlayerController = 4,
    CCSPlayerPawn = 6,
    CBaseViewModel = 10,
    CHEGrenade = 13,
    CSmokeGrenade = 14,
    CMolotovGrenade = 15,
    CPlantedC4 = 19,
    CInferno = 24
};

enum class EntityFlags : int {
    FL_ONGROUND = (1 << 0),
    FL_DUCKING = (1 << 1),
    FL_WATERJUMP = (1 << 2),
    FL_ONTRAIN = (1 << 3),
    FL_INRAIN = (1 << 4),
    FL_FROZEN = (1 << 5),
    FL_ATCONTROLS = (1 << 6),
    FL_CLIENT = (1 << 7),
    FL_FAKECLIENT = (1 << 8),
    FL_INWATER = (1 << 9),
    FL_FLY = (1 << 10),
    FL_SWIM = (1 << 11),
    FL_CONVEYOR = (1 << 12),
    FL_NPC = (1 << 13),
    FL_GODMODE = (1 << 14),
    FL_NOTARGET = (1 << 15),
    FL_AIMTARGET = (1 << 16),
    FL_PARTIALGROUND = (1 << 17),
    FL_STATICPROP = (1 << 18),
    FL_GRAPHED = (1 << 19),
    FL_GRENADE = (1 << 20),
    FL_STEPMOVEMENT = (1 << 21),
    FL_DONTTOUCH = (1 << 22),
    FL_BASEVELOCITY = (1 << 23),
    FL_WORLDBRUSH = (1 << 24),
    FL_OBJECT = (1 << 25),
    FL_KILLME = (1 << 26),
    FL_ONFIRE = (1 << 27),
    FL_DISSOLVING = (1 << 28),
    FL_TRANSRAGDOLL = (1 << 29),
    FL_UNBLOCKABLE_BY_PLAYER = (1 << 30)
};

enum class ObserverMode : int {
    OBS_MODE_NONE = 0,
    OBS_MODE_DEATHCAM = 1,
    OBS_MODE_FREEZECAM = 2,
    OBS_MODE_FIXED = 3,
    OBS_MODE_IN_EYE = 4,
    OBS_MODE_CHASE = 5,
    OBS_MODE_ROAMING = 6
};

enum class MoveType : int {
    MOVETYPE_NONE = 0,
    MOVETYPE_ISOMETRIC,
    MOVETYPE_WALK,
    MOVETYPE_STEP,
    MOVETYPE_FLY,
    MOVETYPE_FLYGRAVITY,
    MOVETYPE_VPHYSICS,
    MOVETYPE_PUSH,
    MOVETYPE_NOCLIP,
    MOVETYPE_LADDER,
    MOVETYPE_OBSERVER,
    MOVETYPE_CUSTOM,
    MOVETYPE_LAST = MOVETYPE_CUSTOM,
    MOVETYPE_MAX_BITS = 4
};

// Weapon info
struct WeaponInfo {
    char pad_0[0x14];
    int max_clip1;
    int max_clip2;
    int default_clip1;
    int default_clip2;
    char pad_1[0x8];
    char* hud_name;
    char* weapon_name;
    char pad_2[0x3C];
    int weapon_type;
    int weapon_price;
    int kill_award;
    char* animation_prefix;
    float cycle_time;
    float cycle_time_alt;
    float time_to_idle;
    float idle_interval;
    bool full_auto;
    char pad_3[0x3];
    int damage;
    float armor_ratio;
    int bullets;
    float penetration;
    float flinch_velocity_modifier_large;
    float flinch_velocity_modifier_small;
    float range;
    float range_modifier;
    float throw_velocity;
    char pad_4[0xC];
    bool has_silencer;
    char pad_5[0x3];
    float max_speed;
    float max_speed_alt;
    char pad_6[0x4C];
    float spread;
    float spread_alt;
    float inaccuracy_crouch;
    float inaccuracy_stand;
    float inaccuracy_jump;
    float inaccuracy_land;
    float inaccuracy_ladder;
    float inaccuracy_fire;
    float inaccuracy_move;
    float inaccuracy_reload;
    int recoil_seed;
    float recoil_angle;
    float recoil_angle_alt;
    float recoil_angle_variance;
    float recoil_angle_variance_alt;
    float recoil_magnitude;
    float recoil_magnitude_alt;
    float recoil_magnitude_variance;
    float recoil_magnitude_variance_alt;
    float recovery_time_crouch;
    float recovery_time_stand;
    float recovery_time_crouch_final;
    float recovery_time_stand_final;
    int recovery_transition_start_bullet;
    int recovery_transition_end_bullet;
    bool unzoom_after_shot;
    bool hide_view_model_zoomed;
    char pad_7[0x2];
    float walk_speed;
    float run_speed;
    char pad_8[0x10];
    float inaccuracy_fire_alt;
    float recoil_angle_alt2;
    float recoil_angle_variance_alt2;
};

// Bones
enum class BoneIndex : int {
    INVALID = -1,
    PELVIS = 0,
    LEAN_ROOT,
    CAM_DRIVER,
    WEAPON_HAND_R,
    WEAPON_HAND_L,
    WEAPON_SIGHT,
    CAMERA,
    ROOT,
    SPINE_0,
    SPINE_1,
    SPINE_2,
    SPINE_3,
    NECK_0,
    HEAD_0,
    CLAVICLE_L,
    ARM_UPPER_L,
    ARM_LOWER_L,
    HAND_L,
    FINGER_MIDDLE_META_L,
    FINGER_MIDDLE_0_L,
    FINGER_MIDDLE_1_L,
    FINGER_MIDDLE_2_L,
    FINGER_PINKY_META_L,
    FINGER_PINKY_0_L,
    FINGER_PINKY_1_L,
    FINGER_PINKY_2_L,
    FINGER_INDEX_META_L,
    FINGER_INDEX_0_L,
    FINGER_INDEX_1_L,
    FINGER_INDEX_2_L,
    FINGER_THUMB_0_L,
    FINGER_THUMB_1_L,
    FINGER_THUMB_2_L,
    CLAVICLE_R,
    ARM_UPPER_R,
    ARM_LOWER_R,
    HAND_R,
    FINGER_MIDDLE_META_R,
    FINGER_MIDDLE_0_R,
    FINGER_MIDDLE_1_R,
    FINGER_MIDDLE_2_R,
    FINGER_PINKY_META_R,
    FINGER_PINKY_0_R,
    FINGER_PINKY_1_R,
    FINGER_PINKY_2_R,
    FINGER_INDEX_META_R,
    FINGER_INDEX_0_R,
    FINGER_INDEX_1_R,
    FINGER_INDEX_2_R,
    FINGER_THUMB_0_R,
    FINGER_THUMB_1_R,
    FINGER_THUMB_2_R,
    LEG_UPPER_L,
    LEG_LOWER_L,
    ANKLE_L,
    FOOT_L,
    TOE_L,
    LEG_UPPER_R,
    LEG_LOWER_R,
    ANKLE_R,
    FOOT_R,
    TOE_R,
    MAX
};

// Animation layers
struct AnimationLayer {
    char pad_0[0x8];
    uint32_t order;
    uint32_t sequence;
    float prev_cycle;
    float weight;
    float weight_delta_rate;
    float playback_rate;
    float cycle;
    void* owner;
    char pad_1[0x4];
};

// Hitbox
struct Hitbox {
    int bone;
    int group;
    Vector3D mins;
    Vector3D maxs;
    int name_index;
    float radius;
    char pad[0x4];
};

struct StudioHdr {
    int id;
    int version;
    long checksum;
    char name[64];
    int length;
    Vector3D eye_pos;
    Vector3D illum_pos;
    Vector3D hull_min;
    Vector3D hull_max;
    Vector3D view_bb_min;
    Vector3D view_bb_max;
    int flags;
    int bone_count;
    int bone_offset;
    int hitbox_count;
    int hitbox_offset;
    int local_bone_count;
    int local_bone_offset;
    int mesh_count;
    int mesh_offset;
    int model_count;
    int model_offset;
};

// UserCmd
struct CUserCmd {
    char pad_0[0x8];
    int command_number;
    int tick_count;
    QAngle view_angles;
    Vector3D aim_direction;
    float forward_move;
    float side_move;
    float up_move;
    int buttons;
    char impulse;
    int weapon_select;
    int weapon_subtype;
    int random_seed;
    short mouse_dx;
    short mouse_dy;
    bool has_been_predicted;
    Vector3D head_angles;
    Vector3D head_offset;
    bool used_teleport;
    char pad_1[0x3];
int touch_count;
    Vector2D touches[24];
};

struct CMoveData {
    bool first_run_of_functions;
    bool game_code_moved_player;
    int player_handle;
    int impulse_command;
    QAngle view_angles;
    QAngle abs_view_angles;
    float forward_move;
    float side_move;
    float up_move;
    float max_speed;
    float client_max_speed;
    Vector3D velocity;
    Vector3D old_velocity;
    float unknown;
    QAngle angles;
    QAngle old_angles;
    float out_step_height;
    Vector3D out_wish_vel;
    Vector3D out_jump_vel;
    Vector3D constraint_center;
    float constraint_radius;
    float constraint_width;
    float constraint_speed_factor;
    bool unknown_bool;
    Vector3D m_vecAbsOrigin;
};

struct InputHistoryEntry {
    QAngle view_angles;
    int target_tick;
    int render_tick_start;
    int render_tick_fraction;
    int buttons;
    int buttons_changed;
    Vector3D move;
    int impulse;
    int weapon_select;
    int weapon_subtype;
    int sequence_number;
    Vector3D origin;
    Vector3D angles;
    Vector3D velocity;
    int flags;
    float duck_amount;
    int tick_base;
    int command_number;
    int weapon;
    int weapon_subtype2;
    int buttons2;
    int buttons_changed2;
};

// Game rules
struct CCSGameRules {
    char pad_0[0x20];
    bool freeze_period;
    bool warmup_period;
    bool valve_ds;
    bool bomb_planted;
    bool bomb_defused;
    bool bomb_exploded;
    bool round_end;
    int total_rounds_played;
    int rounds_won_team_ct;
    int rounds_won_team_t;
    int match_type;
    char pad_1[0x100];
    float round_start_time;
    char pad_2[0x4];
    int round_end_reason;
};

// ConVar
struct ConVar {
    void* vtable;
    ConVar* next;
    int registered;
    char* name;
    char* help_string;
    int flags;
    char pad_0[0x4];
    ConVar* parent;
    char* default_value;
    char* string;
    int string_length;
    float float_value;
    int int_value;
    bool has_min;
    float min_value;
    bool has_max;
    float max_value;
    void* change_callback;
};

// Client class
struct ClientClass {
    void* create_fn;
    void* create_event_fn;
    char* network_name;
    void* recv_table;
    ClientClass* next;
    int class_id;
};

// Interfaces forward declarations
struct IEngineClient;
struct IClientEntityList;
struct IBaseClientDLL;
struct IVEngineClient;
struct IClientMode;
struct IInputSystem;
struct IInput;
struct ICvar;
struct IGameEventManager2;
struct IGameMovement;
struct IPrediction;
struct IMoveHelper;
struct IPhysicsSurfaceProps;
struct IEngineTrace;
struct IDebugOverlay;
struct IVModelInfo;
struct IVModelRender;
struct IVRenderView;
struct IMaterialSystem;
struct IMaterial;
struct IStudioRender;
struct IVDebugOverlay;
struct ILocalize;
struct IEngineSound;
struct IGameUIFuncs;
struct IMatchFramework;
struct IMatchmaker;
struct ISteamClient;
struct ISteamUser;
struct ISteamFriends;
struct ISteamUtils;
struct ISteamMatchmaking;
struct ISteamUserStats;
struct ISteamApps;
struct ISteamNetworking;
struct ISteamRemoteStorage;
struct ISteamScreenshots;
struct ISteamHTTP;
struct ISteamController;
struct ISteamUGC;
struct ISteamAppList;
struct ISteamMusic;
struct ISteamMusicRemote;
struct ISteamHTMLSurface;
struct ISteamInventory;
struct ISteamVideo;
struct ISteamParentalSettings;
struct ISteamInput;

// Global vars
struct GlobalVars {
    float real_time;
    int frame_count;
    float absolute_frame_time;
    float absolute_frame_start_time_std_dev;
    float current_time;
    float frame_time;
    int max_clients;
    int tick_count;
    float interval_per_tick;
    float interpolation_amount;
    int simulation_ticks_this_frame;
    int network_protocol;
    void* save_data;
    bool client;
    bool remote_client;
    int last_timestamp_networked;
    int timestamp_randomize_window;
    char pad_0[0x4];
};

// Color
struct Color {
    uint8_t r, g, b, a;
    Color() : r(255), g(255), b(255), a(255) {}
    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255) : r(r_), g(g_), b(b_), a(a_) {}
    Color(uint32_t hex) : r((hex >> 24) & 0xFF), g((hex >> 16) & 0xFF), b((hex >> 8) & 0xFF), a(hex & 0xFF) {}
    uint32_t ToU32() const { return (r << 24) | (g << 16) | (b << 8) | a; }
    static Color FromHSV(float h, float s, float v) {
        float c = v * s;
        float x = c * (1 - fabsf(fmodf(h / 60.0f, 2) - 1));
        float m = v - c;
        float r, g, b;
        if (h < 60) { r = c; g = x; b = 0; }
        else if (h < 120) { r = x; g = c; b = 0; }
        else if (h < 180) { r = 0; g = c; b = x; }
        else if (h < 240) { r = 0; g = x; b = c; }
        else if (h < 300) { r = x; g = 0; b = c; }
        else { r = c; g = 0; b = x; }
        return Color((r + m) * 255, (g + m) * 255, (b + m) * 255);
    }
};

// Ray / Trace
struct Ray_t {
    Vector3D start;
    Vector3D delta;
    Vector3D start_offset;
    Vector3D extents;
    bool is_ray;
    bool is_swept;
    Ray_t() : delta(0,0,0), is_ray(true), is_swept(false) {}
    void Init(const Vector3D& start_, const Vector3D& end_) {
        start = start_;
        delta = end_ - start_;
        is_swept = delta.Length() > 0;
        is_ray = true;
    }
};

struct cplane_t {
    Vector3D normal;
    float dist;
    char type;
    char signbits;
    char pad[2];
};

struct csurface_t {
    char name[16];
    short surface_props;
    unsigned short flags;
};

struct Trace_t {
    Vector3D start_pos;
    Vector3D end_pos;
    cplane_t plane;
    float fraction;
    int contents;
    unsigned short disp_flags;
    bool all_solid;
    bool start_solid;
    float fraction_left_solid;
    csurface_t surface;
    int hitgroup;
    short physics_bone;
    unsigned short world_surface_index;
    void* entity;
    int hitbox;
    bool hit;
    bool did_hit_world() const { return entity == nullptr; }
    bool did_hit_non_world() const { return entity != nullptr; }
};

struct TraceFilter {
    void* skip;
    int collision_group;
    bool should_hit_entity(void* entity, int mask) { return entity != skip; }
    int get_trace_type() const { return 0; }
};

// Player info
struct PlayerInfo {
    uint64_t xuid;
    char name[128];
    int user_id;
    char guid[33];
    uint32_t friend_id;
    char friends_name[128];
    bool fake_player;
    bool is_hltv;
    int custom_files[4];
    unsigned char files_downloaded;
};

// View setup
struct CViewSetup {
    char pad_0[0x8];
    int x, x_old;
    int y, y_old;
    int width, width_old;
    int height, height_old;
    bool ortho;
    float ortho_left;
    float ortho_top;
    float ortho_right;
    float ortho_bottom;
    bool use_custom_view_matrix;
    Matrix4x4 custom_view_matrix;
    char pad_1[0x20];
    float fov;
    float fov_viewmodel;
    Vector3D origin;
    QAngle angles;
    float z_near;
    float z_far;
    float z_near_viewmodel;
    float z_far_viewmodel;
    float aspect_ratio;
    bool render_to_subrect_of_larger_screen;
    float forced_aspect_ratio;
};

// Material
struct MaterialParam {
    char* name;
    float value[4];
    int type;
};

} // namespace sdk