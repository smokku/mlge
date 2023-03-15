#include <RmlUi/Core.h>

#include <raylib-cpp.hpp>
#include <vector>

#include "rlgl.h"

class GameRenderInterface : public Rml::RenderInterface
{
	rlRenderBatch		 batch;
	unsigned int		 default_texture_id;
	bool				 scissor;
	int					 scissor_x, scissor_y, scissor_width, scissor_height;
	const Rml::Matrix4f *transform;

   public:
	GameRenderInterface();
	virtual ~GameRenderInterface();

	void BeginFrame();
	void EndFrame();

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

	void SetTransform(const Rml::Matrix4f *transform) override;
};

Rml::Input::KeyIdentifier raylib_key_to_identifier(KeyboardKey key);

class GameSystemInterface : public Rml::SystemInterface
{
   public:
	GameSystemInterface() {}
	virtual ~GameSystemInterface() = default;

	double GetElapsedTime() override;
	bool   LogMessage(Rml::Log::Type type, const Rml::String &message) override;
	void   SetMouseCursor(const Rml::String &cursor_name) override;
	void   SetClipboardText(const Rml::String &text) override;
	void   GetClipboardText(Rml::String &text) override;
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
