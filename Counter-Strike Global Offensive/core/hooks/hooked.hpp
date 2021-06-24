#pragma once

#include "source.hpp"

namespace F
{
	extern vgui::HFont Menu;
	extern vgui::HFont MenuV2;
	extern vgui::HFont MenuV2i;
	extern vgui::HFont MenuV2n;
	extern vgui::HFont ESPInfo;
	extern vgui::HFont ESP;
	extern vgui::HFont LBY;
	extern vgui::HFont Events;
	extern vgui::HFont Icons;
	extern vgui::HFont MenuIcons;
	extern vgui::HFont ESPFont;
	extern vgui::HFont OtherVisualsFont;
	extern vgui::HFont GrenadeWarningFont;
}

namespace Drawing
{
	extern void CreateFonts();
	extern void Texture(const int32_t x, const int32_t y, const int32_t w, const int32_t h, const int texture, const Color tint);
	extern void LimitDrawingArea(int x, int y, int w, int h);
	extern void GetDrawingArea(int& x, int& y, int& w, int& h);
	extern void ColorPicker(Vector2D pos, int width, int height, float alpha = 255.f);
	extern void DrawString(vgui::HFont font, int x, int y, Color color, DWORD alignment, const char* msg, ...);
	extern void DrawStringFont(vgui::HFont font, int x, int y, Color clrColor, bool bCenter, const char* szText, ...);
	extern void DrawStringUnicode(vgui::HFont font, int x, int y, Color color, bool bCenter, const wchar_t* msg, ...);
	extern void DrawRect(int x, int y, int w, int h, Color col);
	extern void DrawRectA(int x, int y, int w, int h, Color col);
	extern void Rectangle(float x, float y, float w, float h, float px, Color col);
	extern void Border(int x, int y, int w, int h, int line, Color col);
	extern void DrawRectRainbow(int x, int y, int w, int h, float flSpeed, float& flRainbow);
	extern void DrawRectGradientVertical(int x, int y, int w, int h, Color color1, Color color2);
	extern void DrawRectGradientVerticalA(int x, int y, int width, int height, Color color1, Color color2);
	extern void DrawRectGradientHorizontal(int x, int y, int w, int h, Color color1, Color color2);
	extern void DrawPixel(int x, int y, Color col);
	extern void DrawOutlinedRect(int x, int y, int w, int h, Color col);
	extern void DrawOutlinedRectA(int x, int y, int w, int h, Color col);
	extern void DrawOutlinedCircle(int x, int y, int r, Color col);
	extern void DrawLine(int x0, int y0, int x1, int y1, Color col, bool shadow = false);
	extern void DrawCircularProgressBar(Vector2D pos, int r1, int r2, int s, int d, Color color);
	extern void DrawCorner(int iX, int iY, int iWidth, int iHeight, bool bRight, bool bDown, Color colDraw);
	extern void DrawRoundedBox(int x, int y, int w, int h, int r, int v, Color col);
	extern void Triangle(Vector ldcorner, Vector rucorner, Color col);
	extern void DrawPolygon(int count, Vertex_t* Vertexs, Color color);
	extern void DrawBox(int x, int y, int w, int h, Color color);
	extern bool ScreenTransform(const Vector& point, Vector& screen);
	extern bool WorldToScreen(const Vector& origin, Vector& screen);
	extern RECT GetViewport();
	extern int	GetStringWidth(vgui::HFont font, const char* msg, ...);
	extern RECT GetTextSize(vgui::HFont font, const char* text);
	extern RECT GetTextSize(vgui::HFont font, const wchar_t* text);
	extern void Draw3DBox(Vector* boxVectors, Color color);
	extern void rotate_point(Vector2D& point, Vector2D origin, bool clockwise, float angle);
	extern void DrawFilledCircle(int x, int y, int radius, int segments, Color color);
	extern void TexturedPolygon(int n, std::vector<Vertex_t> vertice, Color color);
	extern void DrawArc(float x, float y, float radius, float min_angle, float max_angle, Color col, float thickness);
	extern void filled_tilted_triangle(Vector2D position, Vector2D size, Vector2D origin, bool clockwise, float angle, Color color, bool rotate = true, Color col2 = Color::Black());
	extern void DrawCircle(float x, float y, float r, float s, Color color);
	extern void Draw3DFilledCircle(const Vector& origin, float radius, Color color);
	extern void triangle(Vector2D point_one, Vector2D point_two, Vector2D point_three, Color color);

	extern const char* LastFontName;
}

