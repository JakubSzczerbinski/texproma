#include "../libtexproma/libtexproma.h"
#include "Logger.hpp"
#include <SDL.h>
#include <stdexcept>

class DebugWindow
{
public:
	DebugWindow(unsigned rows, unsigned cols)
	{
		if (SDL_Init(SDL_INIT_VIDEO) < 0) {
			std::string error = SDL_GetError();
	    	SDL_log << "Couldn't initialize SDL: " << error << '\n';
	 		throw std::runtime_error(error);
	 	}

	 	unsigned windowWidth = rows * TP_WIDTH;
	 	unsigned windowHeight = cols * TP_HEIGHT;

	 	window_ = SDL_CreateWindow(
	 		"Debug Window",
	 		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
	                            windowWidth, windowHeight, 0);

	 	if(window_ == nullptr)
	 	{
	 		std::string error = SDL_GetError();
	 		SDL_log << "Couldn't create window: " << error << '\n';
	 		throw std::runtime_error(error);
	 	}

	 	renderer_ = SDL_CreateRenderer(window_, -1,
	                                SDL_RENDERER_SOFTWARE);
	  	texture_ = SDL_CreateTexture(renderer_, 
	                              SDL_PIXELFORMAT_RGBA8888,
	                              SDL_TEXTUREACCESS_STREAMING,
	                              TP_WIDTH, TP_HEIGHT);
	}
	void displayBuf(tpm_mono_buf buffer)
	{
		
	}
	void displayBuf(tpm_color_buf buffer);
private:
	Logger SDL_log{"SDL"};
	SDL_Window* window_;
	SDL_Renderer* renderer_;
	SDL_Texture* texture_;
};