#include "DebugWindow.hpp"


int main(){
	DebugWindow window(5, 3);
	while(true)
	{
		SDL_Event event;
		SDL_PollEvent(&event);
		if(event.type == SDL_QUIT)
			return 0;
		tpm_mono_buf buffer = nullptr;
		window.displayBuf(buffer);
	}	
}