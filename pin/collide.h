#pragma once

enum
	{
	eNull,
	eLineSeg,
	eJoint,
	eCircle,
	eFlipper,
	ePlunger,
	eSpinner,
	eBall,
	e3DPoly,
	eTriangle,
	e3DLine,
	eGate,
	eTextbox,
    eDispReel,
	eLightSeq,
	ePrimitive,
	eTrigger,	// this value and greater are volume set tested, add rigid or non-volume set above
	eKicker		// this is done to limit to one test
	};

extern float c_hardFriction; 
extern float c_hardScatter; 
extern float c_maxBallSpeedSqr; 
extern float c_dampingFriction;

extern float c_plungerNormalize;  //Adjust Mech-Plunger, useful for component change or weak spring etc.
extern bool c_plungerFilter;

class Ball;
class HitObject;
class AnimObject;

HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections);

class HitObject
	{
public:

	HitObject();
	virtual ~HitObject() {}

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal) = 0;

	virtual int GetType() const = 0;

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal) = 0;

	virtual void CalcHitRect() = 0;
	
	virtual AnimObject *GetAnimObject() {return NULL;}

	IFireEvents *m_pfe;
	float m_threshold;
	
	//IDispatch *m_pdisp;
	IFireEvents *m_pfedebug;

	FRect3D m_rcHitRect;

	BOOL  m_fEnabled;
	int   m_ObjType;
	void* m_pObj;
	float m_elasticity;
	float m_antifriction;
	float m_scatter;
	};

class AnimObject
	{
public:
	virtual bool FMover() const {return false;}
	virtual void UpdateDisplacements(const float dtime) {}
	virtual void UpdateVelocities() {}

    virtual void Check3D() {}
	virtual ObjFrame *Draw3D(const RECT * const prc) {return NULL;}
	virtual void Reset() {}
	};

class LineSeg : public HitObject
	{
public:
	virtual float HitTestBasic(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal, const bool direction, const bool lateral, const bool rigid);
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);
	virtual int GetType() const {return eLineSeg;}
	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);
	void CalcNormal();
	void CalcLength();
	virtual void CalcHitRect();

	Vertex2D normal;
	Vertex2D v1, v2;
	float length;
	};

class HitCircle : public HitObject
	{
public:
	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	float HitTestBasicRadius(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal,
									const bool direction, const bool lateral, const bool rigid);

	float HitTestRadius(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual int GetType() const {return eCircle;}

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	Vertex2D center;
	float radius;
	float zlow;
	float zhigh;
	};

class Joint : public HitCircle
	{
public:
	Joint();

	virtual float HitTest(Ball * const pball, const float dtime, Vertex3Ds * const phitnormal);

	virtual int GetType() const {return eJoint;}

	virtual void Collide(Ball * const pball, Vertex3Ds * const phitnormal);

	virtual void CalcHitRect();

	//Vertex2D v;
	Vertex2D normal;
	};


#define SSE_LEAFTEST

class HitKD
{
public:
	HitKD(Vector<HitObject> *vho, const unsigned int num_items, const bool dynamic = false)
	{
		m_org_vho = vho;
		m_num_items = num_items;

		m_num_nodes = 0;

		m_dynamic = dynamic;

		m_max_items = num_items;

#ifdef SSE_LEAFTEST
		l_r_t_b_zl_zh = NULL;
#endif
		if(num_items > 0)
		{
			m_org_idx = (unsigned int*)malloc(m_max_items*4);
			tmp = (unsigned int*)malloc(m_max_items*4);
			m_nodes = malloc(m_max_items*12*4 * 4); //!!
		}
		else
		{
			m_org_idx = NULL;
			tmp = NULL;
			m_nodes = NULL;
		}
	}
	~HitKD();

	void Update(Vector<HitObject> *vho, const unsigned int num_items)
	{
		m_org_vho = vho;
		m_num_items = num_items;

		m_num_nodes = 0;

		if(num_items > m_max_items)
		{
			if(m_max_items > 0)
			{
#ifdef SSE_LEAFTEST
				_aligned_free(l_r_t_b_zl_zh);
#endif
				free(m_org_idx);
				free(tmp);
				free(m_nodes);
			}
			m_max_items = num_items;

			l_r_t_b_zl_zh = (float*)_aligned_malloc(sizeof(float) * ((m_max_items+3)&0xFFFFFFFC) * 6, 16);
			m_org_idx = (unsigned int*)malloc(m_max_items*4);
			tmp = (unsigned int*)malloc(m_max_items*4);
			m_nodes = malloc(m_max_items*12*4 * 4); //!!
		}
	}

	void InitSseArrays();

	unsigned int m_num_items;
	unsigned int m_max_items;

	Vector<HitObject> *m_org_vho;

	unsigned int * __restrict m_org_idx;
	unsigned int * __restrict tmp;
#ifdef SSE_LEAFTEST
	float * __restrict l_r_t_b_zl_zh;
#endif

	void* __restrict m_nodes;
	unsigned int m_num_nodes;

	bool m_dynamic;
};

class HitKDNode
{
public:
	HitKDNode() { m_children = NULL; m_hitoct = NULL; }
	~HitKDNode();

	void HitTestXRay(Ball * const pball, Vector<HitObject> * const pvhoHit) const;

	void HitTestBall(Ball * const pball) const;
#ifdef SSE_LEAFTEST
	void HitTestBallSse(Ball * const pball) const;
	void HitTestBallSseInner(Ball * const pball, const int i) const;
#else
#define HitTestBallSse HitTestBall
#endif

	void CreateNextLevel();

	FRect3D m_rectbounds;
	unsigned int m_start;
	unsigned int m_items; // contains the 2 bits for axis (bits 30/31)

	HitKDNode * __restrict m_children; // if NULL then this is a leaf, otherwise keeps the 2 children

	HitKD * __restrict m_hitoct; //!! meh, stupid
};
