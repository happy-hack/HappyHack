#pragma once

#include "bspflags.h"

#define CHAR_TEX_ANTLION		'A'
#define CHAR_TEX_BLOODYFLESH	'B'
#define	CHAR_TEX_CONCRETE		'C'
#define CHAR_TEX_DIRT			'D'
#define CHAR_TEX_EGGSHELL		'E' ///< the egg sacs in the tunnels in ep2.
#define CHAR_TEX_FLESH			'F'
#define CHAR_TEX_GRATE			'G'
#define CHAR_TEX_ALIENFLESH		'H'
#define CHAR_TEX_CLIP			'I'
//#define CHAR_TEX_UNUSED		'J'
#define CHAR_TEX_SNOW			'K'
#define CHAR_TEX_PLASTIC		'L'
#define CHAR_TEX_METAL			'M'
#define CHAR_TEX_SAND			'N'
#define CHAR_TEX_FOLIAGE		'O'
#define CHAR_TEX_COMPUTER		'P'
//#define CHAR_TEX_UNUSED		'Q'
#define CHAR_TEX_REFLECTIVE		'R'
#define CHAR_TEX_SLOSH			'S'
#define CHAR_TEX_TILE			'T'
#define CHAR_TEX_CARDBOARD		'U'
#define CHAR_TEX_VENT			'V'
#define CHAR_TEX_WOOD			'W'
//#define CHAR_TEX_UNUSED		'X' ///< do not use - "fake" materials use this (ladders, wading, clips, etc)
#define CHAR_TEX_GLASS			'Y'
#define CHAR_TEX_WARPSHIELD		'Z' ///< wierd-looking jello effect for advisor shield.

#define CHAR_TEX_STEAM_PIPE		11

struct C_Ray
{
	VectorAligned m_Start; // starting point, centered within the extents
	VectorAligned m_Delta; // direction + length of the ray
	VectorAligned m_StartOffset; // Add this to m_Start to get the actual ray start
	VectorAligned m_Extents; // Describes an axis aligned box extruded along a ray
	const matrix3x4_t* m_pWorldAxisTransform;
	//const matrix3x4_t *m_pWorldAxisTransform;
	bool m_IsRay; // are the extents zero?
	bool m_IsSwept; // is delta != 0?


	C_Ray(Vector _start, Vector _end)
	{
		Init(_start, _end);
	}

	C_Ray(Vector _start, Vector _end, Vector _mins, Vector _maxs)
	{
		Init(_start, _end, _mins, _maxs);
	}

	void Init(const Vector& vecStart, const Vector& vecEnd)
	{
		m_Delta = vecEnd - vecStart;

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents.x = m_Extents.y = m_Extents.z = 0.0f;
		m_IsRay = true;

		m_StartOffset.x = m_StartOffset.y = m_StartOffset.z = 0.0f;

		m_Start = vecStart;
	}

	void Init(Vector& vecStart, Vector& vecEnd, Vector min, Vector max)
	{
		m_Delta = vecEnd - vecStart;

		m_IsSwept = (m_Delta.LengthSqr() != 0);

		m_Extents.x = (max.x - min.x);
		m_Extents.y = (max.y - min.y);
		m_Extents.z = (max.z - min.z);
		m_IsRay = false;

		m_StartOffset.x = m_StartOffset.y = m_StartOffset.z = 0.0f;

		m_Start = vecStart + ((max + min) * 0.5f);
	}

private:
};

struct csurface_t
{
	const char* name;
	short surfaceProps;
	unsigned short flags;
};

struct cplane_t
{
	Vector normal;
	float dist;
	byte type;
	byte signbits;
	byte pad[2];
};

struct C_Trace
{
	bool DidHitWorld() const
	{
		return m_pEnt && m_pEnt->EntIndex() == 0;
	}

	bool DidHitNonWorldEntity() const
	{
		return m_pEnt != NULL && !DidHitWorld();
	}

	bool DidHit() const
	{
		return flFraction < 1.f;
	}

	Vector start;
	Vector end;
	cplane_t plane;
	float flFraction;
	int contents;
	WORD dispFlags;
	bool allsolid;
	bool startSolid;
	float fractionLeftSolid;
	csurface_t surface;
	int hitGroup;
	short physicsBone;
	C_BaseEntity* m_pEnt;
	int hitbox;
};

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY, // NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY, // NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS, // NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};

class ITraceFilter
{
public:
	virtual bool ShouldHitEntity(void* pEntity, int contentsMask) = 0;
	virtual TraceType_t GetTraceType() const = 0;
};

