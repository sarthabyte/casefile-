#ifndef STRUCTURES_H
#define STRUCTURES_H

#define MAX_TEXT   200
#define MAX_NAME   40
#define MAX_FLAGS  256

#define LOCATION_COUNT 12
#define MAX_LOCATION_CONNECTIONS 4

#define FLAG_NONE               (-1)
#define FLAG_PROVEN_BASE        64
#define FLAG_COURT_MISTAKES     128
#define FLAG_INVESTIGATION_DONE 129
#define FLAG_TIMELINE_SUBMITTED 130
#define FLAG_ROOM_VISITED_BASE 140
#define FLAG_ROOM_COUNT 153
#define FLAG_SELECTED_SUSPECT_BASE 160
#define FLAG_SELECTED_EVIDENCE_BASE 180
#define FLAG_EVIDENCE_BASE 200
#define MAX_INVESTIGATED_ROOMS 6
#define LOCKER_CLUE_COUNT 6
#define LOCKER_ROOM_ID 6

typedef enum {
    PHASE_DETECTIVE,
    PHASE_DEFENCE,
    PHASE_ENDED
} Phase;

typedef enum {
    OUTCOME_NONE,
    OUTCOME_INNOCENT,
    OUTCOME_GUILTY
} Outcome;

enum {
    EVID_NONE = 0,
    EVID_ACCESS_LOG,
    EVID_CCTV,
    EVID_CALL_RECORDING,
    EVID_SLIP_MARKS,
    EVID_GATE_LOG,
    EVID_COMPLAINT,
    EVID_BROKEN_WATCH,
    EVID_WET_FOOTPRINT,
    EVID_TORN_NOTE,
    EVID_MISSING_KEY,
    EVID_CLEANING_RECEIPT,
    EVID_SECOND_PHONE
};

typedef struct {
    int  id;
    char name[MAX_NAME];
    int  discovered;
    int  locationId;
    char description[MAX_TEXT];
} Evidence;

typedef struct EvidenceNode {
    Evidence data;
    struct EvidenceNode *next;
} EvidenceNode;

typedef struct {
    int  id;
    char name[MAX_NAME];
    char description[MAX_TEXT];
    int  connections[MAX_LOCATION_CONNECTIONS];
    int  connectionCount;
} Location;

typedef struct {
    int  id;
    char name[MAX_NAME];
    char occupation[MAX_NAME];
    char alibi[MAX_TEXT];
    char dialogueFile[60];
} Suspect;

typedef struct {
    int  id;
    int  speakerId;
    int  requiredFlag;
    char text[MAX_TEXT];
} Dialogue;

typedef struct {
    int  roomID;
    char digit;
    int  discovered;
} LockerClue;

typedef struct {
    Phase         phase;
    int           playerLocation;
    EvidenceNode *evidence;
    int           flags[MAX_FLAGS];
    Outcome       outcome;
    LockerClue    lockerClues[LOCKER_CLUE_COUNT];
    int           lockerFound;
    int           lockerOpened;
} GameState;

#endif
