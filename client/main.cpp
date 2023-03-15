#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

#include <cassert>
#include <raylib-cpp.hpp>

#include "physfs.h"
#include "rml.h"

unsigned char *load_file_data(const char *fileName, unsigned int *bytesRead)
{
	auto		   file		   = PHYSFS_openRead(fileName);
	auto		   buffer_size = PHYSFS_fileLength(file);
	unsigned char *buffer	   = static_cast<unsigned char *>(malloc(buffer_size));
	*bytesRead				   = PHYSFS_readBytes(file, buffer, buffer_size);
	return buffer;
}

int main(int, char *argv[])
{
	// Initialization
	SetTraceLogLevel(LOG_DEBUG);

	//--------------------------------------------------------------------------------------
	int			   screenWidth	= 800;
	int			   screenHeight = 480;
	raylib::Color  textColor	= raylib::Color::RayWhite();
	raylib::Window window(screenWidth, screenHeight, "MLGE");
	// SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	SetTargetFPS(60);
	//--------------------------------------------------------------------------------------

	// Install the custom interfaces.
	GameSystemInterface system_interface;
	Rml::SetSystemInterface(&system_interface);
	GameRenderInterface render_interface;
	Rml::SetRenderInterface(&render_interface);
	GameFileInterface file_interface(argv);
	Rml::SetFileInterface(&file_interface);
	SetLoadFileDataCallback(load_file_data);

	file_interface.mount("resources");

	// RmlUi initialisation.
	Rml::Initialise();

	// Create the main RmlUi context.
	Rml::Context *context = Rml::CreateContext("main", Rml::Vector2i(screenWidth, screenHeight));
	assert(context);

	Rml::Debugger::Initialise(context);

	// Fonts should be loaded before any documents are loaded.
	Rml::LoadFontFace("assets/PressStart2P-vaV7.ttf");

	// Load and show the document.
	Rml::ElementDocument *document = context->LoadDocument("data/tutorial.rml");
	assert(document);
	document->Show();

	// Main game loop
	while (!window.ShouldClose()) {	 // Detect window close button or ESC key

		// Submit input events before the call to Context::Update().

		while (int key = GetCharPressed()) {
			context->ProcessTextInput(key);
		}

		int key_modifier_state = 0;
		if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) key_modifier_state |= Rml::Input::KeyModifier::KM_CTRL;
		if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) key_modifier_state |= Rml::Input::KeyModifier::KM_SHIFT;
		if (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) key_modifier_state |= Rml::Input::KeyModifier::KM_ALT;
		if (IsKeyDown(KEY_LEFT_SUPER) || IsKeyDown(KEY_RIGHT_SUPER)) key_modifier_state |= Rml::Input::KeyModifier::KM_META;
		if (IsKeyDown(KEY_CAPS_LOCK)) key_modifier_state |= Rml::Input::KeyModifier::KM_CAPSLOCK;
		if (IsKeyDown(KEY_NUM_LOCK)) key_modifier_state |= Rml::Input::KeyModifier::KM_NUMLOCK;
		if (IsKeyDown(KEY_SCROLL_LOCK)) key_modifier_state |= Rml::Input::KeyModifier::KM_SCROLLLOCK;

		while (int key = GetKeyPressed()) {
			context->ProcessKeyDown(raylib_key_to_identifier(static_cast<KeyboardKey>(key)), key_modifier_state);
		}

		auto mouse_delta = GetMouseDelta();
		if (mouse_delta.x || mouse_delta.y) {
			context->ProcessMouseMove(GetMouseX(), GetMouseY(), key_modifier_state);
		}

		for (const int button : {0, 1, 2}) {
			if (IsMouseButtonPressed(button)) {
				context->ProcessMouseButtonDown(button, key_modifier_state);
			}
			if (IsMouseButtonReleased(button)) {
				context->ProcessMouseButtonUp(button, key_modifier_state);
			}
		}

		context->ProcessMouseWheel(GetMouseWheelMove(), key_modifier_state);

		// Toggle the debugger with a key binding.
		if (IsKeyPressed(KEY_F8)) {
			Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
		}

		// Update
		//----------------------------------------------------------------------------------
		// Update your variables here
		//----------------------------------------------------------------------------------

		// Update any elements to reflect changed data.
		// if (Rml::Element *el = document->GetElementById("score"))
		//   el->SetInnerRML("Current score: " + my_application->GetScoreAsString());

		// Update the context to reflect any changes resulting from
		// input events, animations, modified and added elements, or
		// changed data in data bindings.
		context->Update();

		// Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();
		{
			window.ClearBackground(BLACK);
			DrawFPS(10, 10);

			textColor.DrawText("All your codebase are belong to us", 216, 200, 20);

			// Set up any rendering states necessary before the render.
			render_interface.BeginFrame();
			// Render the user interface on top of the application.
			context->Render();
			// Present the rendered frame.
			render_interface.EndFrame();
		}
		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// Shutdown RmlUi.
	Rml::Shutdown();
	// It is now safe to destroy the custom interfaces previously passed to RmlUi.

	return 0;
}