class C_TraceFilter : public ITraceFilter
{
public:

	explicit C_TraceFilter(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pSkip1 = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return (pEntityHandle != pSkip1);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip1;
};
class CTraceFilterWorldOnly : public ITraceFilter
{
public:
	bool ShouldHitEntity(void* /*pServerEntity*/, int /*contentsMask*/)
	{
		return false;
	}
	virtual TraceType_t GetTraceType() const
	{
		return TraceType_t::TRACE_WORLD_ONLY;
	}
};
class CTraceFilterOneEntity : public ITraceFilter
{
public:

	explicit CTraceFilterOneEntity(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pEntity = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return (pEntityHandle == pEntity);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pEntity;
};

class CTraceFilterOneEntity2 : public ITraceFilter
{
public:

	explicit CTraceFilterOneEntity2(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pEntity = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return (pEntityHandle == pEntity);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}

	void* pEntity;
};

class CTraceFilterSkipTwoEntities : public ITraceFilter
{
public:
	CTraceFilterSkipTwoEntities(void* pPassEnt1, void* pPassEnt2)
	{
		passentity1 = pPassEnt1;
		passentity2 = pPassEnt2;
	}

	virtual bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == passentity1 || pEntityHandle == passentity2);
	}

	virtual TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* passentity1;
	void* passentity2;
};

class CTraceEntity : public ITraceFilter
{
public:

	explicit CTraceEntity(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pSkip1 = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return !(pEntityHandle == pSkip1);
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_ENTITIES_ONLY;
	}

	void* pSkip1;
};

class CTraceWorldOnly : public ITraceFilter
{
public:

	explicit CTraceWorldOnly(C_BaseEntity* entity, TraceType_t tracetype = TRACE_EVERYTHING)
	{
		pSkip1 = entity;
	}

	bool ShouldHitEntity(void* pEntityHandle, int contentsMask)
	{
		return false;
	}

	TraceType_t GetTraceType() const
	{
		return TRACE_EVERYTHING;
	}

	void* pSkip1;
};

class IEngineTrace
{
public:
	// Returns the contents mask + entity at a particular world-space position
	virtual int		GetPointContents(const Vector &vecAbsPosition, int contentsMask = mask_all, IHandleEntity** ppEntity = NULL) = 0;

	// Returns the contents mask of the world only @ the world-space position (static props are ignored)
	virtual int		GetPointContents_WorldOnly(const Vector &vecAbsPosition, int contentsMask = mask_all) = 0;

	// Get the point contents, but only test the specific entity. This works
	// on static props and brush models.
	//
	// If the entity isn't a static prop or a brush model, it returns CONTENTS_EMPTY and sets
	// bFailed to true if bFailed is non-null.
	virtual int		GetPointContents_Collideable(ICollideable *pCollide, const Vector &vecAbsPosition) = 0;

	// Traces a ray against a particular entity
	virtual void	ClipRayToEntity(const C_Ray &ray, unsigned int fMask, IHandleEntity *pEnt, C_Trace *pTrace) = 0;

	// Traces a ray against a particular entity
	virtual void	ClipRayToCollideable(const C_Ray &ray, unsigned int fMask, ICollideable *pCollide, C_Trace *pTrace) = 0;

	// A version that simply accepts a ray (can work as a traceline or tracehull)
	virtual void	TraceRay(const C_Ray &ray, unsigned int fMask, ITraceFilter *pTraceFilter, C_Trace *pTrace) = 0;
};

extern IEngineTrace* g_pTrace;


enum
{
	TE_BEAMPOINTS = 0x00,
	TE_SPRITE = 0x01,
	TE_BEAMDISK = 0x02,
	TE_BEAMCYLINDER = 0x03,
	TE_BEAMFOLLOW = 0x04,
	TE_BEAMRING = 0x05,
	TE_BEAMSPLINE = 0x06,
	TE_BEAMRINGPOINT = 0x07,
	TE_BEAMLASER = 0x08,
	TE_BEAMTESLA = 0x09,
};
enum
{
	FBEAM_STARTENTITY = 0x00000001,
	FBEAM_ENDENTITY = 0x00000002,
	FBEAM_FADEIN = 0x00000004,
	FBEAM_FADEOUT = 0x00000008,
	FBEAM_SINENOISE = 0x00000010,
	FBEAM_SOLID = 0x00000020,
	FBEAM_SHADEIN = 0x00000040,
	FBEAM_SHADEOUT = 0x00000080,
	FBEAM_ONLYNOISEONCE = 0x00000100,
	FBEAM_NOTILE = 0x00000200,
	FBEAM_USE_HITBOXES = 0x00000400,
	FBEAM_STARTVISIBLE = 0x00000800,
	FBEAM_ENDVISIBLE = 0x00001000,
	FBEAM_ISACTIVE = 0x00002000,
	FBEAM_FOREVER = 0x00004000,
	FBEAM_HALOBEAM = 0x00008000,
	FBEAM_REVERSED = 0x00010000,
	NUM_BEAM_FLAGS = 17
};
class c_beam;
class beam_t;

