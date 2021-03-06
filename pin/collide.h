#pragma once

enum
	{
	eNull,
    ePoint,
	eLineSeg,
	eJoint,
	eCircle,
	eFlipper,
	ePlunger,
	eSpinner,
	eBall,
	e3DPoly,
	eTriangle,
	ePlane,
	e3DLine,
	eGate,
	eTextbox,
    eDispReel,
	eLightSeq,
	ePrimitive,
	eTrigger,	// this value and greater are volume set tested, add rigid or non-volume set above
	eKicker		// this is done to limit to one test
	};

extern float c_hardScatter; 

extern float c_plungerNormalize;  //Adjust Mech-Plunger, useful for component change or weak spring etc.
extern bool c_plungerFilter;

// forward declarations
class Ball;
class HitObject;
class AnimObject;


struct CollisionEvent
{
    CollisionEvent() : ball(0), obj(0), isContact(false) {}

    Ball* ball;         // the ball that collided with smth
    HitObject* obj;     // what the ball collided with

    float hittime;      // when the collision happens (relative to current physics state, units: 10 ms)
    float distance;     // hit distance 

    // additional collision information; typical use (not always the same):
    // 0: hit normal, 1: hit object velocity, 2: monent and angular rate, 4: contact distance
    Vertex3Ds normal[5];

    float hitx, hity;   // position of the ball at hit time (saved to avoid floating point errors with multiple time slices)

    bool hitRigid;      // rigid body collision?

    bool isContact;     // set to true if impact velocity is 0
};


HitObject *CreateCircularHitPoly(const float x, const float y, const float z, const float r, const int sections);

class HitObject
	{
public:

	HitObject();
	virtual ~HitObject() {}

	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll) = 0;

	virtual int GetType() const = 0;

    virtual void Collide(CollisionEvent *hit) = 0;

    virtual void Contact(CollisionEvent& coll, float dtime)  { }     // apply contact forces for the given time interval

	virtual void CalcHitRect() = 0;
	
	virtual AnimObject *GetAnimObject() {return NULL;}

    void SetFriction(float friction)        { m_friction = friction; }

    void FireHitEvent(Ball* pball);

	IFireEvents *m_pfe;
	float m_threshold;
	
	//IDispatch *m_pdisp;
	IFireEvents *m_pfedebug;

	FRect3D m_rcHitRect;

	BOOL  m_fEnabled;
	int   m_ObjType;
	void* m_pObj;
	float m_elasticity;
	float m_friction;
	float m_scatter;
	};

class AnimObject
	{
public:
	virtual bool FMover() const {return false;}
	virtual void UpdateDisplacements(const float dtime) {}
	virtual void UpdateVelocities() {}

    virtual void Check3D() {}
	virtual void Reset() {}
	};

class LineSeg : public HitObject
	{
public:
    LineSeg() { }
    LineSeg(const Vertex2D& p1, const Vertex2D& p2);

	virtual float HitTestBasic(const Ball * pball, const float dtime, CollisionEvent& coll, const bool direction, const bool lateral, const bool rigid);
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
	virtual int GetType() const {return eLineSeg;}
	virtual void Collide(CollisionEvent *coll);
    virtual void Contact(CollisionEvent& coll, float dtime);
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
	virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);

	float HitTestBasicRadius(const Ball * pball, float dtime, CollisionEvent& coll,
                             bool direction, bool lateral, bool rigid);

	float HitTestRadius(const Ball * pball, const float dtime, CollisionEvent& coll);

	virtual int GetType() const {return eCircle;}

	virtual void Collide(CollisionEvent *coll);
    virtual void Contact(CollisionEvent& coll, float dtime);

	virtual void CalcHitRect();

	Vertex2D center;
	float radius;
	float zlow;
	float zhigh;
	};


// collision object which is a line segment parallel to the z axis
class HitLineZ : public HitObject
{
public:
    HitLineZ()      { }
    HitLineZ(const Vertex2D& xy, float zlow, float zhigh);

    virtual void CalcHitRect();
    virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
    virtual int GetType() const { return eJoint; }
    virtual void Collide(CollisionEvent *coll);
    virtual void Contact(CollisionEvent& coll, float dtime);

    Vertex2D m_xy;
    float m_zlow, m_zhigh;
};


class HitPoint : public HitObject
{
public:
    HitPoint(const Vertex3Ds& p);

    virtual void CalcHitRect();
    virtual float HitTest(const Ball * pball, float dtime, CollisionEvent& coll);
    virtual int GetType() const { return ePoint; }
    virtual void Collide(CollisionEvent *coll);
    virtual void Contact(CollisionEvent& coll, float dtime);

    Vertex3Ds m_p;
};



// Callback for the broadphase collision test.
// Perform the actual hittest between ball and hit object and update
// collision information if a hit occurred.
void DoHitTest(Ball *pball, HitObject *pho, CollisionEvent& coll);

