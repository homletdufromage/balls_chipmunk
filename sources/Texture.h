#pragma once
#include <string>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

class Texture {
	public:
		// Initialise les variables
		Texture();

		// Désalloue la mémoire
		~Texture();

      bool initFont(std::string fontFile);

		//Creates image from font string
      bool loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* renderer);

		// Désalloue la mémoire de la texture
		void free();

		// Change le modificateur de couleur
		void setColor(Uint8 red, Uint8 green, Uint8 blue);

		//Set blending
      void setBlendMode(SDL_BlendMode blending);

		// Affiche la texture à certaines coordonnées donné
		void render(SDL_Renderer* renderer);

		// Accesseurs de la texture
		int getWidth();
		int getHeight();

      void setPosition(int x, int y) { _x = x; _y = y; }

	private:
      TTF_Font* _font;
		SDL_Texture* _texture;

		// Dimensions de l'image
		int _width, _height;
      int _x, _y;
};