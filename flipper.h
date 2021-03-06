// Flipper.h: Definition of the Flipper class
//
//////////////////////////////////////////////////////////////////////
#pragma once
#if !defined(AFX_FLIPPER_H__D65AA2A2_9F89_4AA4_88F3_D325B1001825__INCLUDED_)
#define AFX_FLIPPER_H__D65AA2A2_9F89_4AA4_88F3_D325B1001825__INCLUDED_

#include "resource.h"  
/////////////////////////////////////////////////////////////////////////////
// Flipper     // main symbols

class FlipperData
{
public:
	float m_BaseRadius;
	float m_EndRadius;
	float m_FlipperRadiusMax;  
	float m_FlipperRadius;
	float m_StartAngle;
	float m_EndAngle;
	Vertex2D m_Center;
	TimerDataRoot m_tdr;
	char m_szSurface[MAXTOKEN];
	COLORREF m_color;
	COLORREF m_rubbercolor;
	float m_force;
	int m_rubberthickness;
	int m_rubberheight;
	int m_rubberwidth;
	float m_strength;
	BOOL  m_fVisible;
    BOOL  m_fEnabled;
    BOOL  m_fCompatibility;
	float m_elasticity;
    float m_elasticityFalloff;
	float m_friction;
    float m_rampUp;
	float m_scatter;
	float m_height;
	float m_mass;
	float m_powerlaw;
	float m_obliquecorrection;
	float m_scatterangle;
	float m_FlipperRadiusMin;	// the flipper length reduction at maximum difficulty 
	float m_recoil;
	float m_return;
	float m_angleEOS;	//angle at which EOS switch opens, as measured from EOS parked position

	float m_OverrideSpeed;
	float m_OverrideStrength;
 	float m_OverrideElasticity;
  	float m_OverrideScatter;
  	float m_OverrideReturnStrength;
	float m_OverrideRecoil;
  	float m_OverridePowerLaw;
	float m_OverrideOblique;
	int m_OverridePhysics;
};

class Flipper : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<Flipper, &CLSID_Flipper>,
	public IDispatchImpl<IFlipper, &IID_IFlipper, &LIBID_VPinballLib>,
#ifdef VBA
	public CApcProjectItem<Flipper>,