struct beam_info_t
{
	int				m_nType;
	C_BaseEntity* m_pStartEnt;
	int				m_nStartAttachment;
	C_BaseEntity* m_pEndEnt;
	int				m_nEndAttachment;
	Vector			m_vecStart;
	Vector			m_vecEnd;
	int				m_nModelIndex;
	const char* m_pszModelName;
	int				m_nHaloIndex;
	const char* m_pszHaloName;
	float			m_flHaloScale;
	float			m_flLife;
	float			m_flWidth;
	float			m_flEndWidth;
	float			m_flFadeLength;
	float			m_flAmplitude;
	float			m_flBrightness;
	float			m_flSpeed;
	int				m_nStartFrame;
	float			m_flFrameRate;
	float			m_flRed;
	float			m_flGreen;
	float			m_flBlue;
	bool			m_bRenderable;
	int				m_nSegments;
	int				m_nFlags;
	Vector			m_vecCenter;
	float			m_flStartRadius;
	float			m_flEndRadius;

	beam_info_t()
	{
		m_nType = TE_BEAMPOINTS;
		m_nSegments = -1;
		m_pszModelName = NULL;
		m_pszHaloName = NULL;
		m_nModelIndex = -1;
		m_nHaloIndex = -1;
		m_bRenderable = true;
		m_nFlags = 0;
	}
};

class i_traceFilter;
class i_texture;

class i_viewrender_beams
{
public:
	// Construction
public:
	i_viewrender_beams(void);
	virtual				~i_viewrender_beams(void);

	// Implement IViewRenderBeams
public:
	virtual	void		InitBeams(void);
	virtual	void		ShutdownBeams(void);
	virtual	void		ClearBeams(void);

	// Updates the state of the temp ent beams
	virtual void		UpdateTempEntBeams();

	virtual void		draw_beam(beam_t* pbeam);
	virtual void		DrawBeam(c_beam* pbeam, C_TraceFilter* pEntityBeamTraceFilter = NULL);

	virtual	void		KillDeadBeams(C_BaseEntity* pDeadEntity);

	virtual	void		CreateBeamEnts(int startEnt, int endEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int type = -1);
	virtual beam_t* CreateBeamEnts(beam_info_t& beamInfo);

	virtual	void		CreateBeamEntPoint(int	nStartEntity, const Vector* pStart, int nEndEntity, const Vector* pEnd,
		int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b);
	virtual beam_t* CreateBeamEntPoint(beam_info_t& beamInfo);

	virtual	void		create_beam_points(Vector& start, Vector& end, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b);
	virtual	beam_t* create_beam_points(beam_info_t& beamInfo);

	virtual	void		CreateBeamRing(int startEnt, int endEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int flags);
	virtual beam_t* CreateBeamRing(beam_info_t& beamInfo);

	virtual void		CreateBeamRingPoint(const Vector& center, float start_radius, float end_radius, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float m_nEndWidth, float m_nFadeLength, float amplitude,
		float brightness, float speed, int startFrame,
		float framerate, float r, float g, float b, int flags);
	virtual beam_t* CreateBeamRingPoint(beam_info_t& beamInfo);

	virtual	void		CreateBeamCirclePoints(int type, Vector& start, Vector& end,
		int modelIndex, int haloIndex, float haloScale, float life, float width,
		float endWidth, float fadeLength, float amplitude, float brightness, float speed,
		int startFrame, float framerate, float r, float g, float b);
	virtual beam_t* CreateBeamCirclePoints(beam_info_t& beamInfo);

	virtual	void		CreateBeamFollow(int startEnt, int modelIndex, int haloIndex, float haloScale,
		float life, float width, float endWidth, float fadeLength, float r, float g, float b,
		float brightness);
	virtual beam_t* CreateBeamFollow(beam_info_t& beamInfo);

}; extern i_viewrender_beams* g_pview_renderbeams;