#include "rml.h"

#include <physfs.h>

using namespace Rml;

// --- Render Interface ----------------------------------------------------

GameRenderInterface::GameRenderInterface()
	: default_texture_id{0},
	  scissor{false},
	  scissor_x{0},
	  scissor_y{0},
	  scissor_width{0},
	  scissor_height{0},
	  transform{nullptr}
{
	batch = rlLoadRenderBatch(RL_DEFAULT_BATCH_BUFFERS, RL_DEFAULT_BATCH_BUFFER_ELEMENTS);
}

GameRenderInterface::~GameRenderInterface()
{
	rlUnloadRenderBatch(batch);
}

void GameRenderInterface::BeginFrame()
{
	rlSetRenderBatchActive(&batch);
	default_texture_id = batch.draws[0].textureId;
}

void GameRenderInterface::EndFrame()
{
	rlSetRenderBatchActive(nullptr);
}

void GameRenderInterface::RenderGeometry(Vertex *vertices, int num_vertices,
										 int *indices, int num_indices,
										 TextureHandle	 texture_handle,
										 const Vector2f &translation)
{
	if (scissor) BeginScissorMode(scissor_x, scissor_y, scissor_width, scissor_height);

	rlPushMatrix();

	rlTranslatef(translation.x, translation.y, 0.0f);
	if (transform) {
		rlMultMatrixf(transform->data());
	}

	// Texturing is only supported on RL_QUADS
	rlBegin(RL_QUADS);

	if (texture_handle)
		rlSetTexture(reinterpret_cast<raylib::Texture *>(texture_handle)->id);
	else
		rlSetTexture(default_texture_id);

	for (int index = 0; index < num_indices; index++) {
		auto vertex_index = indices[index];
		if (vertex_index > num_vertices) continue;

		auto vertex = &vertices[vertex_index];

		rlColor4ub(vertex->colour.red, vertex->colour.green, vertex->colour.blue, vertex->colour.alpha);
		rlTexCoord2f(vertex->tex_coord.x, vertex->tex_coord.y);
		rlVertex2f(vertex->position.x, vertex->position.y);

		if (index % 3 == 2)	 // duplicate every third vertex to create quad
			rlVertex2f(vertex->position.x, vertex->position.y);
	}
	rlEnd();

	rlPopMatrix();
	rlSetTexture(0);
	if (scissor) EndScissorMode();
}

void GameRenderInterface::EnableScissorRegion(bool enable)
{
	scissor = enable;
}

void GameRenderInterface::SetScissorRegion(int x, int y, int width, int height)
{
	scissor_x	   = x;
	scissor_y	   = y;
	scissor_width  = width;
	scissor_height = height;
}

bool GameRenderInterface::LoadTexture(TextureHandle &texture_handle, Vector2i &texture_dimensions, const String &source)
{
	try {
		auto texture	   = new raylib::Texture(source);
		texture_handle	   = reinterpret_cast<TextureHandle>(texture);
		texture_dimensions = Vector2i(texture->width, texture->height);
		return true;
	}
	catch (raylib::RaylibException e) {
		return false;
	}
}

bool GameRenderInterface::GenerateTexture(TextureHandle &texture_handle, const byte *source, const Vector2i &source_dimensions)
{
	try {
		raylib::Image image(nullptr, source_dimensions.x, source_dimensions.y);
		auto		  texture = new raylib::Texture(image);
		texture->Update(source);
		texture_handle = reinterpret_cast<TextureHandle>(texture);
		return true;
	}
	catch (raylib::RaylibException e) {
		return false;
	}
}

void GameRenderInterface::ReleaseTexture(TextureHandle texture_handle)
{
	delete reinterpret_cast<raylib::Texture *>(texture_handle);
}

void GameRenderInterface::SetTransform(const Matrix4f *transform)
{
	this->transform = transform;
}

// --- System Interface ----------------------------------------------------

double GameSystemInterface::GetElapsedTime()
{
	return GetTime();
}

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

