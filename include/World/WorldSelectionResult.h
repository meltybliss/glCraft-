#pragma once

enum class WorldSelectionAction
{
    None,
    CreateNew,
    LoadWorld
};


struct WorldSelectionResult {

    WorldSelectionAction action = WorldSelectionAction::None;

};
