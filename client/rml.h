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

	bool LoadTexture(Rml::TextureHandle &texture_handle,
					 Rml::Vector2i		&texture_dimensions,
					 const Rml::String	&source) override;
	bool GenerateTexture(Rml::TextureHandle	 &texture_handle,
						 const Rml::byte	 *source,
						 const Rml::Vector2i &source_dimensions) override;
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

class GameFileInterface : public Rml::FileInterface
{
   public:
	GameFileInterface(char *argv[]);
	virtual ~GameFileInterface();

	Rml::FileHandle Open(const Rml::String &path) override;
	void			Close(Rml::FileHandle file) override;
	size_t			Read(void *buffer, size_t size, Rml::FileHandle file) override;
	bool			Seek(Rml::FileHandle file, long offset, int origin) override;
	size_t			Tell(Rml::FileHandle file) override;
	size_t			Length(Rml::FileHandle file) override;

	void mount(Rml::String const &newDir, Rml::String const &mountPoint = "/", bool appendToPath = true);
};
