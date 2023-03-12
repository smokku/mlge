#include <RmlUi/Core.h>
// #include <RmlUi/Debugger.h>

#include <cassert>
#include <raylib-cpp.hpp>

#include "rml.h"

int main()
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

	// RmlUi initialisation.
	Rml::Initialise();

	// Create the main RmlUi context.
	Rml::Context *context = Rml::CreateContext("main", Rml::Vector2i(screenWidth, screenHeight));
	assert(context);

	// Rml::Debugger::Initialise(context);

	// Fonts should be loaded before any documents are loaded.
	Rml::LoadFontFace("client/assets/PressStart2P-vaV7.ttf");

	// Load and show the document.
	Rml::ElementDocument *document = context->LoadDocument("client/data/tutorial.rml");
	assert(document);
	document->Show();

	// Main game loop
	while (!window.ShouldClose()) {	 // Detect window close button or ESC key

		// Submit input events before the call to Context::Update().
		auto mouse_delta = GetMouseDelta();
		if (mouse_delta.x || mouse_delta.y) {
			context->ProcessMouseMove(GetMouseX(), GetMouseY(), 0);
		}

		// Toggle the debugger with a key binding.
		if (IsKeyPressed(KEY_F8)) {
			// Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
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

			textColor.DrawText("All your codebase are belong to us", 200, 200, 20);

			// Set up any rendering states necessary before the render.
			// my_renderer->PrepareRenderBuffer();

			// Render the user interface on top of the application.
			context->Render();

			// Present the rendered frame.
			// my_renderer->PresentRenderBuffer();
		}
		EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// Shutdown RmlUi.
	Rml::Shutdown();
	// It is now safe to destroy the custom interfaces previously passed to RmlUi.

	return 0;
}