using LevelInitPostEntity_t        = void(__thiscall*)(void*);
using LevelShutdown_t              = void(__thiscall*)(void*);
using EmitSound_t					= int(__thiscall*)(void*, IRecipientFilter&, int, int, const char*, unsigned int, const char*, float, int, float, int, int, const Vector*, const Vector*, void*, bool, float, int, int);
using LevelInitPreEntity_t         = void(__thiscall*)(void*, const char*);

namespace Hooked
{

	void __fastcall  PacketEnd(pPastaState* cl_state, void* EDX);
	void __fastcall  FrameStageNotify(void* ecx, void* edx, ClientFrameStage_t stage);
	void __fastcall  ProcessMovement(void* ecx, void* edx, C_BasePlayer* basePlayer, CMoveData* moveData);
	void __fastcall  RunCommand(void* ecx, void* edx, C_BasePlayer* player, CUserCmd* ucmd, IMoveHelper* moveHelper);
	void __fastcall  OverrideView(void* ecx, void* edx, CViewSetup* vsView);
	void __fastcall  SetupMove(void* player, int edx, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move);
	void __fastcall  LevelInitPreEntity(void* ecx, void* edx, const char* map);
	void __fastcall  PacketStart(void* ecx, void* edx, int incoming_sequence, int outgoing_acknowledged);
	void __fastcall  LevelInitPostEntity(void* ecx, void* edx);
	void __fastcall  LevelShutdown(void* ecx, void* edx);
	void __fastcall  Shutdown(INetChannel* pNetChan, void* EDX, const char* reason);
	//void __fastcall  ProcessPacket(void* ecx, void* edx, bool header);
	void __fastcall  PaintTraverse(void* ecx, void* edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce);
	void __fastcall  SceneEnd(void* ecx, void* edx);

	void _stdcall    EngineVGUI_Paint(int mode);
	void _stdcall    DrawModelExecute(void* context, void* state, const ModelRenderInfo_t& info, matrix3x4_t* pCustomBoneToWorld);
	void __stdcall   LockCursor();

	bool __fastcall  ProcessTempEntities(pPastaState* cl_state, void* EDX, void* msg);
	bool __fastcall  InPrediction(void* ecx, void* edx);
	bool __fastcall  SendNetMsg(void* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice);
	void __fastcall  ProcessPacketNet(INetChannel* channel, uint32_t, void* packet, bool header);
	bool __fastcall  DoPostScreenEffects(void* clientmode, void*, int a1);
	bool __stdcall   CreateMove(float input_sample_frametime, CUserCmd* cmd);

	int __fastcall   SendDatagram(void* netchan, void*, void* datagram);
	int __fastcall   AddRenderable(void* ecx, void* edx, IClientRenderable* p_renderable, int unk1, RenderableTranslucencyType_t n_type, int unk2, int unk3);
	int __fastcall   ListLeavesInBox(void* bsp, void* edx, Vector& mins, Vector& maxs, unsigned short* pList, int listMax);
	int __fastcall   EmitSound1(void* _this, int edx, IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, int nSeed, float flAttenuation, int iFlags, int iPitch, const Vector* pOrigin, const Vector* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity, int unk);
	
	float __fastcall GetViewModelFOV(void* a1, int ecx);

	long __stdcall   EndScene(IDirect3DDevice9* device);
	long __stdcall   Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pp);

	extern WNDPROC oldWindowProc;
	extern LRESULT __stdcall WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	extern void skinfix(int itemfix);
}


#define CREATE_EVENT_LISTENER(class_name)\
class class_name : public IGameEventListener\
{\
public:\
	~class_name() { csgo.m_event_manager()->RemoveListener(this); }\
\
	virtual void FireGameEvent(IGameEvent* game_event);\
};\

namespace game_events //fuck namespaces, fuck ur style bolbi
{
	void init();

	CREATE_EVENT_LISTENER( PlayerHurtListener );
	CREATE_EVENT_LISTENER( BulletImpactListener );
	CREATE_EVENT_LISTENER( PlayerDeathListener );
	CREATE_EVENT_LISTENER( RoundStartListener );
	CREATE_EVENT_LISTENER( RoundEndListener );
	CREATE_EVENT_LISTENER( PurchaseListener );
	CREATE_EVENT_LISTENER( BombPlantListener );
	CREATE_EVENT_LISTENER( WeaponFireListener );
	CREATE_EVENT_LISTENER( BombDefuseListener );
	CREATE_EVENT_LISTENER( BombStopPlantListener );
	CREATE_EVENT_LISTENER( BombStopDefuseListener );
	CREATE_EVENT_LISTENER( BombStartPlantListener );
	CREATE_EVENT_LISTENER( BombStartDefuseListener );
};