#include <assert.h>
#include <stdio.h>
#include "suspects.h"
#include "dialogue.h"

int main(void) {
    assert(loadSuspects("data/suspects.txt") == 4);
    assert(getSuspectCount() == 4);
    const Suspect *v = getSuspectById(0);
    assert(v && v->id == 0);
    assert(v && v->name[0] == 'V');
    assert(loadAllDialogue() == 23);
    assert(getDialogueCount() == 23);
    puts("PASS: suspects and dialogue load dynamically from data files.");
    return 0;
}
