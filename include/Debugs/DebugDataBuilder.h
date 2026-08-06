#pragma once

#include "DebugSettings.h"

class Application;
class World;

class DebugDataBuilder {
public:

	static DebugSettings BuildDebugData(const Application& app, World& world);



};