void GameSystemInterface::SetMouseCursor(const String &cursor_name)
{
	MouseCursor cursor = MouseCursor::MOUSE_CURSOR_DEFAULT;

	if (cursor_name.rfind("rmlui-scroll-", 0) == 0) {
		cursor = MouseCursor::MOUSE_CURSOR_RESIZE_ALL;
	}

	if (cursor_name == "") {
		cursor = MouseCursor::MOUSE_CURSOR_ARROW;
	}
	if (cursor_name == "text") {
		cursor = MouseCursor::MOUSE_CURSOR_IBEAM;
	}
	if (cursor_name == "crosshair") {
		cursor = MouseCursor::MOUSE_CURSOR_CROSSHAIR;
	}
	if (cursor_name == "pointer" || cursor_name == "hand") {
		cursor = MouseCursor::MOUSE_CURSOR_POINTING_HAND;
	}
	if (cursor_name == "col-resize" || cursor_name == "ew-resize") {
		cursor = MouseCursor::MOUSE_CURSOR_RESIZE_EW;
	}
	if (cursor_name == "row-resize" || cursor_name == "ns-resize") {
		cursor = MouseCursor::MOUSE_CURSOR_RESIZE_NS;
	}
	if (cursor_name == "nwse-resize") {
		cursor = MouseCursor::MOUSE_CURSOR_RESIZE_NWSE;
	}
	if (cursor_name == "nesw-resize") {
		cursor = MouseCursor::MOUSE_CURSOR_RESIZE_NESW;
	}
	if (cursor_name == "all-scroll" || cursor_name == "move") {
		cursor = MouseCursor::MOUSE_CURSOR_RESIZE_ALL;
	}
	if (cursor_name == "not-allowed") {
		cursor = MouseCursor::MOUSE_CURSOR_NOT_ALLOWED;
	}

	::SetMouseCursor(cursor);
}

