#include "rml.h"

#include <physfs.h>

using namespace Rml;

// --- Render Interface ----------------------------------------------------

void GameRenderInterface::RenderGeometry(Vertex *vertices, int num_vertices,
										 int *indices, int num_indices,
										 TextureHandle	 texture,
										 const Vector2f &translation) {}
void GameRenderInterface::EnableScissorRegion(bool enable) {}
void GameRenderInterface::SetScissorRegion(int x, int y, int width, int height) {}

bool GameRenderInterface::LoadTexture(TextureHandle &texture_handle, Vector2i &texture_dimensions, const String &source)
{
	try {
		textures.emplace_back(raylib::Texture(source));
		texture_handle	   = (TextureHandle)textures.size() - 1;
		texture_dimensions = Vector2i(textures.back().width, textures.back().height);
		return true;
	}
	catch (raylib::RaylibException e) {
		return false;
	}
}

void GameRenderInterface::ReleaseTexture(TextureHandle texture)
{
	textures.erase(textures.begin() + (decltype(textures)::size_type)texture);
}

// --- System Interface ----------------------------------------------------

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

// --- File Interface ----------------------------------------------------

GameFileInterface::GameFileInterface(char *argv[])
{
	PHYSFS_init(argv[0]);
}

GameFileInterface::~GameFileInterface()
{
	PHYSFS_deinit();
}

FileHandle GameFileInterface::Open(const String &path)
{
	return reinterpret_cast<FileHandle>(PHYSFS_openRead(path.c_str()));
}

void GameFileInterface::Close(FileHandle file)
{
	PHYSFS_close(reinterpret_cast<PHYSFS_File *>(file));
}

size_t GameFileInterface::Read(void *buffer, size_t size, FileHandle file)
{
	return PHYSFS_readBytes(reinterpret_cast<PHYSFS_File *>(file), buffer, size);
}

bool GameFileInterface::Seek(FileHandle file, long offset, int origin)
{
	switch (origin) {
		case SEEK_SET:
			return PHYSFS_seek(reinterpret_cast<PHYSFS_File *>(file), offset);
		case SEEK_CUR:
			return PHYSFS_seek(reinterpret_cast<PHYSFS_File *>(file), PHYSFS_tell(reinterpret_cast<PHYSFS_File *>(file)) + offset);
		case SEEK_END:
			return PHYSFS_seek(reinterpret_cast<PHYSFS_File *>(file), PHYSFS_fileLength(reinterpret_cast<PHYSFS_File *>(file)) - offset);
	}
	return false;
}

size_t GameFileInterface::Tell(FileHandle file)
{
	return PHYSFS_tell(reinterpret_cast<PHYSFS_File *>(file));
}

size_t GameFileInterface::Length(FileHandle file)
{
	return PHYSFS_fileLength(reinterpret_cast<PHYSFS_File *>(file));
}

void GameFileInterface::mount(Rml::String const &newDir, Rml::String const &mountPoint, bool appendToPath)
{
	PHYSFS_mount(newDir.c_str(), mountPoint.c_str(), appendToPath);
}
