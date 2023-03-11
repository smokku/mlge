#include <RmlUi/Core.h>
// #include <RmlUi/Debugger.h>
#include <raylib.h>

#include <cassert>
#include <raylib-cpp.hpp>

class MyRenderInterface : public Rml::RenderInterface
{
   public:
	MyRenderInterface() {}
	virtual ~MyRenderInterface() = default;

	void RenderGeometry(Rml::Vertex *vertices, int num_vertices, int *indices,
						int num_indices, Rml::TextureHandle texture,
						const Rml::Vector2f &translation) override;
	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(int x, int y, int width, int height) override;
};

class MySystemInterface : public Rml::SystemInterface
{
   public:
	MySystemInterface() {}
	virtual ~MySystemInterface() = default;

	double GetElapsedTime() override;
	bool   LogMessage(Rml::Log::Type type, const Rml::String &message) override;
};

void MyRenderInterface::RenderGeometry(Rml::Vertex *vertices, int num_vertices,
									   int *indices, int num_indices,
									   Rml::TextureHandle	texture,
									   const Rml::Vector2f &translation) {}
void MyRenderInterface::EnableScissorRegion(bool enable) {}
void MyRenderInterface::SetScissorRegion(int x, int y, int width, int height) {}

double MySystemInterface::GetElapsedTime() { return GetTime(); }

bool MySystemInterface::LogMessage(Rml::Log::Type	  type,
								   const Rml::String &message)
{
	TraceLogLevel level = LOG_FATAL;
	switch (type) {
		case Rml::Log::LT_ALWAYS:
			level = LOG_ALL;
			break;
		case Rml::Log::LT_ERROR:
			level = LOG_ERROR;
			break;
		case Rml::Log::LT_ASSERT:
			level = LOG_FATAL;
			break;
		case Rml::Log::LT_WARNING:
			level = LOG_WARNING;
			break;
		case Rml::Log::LT_INFO:
			level = LOG_INFO;
			break;
		case Rml::Log::LT_DEBUG:
			level = LOG_DEBUG;
			break;
		case Rml::Log::LT_MAX:
			level = LOG_TRACE;
			break;
	}
	TraceLog(level, "RMLUI: %s", message.c_str());
	return true;
}

//--------------------------------------------------------------------------------------

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

	// Instantiate the interfaces to RmlUi.
	MyRenderInterface render_interface;
	MySystemInterface system_interface;

	// Install the custom interfaces.
	Rml::SetRenderInterface(&render_interface);
	Rml::SetSystemInterface(&system_interface);

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
