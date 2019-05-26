#pragma once
#include <vector>
#include "../../SDK/Vector.h"
#include "../../SDK/ISurface.h"
#include "../../Menu/Menu.h"
class Dot;


extern std::vector<Dot*> Dots;

class Dot
{
public:
	Dot(Vector2D p, Vector2D v)
	{
		RelativePosition = p;
		Velocity = v;

	}

	virtual void Draw()
	{
		g_pSurface->DrawSetColor(menurgb(140));
		g_pSurface->DrawFilledRect(RelativePosition.x - 2, RelativePosition.y - 2, RelativePosition.x + 2, RelativePosition.y + 2);
		auto t = std::find(Dots.begin(), Dots.end(), this);
		if (t == Dots.end()) return;
		for (auto i = t; i != Dots.end(); i++)
		{
			if ((*i) == this) continue;
			Vector2D Pos = RelativePosition;
			float Dist = Pos.DistTo((*i)->RelativePosition);
			if (Dist < 128)
			{
				Vector2D Dir = ((*i)->RelativePosition - RelativePosition).Normalized();
				//	g_pSurface->DrawSetColor(Color(69, 140, 230, 255));
				g_pSurface->Line(Pos.x, Pos.y, (Pos + Dir * Dist).x, (Pos + Dir * Dist).y, menurgb(140));

			}
		}
	}

	Vector2D RelativePosition = Vector2D(0, 0);
	Vector2D Velocity;
};

namespace Drop
{
	void DrawBackDrop();

}
