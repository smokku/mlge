#include <RmlUi/Core.h>

#include <raylib-cpp.hpp>
#include <vector>

class GameRenderInterface : public Rml::RenderInterface
{
	std::vector<raylib::Texture> textures;

   public:
	GameRenderInterface() {}
	virtual ~GameRenderInterface() = default;

	void RenderGeometry(Rml::Vertex *vertices, int num_vertices, int *indices,
						int num_indices, Rml::TextureHandle texture,
						const Rml::Vector2f &translation) override;
	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(int x, int y, int width, int height) override;
	bool LoadTexture(Rml::TextureHandle &texture_handle, Rml::Vector2i &texture_dimensions, const Rml::String &source) override;
	void ReleaseTexture(Rml::TextureHandle texture) override;
};

class GameSystemInterface : public Rml::SystemInterface
{
   public:
	GameSystemInterface() {}
	virtual ~GameSystemInterface() = default;

	double GetElapsedTime() override;
	bool   LogMessage(Rml::Log::Type type, const Rml::String &message) override;
};
