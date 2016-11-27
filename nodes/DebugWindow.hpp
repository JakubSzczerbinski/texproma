#include "Logger.hpp"
#include <SDL.h>
#include <stdexcept>
extern "C"{
	#include <libtexproma.h>
}

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
	void displayBuf(tpm_mono_buf buffer, unsigned x, unsigned y)
	{
		int posX = x * TP_WIDTH;
		int posY = y * TP_HEIGHT;
		SDL_Rect dst = {
			posX, posY,
			TP_WIDTH, TP_HEIGHT
		};
		RGBA *pixels;
		int pitch;
		SDL_LockTexture(texture_, NULL, (void **)&pixels, &pitch);
		for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
            uint8_t p = ((uint8_t *)buffer)[i];
            pixels[i] = (RGBA){p, p, p, 255};
        }
        SDL_UnlockTexture(texture_);
        SDL_RenderCopy(renderer_, texture_, NULL, &dst);
	}
	void displayBuf(tpm_color_buf buffer, unsigned x, unsigned y)
	{
		int posX = x * TP_WIDTH;
		int posY = y * TP_HEIGHT;
		SDL_Rect dst = {
			posX, posY,
			TP_WIDTH, TP_HEIGHT
		};
		RGBA *pixels;
		int pitch;
		SDL_LockTexture(texture_, NULL, (void **)&pixels, &pitch);
		for (int i = 0; i < TP_WIDTH * TP_HEIGHT; i++) {
            uint8_t r = ((uint8_t **)buffer)[0][i];
            uint8_t g = ((uint8_t **)buffer)[1][i];
            uint8_t b = ((uint8_t **)buffer)[2][i];
            pixels[i] = (RGBA){r, g, b, 255};
        }
        SDL_UnlockTexture(texture_);
        SDL_RenderCopy(renderer_, texture_, NULL, &dst);
	}
private:
	typedef struct { uint8_t a, b, g, r; } RGBA;
	Logger SDL_log{"SDL"};
	SDL_Window* window_;
	SDL_Renderer* renderer_;
	SDL_Texture* texture_;
};