#include "Resolver.h"
#include "..\Aimbot\Aimbot.h"
#include "..\Aimbot\Autowall.h"
#include "..\Aimbot\LagComp.h"
#include "..\..\Utils\Utils.h"
#include "..\..\SDK\IVEngineClient.h"
#include "..\..\SDK\Hitboxes.h"
#include "..\..\SDK\PlayerInfo.h"
#include "..\..\Utils\Math.h"
#include "..\..\Menu\Menu.h"

#define set_abs_yaw(x, y) x->SetAbsAngles(Vector(0, y, x->GetEyeAngles().z))
Resolver g_Resolver;
void Resolver::Resolve(ClientFrameStage_t stage)
{
	/*
		We all wanna be happy in the end but none of us ever will be.
	*/
		
	
}