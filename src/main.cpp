#include <SFML/Graphics.hpp>
#include <cstdio>
#include <fstream>
#include <string>

sf::Texture noise_texture(const sf::Vector2u& size) {
	sf::Image img;

	img.create(size.x, size.y, sf::Color::Black);
		
	for (unsigned int y = 0; y < size.y; y++) {
		for (unsigned int x = 0; x < size.x; x++) {
			sf::Color c(
				std::rand() % 256,
				std::rand() % 256,
				std::rand() % 256
			);
			img.setPixel(x, y, c);
		}
	}

	sf::Texture tex;
	tex.loadFromImage(img);
	return tex;
}

sf::Texture noise_texture_from(const sf::Texture& tex) {
    sf::Image img = tex.copyToImage();

    for (unsigned int y = 0; y < img.getSize().y; y++) {
        for (unsigned int x = 0; x < img.getSize().x; x++) {
            sf::Color c(
                std::rand() % 256,
                std::rand() % 256,
                std::rand() % 256
            );
			sf::Color av_c = sf::Color(
                (img.getPixel(x, y).r + c.r) / 2,
                (img.getPixel(x, y).g + c.g) / 2,
                (img.getPixel(x, y).b + c.b) / 2 			
			);
            img.setPixel(x, y, av_c);
        }
    }

    sf::Texture tex_new;
    tex_new.loadFromImage(img);
    return tex_new;
}

sf::Texture imagesrc_to_texture(const std::string& path) {
	std::ifstream f(path, std::ios::binary);
	if (!f.is_open()) std::exit(-1);

	std::string magic;
	f >> magic;
	if (magic != "P6") std::exit(-1);

	int w, h, max;
	f >> w >> h >> max;
	f.ignore(1);

	sf::Image img;
	img.create(w, h);

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			sf::Uint8 rgb[3];
			f.read(reinterpret_cast<char*>(rgb), 3);
			img.setPixel(x, y, sf::Color(rgb[0], rgb[1], rgb[2]));
		}
	}

	sf::Texture tex;
	tex.loadFromImage(img);
	return tex;
}

void download_ppm(const sf::Texture& tex, const std::string& path) {
	sf::Image img = tex.copyToImage();
	unsigned int w = img.getSize().x;
	unsigned int h = img.getSize().y;

	std::ofstream f(path, std::ios::binary);
	if (!f.is_open()) std::exit(-1);

	f << "P6\n" << w << " " << h << "\n255\n";

	for (unsigned int y = 0; y < h; y++) {
		for (unsigned int x = 0; x < w; x++) {
			sf::Color c = img.getPixel(x, y);
			f.write(reinterpret_cast<char*>(&c), 3);
		}
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		std::printf("usage: noise <iterations>\n");
		return -1;
	}

	int iters = std::atoi(argv[1]);

	sf::Texture in_texture = imagesrc_to_texture("bin/sample_img/negro.ppm");

	sf::Vector2u size = in_texture.getSize();

	sf::Texture out_texture_old = noise_texture(size);
	sf::Texture out_texture_new;

	sf::RenderTexture render_texture;
	render_texture.create(in_texture.getSize().x, in_texture.getSize().y);

	sf::Shader comparer;
	if (!comparer.loadFromFile("shaders/comparer.glsl", sf::Shader::Fragment)) {
		std::fprintf(stderr, "failed to load shader\n");
		return -1;
	}

	for (int iter = 0; iter < iters; iter++) {
		out_texture_new = noise_texture_from(out_texture_old);

		comparer.setUniform("old_texture", out_texture_old);
		comparer.setUniform("new_texture", out_texture_new);
		comparer.setUniform("in_texture", in_texture);

		sf::Sprite sprite;
		sprite.setTexture(out_texture_old);
		render_texture.clear();
		render_texture.draw(sprite, &comparer);
		render_texture.display();

		out_texture_old = render_texture.getTexture();
	}

	download_ppm(out_texture_old, "bin/result_img/0-" + std::to_string(iters) + ".ppm");
}
