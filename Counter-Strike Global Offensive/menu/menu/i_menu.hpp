#pragma once
#include "sdk.hpp"
#include <features/inventory/parser.h>
#include <configs/configs.h>
namespace menu
{
	extern LPDIRECT3DTEXTURE9 flag_ewropi;
	extern LPDIRECT3DTEXTURE9 m_tplayer_with_glow;
	extern LPDIRECT3DTEXTURE9 m_tplayer_no_glow;
	extern IDirect3DDevice9* m_device;

	extern int category;
	extern int new_category;


	extern void init(const float& alpha = 1.f);
	extern void draw(IDirect3DDevice9* device);
	extern void save_config();
	extern void load_config();
	extern IDirect3DTexture9* m_pTexture;
}