#include "DebugWindow.hpp"
extern "C"{
	#include <libtexproma.h>
}
int main(){

	DebugWindow window(5, 3);
	while(true)
	{
		SDL_Event event;
		SDL_PollEvent(&event);
		if(event.type == SDL_QUIT)
			return 0;
		tpm_mono_buf buffer = (tpm_mono_buf)malloc(256*256);
		tpm_sine(buffer, 10.0);
		window.displayBuf(buffer, 0, 0);
	}	
}