#endif
	public IConnectionPointContainerImpl<Flipper>,
	public IProvideClassInfo2Impl<&CLSID_Flipper, &DIID_IFlipperEvents, &LIBID_VPinballLib>,
	public EventProxy<Flipper, &DIID_IFlipperEvents>,
	public ISelect,
	public IEditable,
	public Hitable,
	public IScriptable,
	public IFireEvents,
	public IPerPropertyBrowsing // Ability to fill in dropdown in property browser
{
public:
	Flipper();
	virtual ~Flipper();

	void SetVertices(float basex, float basey, const float angle, Vertex2D * const pvEndCenter, Vertex2D * const rgvTangents, const float baseradius, const float endradius) const;

	//virtual HRESULT GetTypeName(BSTR *pVal);
	virtual void GetDialogPanes(Vector<PropertyPane> *pvproppane);

	STANDARD_DISPATCH_DECLARE
	STANDARD_EDITABLE_DECLARES(eItemFlipper)

BEGIN_COM_MAP(Flipper)
	COM_INTERFACE_ENTRY(IFlipper)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
	COM_INTERFACE_ENTRY(IPerPropertyBrowsing)
	COM_INTERFACE_ENTRY(IProvideClassInfo)
	COM_INTERFACE_ENTRY(IProvideClassInfo2)
END_COM_MAP()

BEGIN_CONNECTION_POINT_MAP(Flipper)
	CONNECTION_POINT_ENTRY(DIID_IFlipperEvents)
END_CONNECTION_POINT_MAP()

	virtual void MoveOffset(const float dx, const float dy);
	virtual void SetObjectPos();
	// Multi-object manipulation
	virtual void GetCenter(Vertex2D * const pv) const;
	virtual void PutCenter(const Vertex2D * const pv);

	virtual void RenderShadow(ShadowSur * const psur, const float height);

    void RenderAtThickness(RenderDevice* pd3dDevice, float angle, float height, float baseradius, float endradius, float flipperheight, Vertex3D* buf);

	void WriteRegDefaults();

//DECLARE_NOT_AGGREGATABLE(Flipper) 
// Remove the comment from the line above if you don't want your object to 
// support aggregation. 

DECLARE_REGISTRY_RESOURCEID(IDR_FLIPPER)
// ISupportsErrorInfo
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

	PinTable *m_ptable;

	FlipperData m_d;
    VertexBuffer *vertexBuffer;
    IndexBuffer *indexBuffer;

	HitFlipper *m_phitflipper;

// IFlipper
public:
	STDMETHOD(get_Elasticity)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Elasticity)(/*[in]*/ float newVal);
	STDMETHOD(get_ElasticityFalloff)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_ElasticityFalloff)(/*[in]*/ float newVal);
	STDMETHOD(get_Friction)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Friction)(/*[in]*/ float newVal);
	STDMETHOD(get_RampUp)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_RampUp)(/*[in]*/ float newVal);
    STDMETHOD(get_Visible)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_Visible)(/*[in]*/ VARIANT_BOOL newVal);
    STDMETHOD(get_Enabled)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_Enabled)(/*[in]*/ VARIANT_BOOL newVal);
    STDMETHOD(get_CompatibilityMode)(/*[out, retval]*/ VARIANT_BOOL *pVal);
    STDMETHOD(put_CompatibilityMode)(/*[in]*/ VARIANT_BOOL newVal);
	STDMETHOD(get_Strength)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Strength)(/*[in]*/ float newVal);
	STDMETHOD(get_RubberThickness)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_RubberThickness)(/*[in]*/ long newVal);
	STDMETHOD(get_RubberWidth)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_RubberWidth)(/*[in]*/ long newVal);
	STDMETHOD(get_RubberHeight)(/*[out, retval]*/ long *pVal);
	STDMETHOD(put_RubberHeight)(/*[in]*/ long newVal);
	STDMETHOD(get_RubberColor)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_RubberColor)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Speed)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Speed)(/*[in]*/ float newVal);
	STDMETHOD(get_OverridePhysics)(/*[out, retval]*/ long *pVal);
    STDMETHOD(put_OverridePhysics)(/*[in]*/ long newVal);
	STDMETHOD(get_Color)(/*[out, retval]*/ OLE_COLOR *pVal);
	STDMETHOD(put_Color)(/*[in]*/ OLE_COLOR newVal);
	STDMETHOD(get_Surface)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(put_Surface)(/*[in]*/ BSTR newVal);
	STDMETHOD(get_Y)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Y)(/*[in]*/ float newVal);
	STDMETHOD(get_X)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_X)(/*[in]*/ float newVal);
	STDMETHOD(get_CurrentAngle)(/*[out, retval]*/ float *pVal);
	STDMETHOD(get_EndAngle)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_EndAngle)(/*[in]*/ float newVal);
	STDMETHOD(RotateToStart)();
	STDMETHOD(RotateToEnd)();
	STDMETHOD(get_StartAngle)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_StartAngle)(/*[in]*/ float newVal);
	STDMETHOD(get_Length)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Length)(/*[in]*/ float newVal);
	STDMETHOD(get_EndRadius)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_EndRadius)(/*[in]*/ float newVal);
	STDMETHOD(get_BaseRadius)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_BaseRadius)(/*[in]*/ float newVal);
	STDMETHOD(get_Height)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Height)(/*[in]*/ float newVal);
	STDMETHOD(get_Mass)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Mass)(/*[in]*/ float newVal);
	STDMETHOD(get_Recoil)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Recoil)(/*[in]*/ float newVal);
	STDMETHOD(get_Return)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_Return)(/*[in]*/ float newVal);
	STDMETHOD(get_AngleEOS)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_AngleEOS)(/*[in]*/ float newVal);
	STDMETHOD(get_PowerLaw)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_PowerLaw)(/*[in]*/ float newVal);
	STDMETHOD(get_ObliqueCorrection)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_ObliqueCorrection)(/*[in]*/ float newVal);
	STDMETHOD(get_FlipperRadiusMin)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_FlipperRadiusMin)(/*[in]*/ float newVal);
	STDMETHOD(get_ScatterAngle)(/*[out, retval]*/ float *pVal);
	STDMETHOD(put_ScatterAngle)(/*[in]*/ float newVal);
};

#endif // !defined(AFX_FLIPPER_H__D65AA2A2_9F89_4AA4_88F3_D325B1001825__INCLUDED_)
