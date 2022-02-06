#include "Engine.h"

#include "SDL2/SDL_vulkan.h"

#include "File.h"
#include "Common.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"

Engine::Engine()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) < 0)
	{
		return;
	}

	Window = SDL_CreateWindow(
		"Hello World",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		1280, 720,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
	);

	if (Window == nullptr)
	{
		return;
	}

	NewDevice = new VulkanDevice();
	NewDevice->Initialize(Window);

	NewShader = VulkanShader::CreateFromSPIRV(File::ReadAllBytes("data/vertex.spv"), File::ReadAllBytes("data/fragment.spv"));

	std::vector<VertexAttribute> attribz =
	{
		{ AttributeType::Float2, 2, sizeof(float) * 2, 0 },
		{ AttributeType::Float3, 3, sizeof(float) * 3, 2 * sizeof(float) }
	};
	NewPipeline = VulkanPipeline::Create(NewDevice, NewShader, attribz, sizeof(Vertex));

	Vb = VulkanBuffer::Create(NewDevice, BufferType::Vertex, vertices.data(), vertices.size() * sizeof(Engine::Vertex));
	Ib = VulkanBuffer::Create(NewDevice, BufferType::Index, indices.data(), indices.size() * sizeof(uint16_t));

	std::cout << "Main loop started\n";

	SDL_Event event;
	while (true)
	{
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				break;
			}
			else if (event.type == SDL_WINDOWEVENT)
			{
				if (event.window.event == SDL_WINDOWEVENT_RESIZED)
				{
					std::cout << "resize \n";
					winWidth = event.window.data1;
					winHeight = event.window.data2;
				}
			}
		}

		Render();
	}

	// TODO: !!! do this elsewhere ???
	vkDeviceWaitIdle(NewDevice->Device);

	Cleanup();

	SDL_DestroyWindow(Window);
	SDL_Quit();
}

void Engine::Initialize()
{
}

void Engine::Cleanup()
{
}

void Engine::Render()
{
	NewDevice->BeginFrame(Vb->Buffer, Ib->Buffer, indices.size(), NewPipeline);

	NewDevice->BindVertexBuffer(Vb);
	NewDevice->BindIndexBuffer(Ib);

	NewDevice->DrawIndexed(indices.size());

	NewDevice->Present();
}
