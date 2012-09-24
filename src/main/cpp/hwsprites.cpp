#include "hwsprites.hpp"

hwsprites::hwsprites()
{
}


hwsprites::~hwsprites()
{
}

void hwsprites::init(const uint8_t* src_sprites)
{
    // Clear Sprite RAM buffers
	for (uint16_t i = 0; i < SPRITE_RAM_SIZE; i++)
	{
		ram[i] = 0;
		ramBuff[i] = 0;
	}

    // Convert S16 tiles to a more useable format
	const uint8_t *spr = src_sprites;

	for (uint32_t i = 0; i < SPRITES_LENGTH; i++)
	{
		uint8_t d3 = *spr++;
		uint8_t d2 = *spr++;
		uint8_t d1 = *spr++;
		uint8_t d0 = *spr++;

		sprites[i] = (d0 << 24) | (d1 << 16) | (d2 << 8) | d3;
	}
}

uint8_t hwsprites::read(const uint16_t adr)
{
	uint16_t a = adr >> 1;
	if ((adr & 1) == 1)
        return ram[a] & 0xff;
	else
        return ram[a] >> 8;
}

void hwsprites::write(const uint16_t adr, const uint16_t data)
{
    ram[adr >> 1] = data;
}

// Copy back buffer to main ram, ready for blit
void hwsprites::swap()
{
	uint16_t *src = (uint16_t *)ram;
	uint16_t *dst = (uint16_t *)ramBuff;

	// swap the halves of the road RAM
	for (uint16_t i = 0; i < SPRITE_RAM_SIZE; i++)
	{
		uint16_t temp = *src;
		*src++ = *dst;
		*dst++ = temp;
	}
}

void hwsprites::render(const uint8_t priority)
{
	uint32_t numbanks = SPRITES_LENGTH / 0x10000;

	for (uint16_t data = 0; data < SPRITE_RAM_SIZE; data += 8) 
	{
		if ((ramBuff[data+0] & 0x8000) != 0) break;

		uint32_t sprpri  = 1 << ((ramBuff[data+3] >> 12) & 3);

		if (sprpri != priority) continue;

		int16_t hide    = (ramBuff[data+0] & 0x5000);
		int16_t bank    = (ramBuff[data+0] >> 9) & 7;
		int32_t top     = (ramBuff[data+0] & 0x1ff) - 0x100;
		uint32_t addr    = ramBuff[data+1];
		int32_t pitch  = ((ramBuff[data+2] >> 1) | ((ramBuff[data+4] & 0x1000) << 3)) >> 8;
		int32_t xpos     = ramBuff[data+2] & 0x1ff;
		uint8_t shadow  = (ramBuff[data+3] >> 14) & 1;
		int32_t vzoom    = ramBuff[data+3] & 0x7ff;
		int32_t ydelta = ((ramBuff[data+4] & 0x8000) != 0) ? 1 : -1;
		int32_t flip   = (~ramBuff[data+4] >> 14) & 1;
		int32_t xdelta = ((ramBuff[data+4] & 0x2000) != 0) ? 1 : -1;
		int32_t hzoom    = ramBuff[data+4] & 0x7ff;
		int32_t height  = (ramBuff[data+5] >> 8) + 1;
		int32_t color   = (ramBuff[data+5] & 0x7f) << 4;
		int32_t x, y, ytarget, yacc = 0, pix;

		// adjust X coordinate
		// note: the threshhold below is a guess. If it is too high, rachero will draw garbage
		// If it is too low, smgp won't draw the bottom part of the road
		if (xpos < 0x80 && xdelta < 0)
			xpos += 0x200;
		xpos -= 0xbe;

		// initialize the end address to the start address
		ramBuff[data+7] = addr;

		// if hidden, or top greater than/equal to bottom, or invalid bank, punt
		if (hide != 0 || height == 0)
			continue;

		// clamp to within the memory region size
		if (numbanks != 0)
			bank %= numbanks;

		uint32_t spritedata = 0x10000 * bank;

		// clamp to a maximum of 8x (not 100% confirmed)
		if (vzoom < 0x40) vzoom = 0x40;
		if (hzoom < 0x40) hzoom = 0x40;

		// loop from top to bottom
		ytarget = top + ydelta * height;
		for (y = top; y != ytarget; y += ydelta)
		{
			// skip drawing if not within the cliprect
			if (y >= 0 && y <= 223) {
				//unsigned short* pPixel = pTransDraw + (y * 320);
				uint32_t pPixel = (y * 320);

				int32_t xacc = 0;

				// non-flipped case
				if (flip == 0)
				{
					// start at the word before because we preincrement below
					ramBuff[data+7] = (addr - 1);

					for (x = xpos; (xdelta > 0 && x <= 319) || (xdelta < 0 && x >= 0); )
					{
						uint32_t pixels = sprites[spritedata + ++ramBuff[data+7]];

						// draw four pixels
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;

						// stop if the second-to-last pixel in the group was 0xf
						if ((pixels & 0x000000f0) == 0x000000f0)
							break;
					}
				}

				// flipped case
				else
				{
					// start at the word after because we predecrement below
					ramBuff[data+7] = (addr + 1);

					for (x = xpos; (xdelta > 0 && x <= 319) || (xdelta < 0 && x >= 0); )
					{
						uint32_t pixels = sprites[spritedata + --ramBuff[data+7]];

						// draw four pixels
						pix = (pixels >>  0) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  4) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >>  8) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 12) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 16) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 20) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 24) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;
						pix = (pixels >> 28) & 0xf; while (xacc < 0x200) { draw_pixel(x, pix, color, shadow, pPixel); x += xdelta; xacc += hzoom; } xacc -= 0x200;

						// stop if the second-to-last pixel in the group was 0xf
						if ((pixels & 0x0f000000) == 0x0f000000)
							break;
					}
				}
			}
			// accumulate zoom factors; if we carry into the high bit, skip an extra row
			yacc += vzoom;
			addr += pitch * (yacc >> 9);
			yacc &= 0x1ff;
		}
	}
}

void hwsprites::draw_pixel(const int32_t x, const uint16_t pix, const uint16_t colour, const uint8_t shadow, const uint32_t pPixel)
{
	if (x >= 0 && x <= 319 && pix != 0 && pix != 15)
	{
		if (shadow && pix == 0xa) 
		{     
			video.pixels[x + pPixel] &= 0xfff;
			uint16_t pal_entry = video.read_pal16(video.pixels[x + pPixel]) & 0x8000;
			video.pixels[x + pPixel] += (0x2000 - (pal_entry >> 3));
		} 
		else 
		{
			video.pixels[x + pPixel] = (pix | colour | COLOR_BASE) & 0xfff;
		}
	}  
}
