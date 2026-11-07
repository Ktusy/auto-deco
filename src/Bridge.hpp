#pragma once

#include "Generator.hpp"

#include <string>

namespace autodeco {

struct RunResult {
    bool ok = false;
    int  created = 0;
    int  removed = 0;
    std::string message;   // user-facing text (success summary or error)
};

// Reads the layout from the open level editor, generates decoration and
// inserts it. Objects are tagged so they can be undone / removed later.
RunResult decorateEditor(Options const& opt);

// Removes the objects of the most recent run (lastRunOnly) or every
// auto-generated object from this editor session.
RunResult removeDeco(bool lastRunOnly);

// True if there is at least one run that can still be undone.
bool hasUndoableRun();

// True while the level editor is open.
bool editorIsOpen();

}  // namespace autodeco
