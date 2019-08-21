#include "Texture.h"

Texture::Texture() {
	_texture = NULL;
   _font = NULL;
	_width = 0;
	_height = 0;
   _x = _y = 0;
}

Texture::~Texture() {
	free();
}

bool Texture::initFont(std::string fontFile) {
   _font = TTF_OpenFont(fontFile.c_str(), 20);
   if (!_font) {
      printf("Can't load font. Error : %s\n", TTF_GetError());
      return false;
   }
   return true;
}

bool Texture::loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* renderer) {
	// Get rid of another loaded text if there is one
	free();

	SDL_Surface* textSurface = TTF_RenderText_Blended(_font, textureText.c_str(), textColor);
	if (!textSurface) {
		printf("Could not render text surface. SDL_ttf error : %s\n", TTF_GetError());
	} else {
		_texture = SDL_CreateTextureFromSurface(renderer, textSurface);
		if (_texture == NULL) {
			printf("Unable to create texture from rendered text. Error : %s\n", SDL_GetError());
		} else {
			_width = textSurface->w;
			_height = textSurface->h;
		}

		SDL_FreeSurface(textSurface);
	}

	return _texture != NULL;
}

void Texture::free() {
	if(_texture) {
		SDL_DestroyTexture(_texture);
		_texture = NULL;
		_width = 0;
		_height = 0;
	}
}

void Texture::setColor(Uint8 red, Uint8 green, Uint8 blue) {
	SDL_SetTextureColorMod(_texture, red, green, blue);
}

void Texture::setBlendMode(SDL_BlendMode blending) {
	SDL_SetTextureBlendMode(_texture, blending);
}

void Texture::render(SDL_Renderer* renderer) {
	//Set rendering space and render to the screen
	SDL_Rect renderQuad = { _x, _y, _width, _height };

	// C'est ici qu'on choisit "clip" comme "Rect" de zone
	SDL_RenderCopyEx(renderer, _texture, NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
}

int Texture::getWidth() {
	return _width;
}

int Texture::getHeight() {
	return _height;
}