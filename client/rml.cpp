#include <RmlUi/Core.h>

#include <raylib-cpp.hpp>
#include <vector>

using namespace Rml;

class GameRenderInterface : public RenderInterface
{
	std::vector<raylib::TextureUnmanaged> textures;

   public:
	GameRenderInterface() {}

	// This is a static global object, which is deleted on process shutdown.
	// All raylib textures were already unloaded, thus we do not need to do anything in destructor.
	virtual ~GameRenderInterface() = default;

	void RenderGeometry(Vertex *vertices, int num_vertices, int *indices,
						int num_indices, TextureHandle texture,
						const Vector2f &translation) override;
	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(int x, int y, int width, int height) override;
	bool LoadTexture(TextureHandle &texture_handle, Vector2i &texture_dimensions, const String &source) override;
	void ReleaseTexture(TextureHandle texture) override;
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

bool GameRenderInterface::LoadTexture(TextureHandle &texture_handle, Vector2i &texture_dimensions, const String &source)
{
	try {
		textures.emplace_back(raylib::TextureUnmanaged(source));
		texture_handle	   = (Rml::TextureHandle)textures.size() - 1;
		texture_dimensions = Rml::Vector2i(textures.back().width, textures.back().height);
		return true;
	}
	catch (raylib::RaylibException e) {
		return false;
	}
}

void GameRenderInterface::ReleaseTexture(TextureHandle texture)
{
	auto erased = textures.erase(textures.begin() + (decltype(textures)::size_type)texture);
	erased->Unload();
}

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
