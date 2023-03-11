#include <RmlUi/Core.h>

#include <raylib-cpp.hpp>

#include "raylib.h"

using namespace Rml;

class GameRenderInterface : public RenderInterface
{
   public:
	GameRenderInterface() {}
	virtual ~GameRenderInterface() = default;

	void RenderGeometry(Vertex *vertices, int num_vertices, int *indices,
						int num_indices, TextureHandle texture,
						const Vector2f &translation) override;
	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(int x, int y, int width, int height) override;
};

class GameSystemInterface : public SystemInterface
{
   public:
	GameSystemInterface() {}
	virtual ~GameSystemInterface() = default;

	double GetElapsedTime() override;
	bool   LogMessage(Log::Type type, const String &message) override;
};

static GameRenderInterface render_interface_instance;
static GameSystemInterface system_interface_instance;

extern RenderInterface *const render_interface = &render_interface_instance;
extern SystemInterface *const system_interface = &system_interface_instance;

void GameRenderInterface::RenderGeometry(Vertex *vertices, int num_vertices,
										 int *indices, int num_indices,
										 TextureHandle	 texture,
										 const Vector2f &translation) {}
void GameRenderInterface::EnableScissorRegion(bool enable) {}
void GameRenderInterface::SetScissorRegion(int x, int y, int width, int height) {}

double GameSystemInterface::GetElapsedTime() { return GetTime(); }

bool GameSystemInterface::LogMessage(Log::Type	   type,
									 const String &message)
{
	TraceLogLevel level = LOG_FATAL;
	switch (type) {
		case Log::LT_ALWAYS:
			level = LOG_ALL;
			break;
		case Log::LT_ERROR:
			level = LOG_ERROR;
			break;
		case Log::LT_ASSERT:
			level = LOG_FATAL;
			break;
		case Log::LT_WARNING:
			level = LOG_WARNING;
			break;
		case Log::LT_INFO:
			level = LOG_INFO;
			break;
		case Log::LT_DEBUG:
			level = LOG_DEBUG;
			break;
		case Log::LT_MAX:
			level = LOG_TRACE;
			break;
	}
	TraceLog(level, "RMLUI: %s", message.c_str());
	return true;
}