void GameSystemInterface::SetClipboardText(const String &text)
{
	::SetClipboardText(text.c_str());
}
void GameSystemInterface::GetClipboardText(String &text)
{
	text = ::GetClipboardText();
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

// --- KeyConversion ----------------------------------------------------
// TODO: make it a look-up table
Input::KeyIdentifier raylib_key_to_identifier(KeyboardKey key)
{
	auto key_id = Input::KeyIdentifier::KI_UNKNOWN;

	switch (key) {
		case KEY_NULL:
			break;
		case KEY_APOSTROPHE:
			key_id = Input::KeyIdentifier::KI_OEM_7;  // US standard keyboard; the ''"' key.
			break;
		case KEY_COMMA:
			key_id = Input::KeyIdentifier::KI_OEM_COMMA;  // Any region; the ',<' key.
			break;
		case KEY_MINUS:
			key_id = Input::KeyIdentifier::KI_OEM_MINUS;  // Any region; the '-_' key.
			break;
		case KEY_PERIOD:
			key_id = Input::KeyIdentifier::KI_OEM_PERIOD;  // Any region; the '.>' key.
			break;
		case KEY_SLASH:
			key_id = Input::KeyIdentifier::KI_OEM_2;  // Any region; the '/?' key.
			break;
		case KEY_ZERO:
			key_id = Input::KeyIdentifier::KI_0;
			break;
		case KEY_ONE:
			key_id = Input::KeyIdentifier::KI_1;
			break;
		case KEY_TWO:
			key_id = Input::KeyIdentifier::KI_2;
			break;
		case KEY_THREE:
			key_id = Input::KeyIdentifier::KI_3;
			break;
		case KEY_FOUR:
			key_id = Input::KeyIdentifier::KI_4;
			break;
		case KEY_FIVE:
			key_id = Input::KeyIdentifier::KI_5;
			break;
		case KEY_SIX:
			key_id = Input::KeyIdentifier::KI_6;
			break;
		case KEY_SEVEN:
			key_id = Input::KeyIdentifier::KI_7;
			break;
		case KEY_EIGHT:
			key_id = Input::KeyIdentifier::KI_8;
			break;
		case KEY_NINE:
			key_id = Input::KeyIdentifier::KI_9;
			break;
		case KEY_SEMICOLON:
			key_id = Input::KeyIdentifier::KI_OEM_1;  // US standard keyboard; the ';:' key.
			break;
		case KEY_EQUAL:
			key_id = Input::KeyIdentifier::KI_OEM_PLUS;	 // Any region; the '=+' key.
			break;
		case KEY_A:
			key_id = Input::KeyIdentifier::KI_A;
			break;
		case KEY_B:
			key_id = Input::KeyIdentifier::KI_B;
			break;
		case KEY_C:
			key_id = Input::KeyIdentifier::KI_C;
			break;
		case KEY_D:
			key_id = Input::KeyIdentifier::KI_D;
			break;
		case KEY_E:
			key_id = Input::KeyIdentifier::KI_E;
			break;
		case KEY_F:
			key_id = Input::KeyIdentifier::KI_F;
			break;
		case KEY_G:
			key_id = Input::KeyIdentifier::KI_G;
			break;
		case KEY_H:
			key_id = Input::KeyIdentifier::KI_H;
			break;
		case KEY_I:
			key_id = Input::KeyIdentifier::KI_I;
			break;
		case KEY_J:
			key_id = Input::KeyIdentifier::KI_J;
			break;
		case KEY_K:
			key_id = Input::KeyIdentifier::KI_K;
			break;
		case KEY_L:
			key_id = Input::KeyIdentifier::KI_L;
			break;
		case KEY_M:
			key_id = Input::KeyIdentifier::KI_M;
			break;
		case KEY_N:
			key_id = Input::KeyIdentifier::KI_N;
			break;
		case KEY_O:
			key_id = Input::KeyIdentifier::KI_O;
			break;
		case KEY_P:
			key_id = Input::KeyIdentifier::KI_P;
			break;
		case KEY_Q:
			key_id = Input::KeyIdentifier::KI_Q;
			break;
		case KEY_R:
			key_id = Input::KeyIdentifier::KI_R;
			break;
		case KEY_S:
			key_id = Input::KeyIdentifier::KI_S;
			break;
		case KEY_T:
			key_id = Input::KeyIdentifier::KI_T;
			break;
		case KEY_U:
			key_id = Input::KeyIdentifier::KI_U;
			break;
		case KEY_V:
			key_id = Input::KeyIdentifier::KI_V;
			break;
		case KEY_W:
			key_id = Input::KeyIdentifier::KI_W;
			break;
		case KEY_X:
			key_id = Input::KeyIdentifier::KI_X;
			break;
		case KEY_Y:
			key_id = Input::KeyIdentifier::KI_Y;
			break;
		case KEY_Z:
			key_id = Input::KeyIdentifier::KI_Z;
			break;
		case KEY_LEFT_BRACKET:
			key_id = Input::KeyIdentifier::KI_OEM_4;  // US standard keyboard; the '[{' key.
			break;
		case KEY_BACKSLASH:
			key_id = Input::KeyIdentifier::KI_OEM_5;  // US standard keyboard; the '\|' key.
			break;
		case KEY_RIGHT_BRACKET:
			key_id = Input::KeyIdentifier::KI_OEM_6;  // US standard keyboard; the ']}' key.
			break;
		case KEY_GRAVE:
			key_id = Input::KeyIdentifier::KI_OEM_3;  // Any region; the '`~' key.
			break;
		case KEY_SPACE:
			key_id = Input::KeyIdentifier::KI_SPACE;
			break;
		case KEY_ESCAPE:
			key_id = Input::KeyIdentifier::KI_ESCAPE;  // Escape key.
			break;
		case KEY_ENTER:
			key_id = Input::KeyIdentifier::KI_RETURN;
			break;
		case KEY_TAB:
			key_id = Input::KeyIdentifier::KI_TAB;	// Tab key.
			break;
		case KEY_BACKSPACE:
			key_id = Input::KeyIdentifier::KI_BACK;	 // Backspace key.
			break;
		case KEY_INSERT:
			key_id = Input::KeyIdentifier::KI_INSERT;
			break;
		case KEY_DELETE:
			key_id = Input::KeyIdentifier::KI_DELETE;
			break;
		case KEY_RIGHT:
			key_id = Input::KeyIdentifier::KI_RIGHT;  // Right Arrow key.
			break;
		case KEY_LEFT:
			key_id = Input::KeyIdentifier::KI_LEFT;	 // Left Arrow key.
			break;
		case KEY_DOWN:
			key_id = Input::KeyIdentifier::KI_DOWN;	 // Down Arrow key.
			break;
		case KEY_UP:
			key_id = Input::KeyIdentifier::KI_UP;  // Up Arrow key.
			break;
		case KEY_PAGE_UP:
			key_id = Input::KeyIdentifier::KI_PRIOR;  // Page Up key.
			break;
		case KEY_PAGE_DOWN:
			key_id = Input::KeyIdentifier::KI_NEXT;	 // Page Down key.
			break;
		case KEY_HOME:
			key_id = Input::KeyIdentifier::KI_HOME;
			break;
		case KEY_END:
			key_id = Input::KeyIdentifier::KI_END;
			break;
		case KEY_CAPS_LOCK:
			key_id = Input::KeyIdentifier::KI_CAPITAL;	// Capslock key.
			break;
		case KEY_SCROLL_LOCK:
			key_id = Input::KeyIdentifier::KI_SCROLL;  // Scroll Lock key.
			break;
		case KEY_NUM_LOCK:
			key_id = Input::KeyIdentifier::KI_NUMLOCK;	// Numlock key.
			break;
		case KEY_PRINT_SCREEN:
			key_id = Input::KeyIdentifier::KI_SNAPSHOT;	 // Print Screen key.
			break;
		case KEY_PAUSE:
			key_id = Input::KeyIdentifier::KI_PAUSE;
			break;
		case KEY_F1:
			key_id = Input::KeyIdentifier::KI_F1;
			break;
		case KEY_F2:
			key_id = Input::KeyIdentifier::KI_F2;
			break;
		case KEY_F3:
			key_id = Input::KeyIdentifier::KI_F3;
			break;
		case KEY_F4:
			key_id = Input::KeyIdentifier::KI_F4;
			break;
		case KEY_F5:
			key_id = Input::KeyIdentifier::KI_F5;
			break;
		case KEY_F6:
			key_id = Input::KeyIdentifier::KI_F6;
			break;
		case KEY_F7:
			key_id = Input::KeyIdentifier::KI_F7;
			break;
		case KEY_F8:
			key_id = Input::KeyIdentifier::KI_F8;
			break;
		case KEY_F9:
			key_id = Input::KeyIdentifier::KI_F9;
			break;
		case KEY_F10:
			key_id = Input::KeyIdentifier::KI_F10;
			break;
		case KEY_F11:
			key_id = Input::KeyIdentifier::KI_F11;
			break;
		case KEY_F12:
			key_id = Input::KeyIdentifier::KI_F12;
			break;
		case KEY_LEFT_SHIFT:
			key_id = Input::KeyIdentifier::KI_LSHIFT;
			break;
		case KEY_LEFT_CONTROL:
			key_id = Input::KeyIdentifier::KI_LCONTROL;
			break;
		case KEY_LEFT_ALT:
			key_id = Input::KeyIdentifier::KI_LMETA;
			break;
		case KEY_LEFT_SUPER:
			key_id = Input::KeyIdentifier::KI_LWIN;	 // Left Windows key.
			break;
		case KEY_RIGHT_SHIFT:
			key_id = Input::KeyIdentifier::KI_RSHIFT;
			break;
		case KEY_RIGHT_CONTROL:
			key_id = Input::KeyIdentifier::KI_RCONTROL;
			break;
		case KEY_RIGHT_ALT:
			key_id = Input::KeyIdentifier::KI_RMETA;
			break;
		case KEY_RIGHT_SUPER:
			key_id = Input::KeyIdentifier::KI_RWIN;	 // Right Windows key.
			break;
		case KEY_KB_MENU:
			key_id = Input::KeyIdentifier::KI_RMENU;
			break;
		case KEY_KP_0:
			key_id = Input::KeyIdentifier::KI_NUMPAD0;
			break;
		case KEY_KP_1:
			key_id = Input::KeyIdentifier::KI_NUMPAD1;
			break;
		case KEY_KP_2:
			key_id = Input::KeyIdentifier::KI_NUMPAD2;
			break;
		case KEY_KP_3:
			key_id = Input::KeyIdentifier::KI_NUMPAD3;
			break;
		case KEY_KP_4:
			key_id = Input::KeyIdentifier::KI_NUMPAD4;
			break;
		case KEY_KP_5:
			key_id = Input::KeyIdentifier::KI_NUMPAD5;
			break;
		case KEY_KP_6:
			key_id = Input::KeyIdentifier::KI_NUMPAD6;
			break;
		case KEY_KP_7:
			key_id = Input::KeyIdentifier::KI_NUMPAD7;
			break;
		case KEY_KP_8:
			key_id = Input::KeyIdentifier::KI_NUMPAD8;
			break;
		case KEY_KP_9:
			key_id = Input::KeyIdentifier::KI_NUMPAD9;
			break;
		case KEY_KP_DECIMAL:
			key_id = Input::KeyIdentifier::KI_DECIMAL;	// Period on the numeric keypad.
			break;
		case KEY_KP_DIVIDE:
			key_id = Input::KeyIdentifier::KI_DIVIDE;  // Forward Slash on the numeric keypad.
			break;
		case KEY_KP_MULTIPLY:
			key_id = Input::KeyIdentifier::KI_MULTIPLY;	 // Asterisk on the numeric keypad.
			break;
		case KEY_KP_SUBTRACT:
			key_id = Input::KeyIdentifier::KI_SUBTRACT;	 // Minus on the numeric keypad.
			break;
		case KEY_KP_ADD:
			key_id = Input::KeyIdentifier::KI_ADD;	// Plus on the numeric keypad.
			break;
		case KEY_KP_ENTER:
			key_id = Input::KeyIdentifier::KI_NUMPADENTER;
			break;
		case KEY_KP_EQUAL:
			key_id = Input::KeyIdentifier::KI_OEM_NEC_EQUAL;  // Equals key on the numeric keypad.
			break;
		case KEY_BACK:
			key_id = Input::KeyIdentifier::KI_BROWSER_BACK;
			break;
		case KEY_VOLUME_UP:
			key_id = Input::KeyIdentifier::KI_VOLUME_UP;
			break;
		case KEY_VOLUME_DOWN:
			key_id = Input::KeyIdentifier::KI_VOLUME_DOWN;
			break;
	}

	return key_id;
}
