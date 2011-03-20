// primitive.cpp: implementation of the Primitive class.
//
//////////////////////////////////////////////////////////////////////
 
#include "stdafx.h" 

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Primitive::Primitive()
	{
	} 
  
Primitive::~Primitive() 
	{
	}

HRESULT Primitive::Init(PinTable *ptable, float x, float y, bool fromMouseClick)
	{
	m_ptable = ptable;

	m_d.m_vPosition.x = x;
	m_d.m_vPosition.y = y;

	SetDefaults(false);

	InitVBA(fTrue, 0, NULL);

	return S_OK;
	}

void Primitive::SetDefaults(bool fromMouseClick)
	{

	HRESULT hr;
	int iTmp;
	float fTmp;

	// sides
	hr = GetRegInt("DefaultProps\\Primitive","Sides", &iTmp);
	m_d.m_Sides = (hr == S_OK) && fromMouseClick ? iTmp : 4;


	// colors
	hr = GetRegInt("DefaultProps\\Primitive", "TopColor", &iTmp);
	m_d.m_TopColor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(100,100,100);
	hr = GetRegInt("DefaultProps\\Primitive", "TopColor", &iTmp);
	m_d.m_TopColor = (hr == S_OK) && fromMouseClick ? iTmp : RGB(150,150,150);

	// visible
	hr = GetRegInt("DefaultProps\\Primitive", "TopVisible", &iTmp);
	m_d.m_TopVisible = (hr == S_OK) && fromMouseClick ? (iTmp==1) : true;
	hr = GetRegInt("DefaultProps\\Primitive", "SideVisible", &iTmp);
	m_d.m_SideVisible = (hr == S_OK) && fromMouseClick ? (iTmp==1) : true;

	// Draw Texztures inside
	hr = GetRegInt("DefaultProps\\Primitive", "DrawTexturesInside", &iTmp);
	m_d.m_DrawTexturesInside = (hr == S_OK) && fromMouseClick ? (iTmp==1) : true;
	// Smooth Side Normals
	hr = GetRegInt("DefaultProps\\Primitive", "SmoothSideNormals", &iTmp);
	m_d.m_SmoothSideNormals = (hr == S_OK) && fromMouseClick ? (iTmp==1) : true;

	// Position (X and Y is already set by the click of the user)
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Position_Z", &fTmp);
	m_d.m_vPosition.z = (hr == S_OK) && fromMouseClick ? fTmp : 0;

	// Size
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Size_X", &fTmp);
	m_d.m_vSize.x = (hr == S_OK) && fromMouseClick ? fTmp : 100;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Size_Y", &fTmp);
	m_d.m_vSize.y = (hr == S_OK) && fromMouseClick ? fTmp : 100;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Size_Z", &fTmp);
	m_d.m_vSize.z = (hr == S_OK) && fromMouseClick ? fTmp : 100;

	// Axis Scale
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_X_Y", &fTmp);
	m_d.m_vAxisScaleX.y = (hr == S_OK) && fromMouseClick ? fTmp : 1;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_X_Z", &fTmp);
	m_d.m_vAxisScaleX.z = (hr == S_OK) && fromMouseClick ? fTmp : 1;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_Y_X", &fTmp);
	m_d.m_vAxisScaleY.x = (hr == S_OK) && fromMouseClick ? fTmp : 1;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_Y_Z", &fTmp);
	m_d.m_vAxisScaleY.z = (hr == S_OK) && fromMouseClick ? fTmp : 1;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_Z_X", &fTmp);
	m_d.m_vAxisScaleZ.x = (hr == S_OK) && fromMouseClick ? fTmp : 1;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","AxisScale_Z_Y", &fTmp);
	m_d.m_vAxisScaleZ.y = (hr == S_OK) && fromMouseClick ? fTmp : 1;
	m_d.m_vAxisScaleX.x = 1.0f;
	m_d.m_vAxisScaleY.y = 1.0f;
	m_d.m_vAxisScaleZ.z = 1.0f;


	// Rotation and Transposistion
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra0", &fTmp);
	m_d.m_aRotAndTra[0] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra1", &fTmp);
	m_d.m_aRotAndTra[1] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra2", &fTmp);
	m_d.m_aRotAndTra[2] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra3", &fTmp);
	m_d.m_aRotAndTra[3] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra4", &fTmp);
	m_d.m_aRotAndTra[4] = (hr == S_OK) && fromMouseClick ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","RotAndTra5", &fTmp);
	m_d.m_aRotAndTra[5] = (hr == S_OK) && fromMouseClick ? fTmp : 0;


	hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType0", &iTmp);
	m_d.m_aRotAndTraTypes[0] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : RotX;
	hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType1", &iTmp);
	m_d.m_aRotAndTraTypes[1] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : RotY;
	hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType2", &iTmp);
	m_d.m_aRotAndTraTypes[2] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : RotZ;
	hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType3", &iTmp);
	m_d.m_aRotAndTraTypes[3] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : TraX;
	hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType4", &iTmp);
	m_d.m_aRotAndTraTypes[4] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : TraY;
	hr = GetRegInt("DefaultProps\\Primitive","RotAndTraType5", &iTmp);
	m_d.m_aRotAndTraTypes[5] = (hr == S_OK) && fromMouseClick ? (enum RotAndTraTypeEnum)iTmp : TraZ;
	/*
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Rotation_X", &fTmp);
	m_d.m_vRotation.x = (hr == S_OK) ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Rotation_Y", &fTmp);
	m_d.m_vRotation.y = (hr == S_OK) ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Rotation_Z", &fTmp);
	m_d.m_vRotation.z = (hr == S_OK) ? fTmp : 0;

	//Transposition
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Transposition_X", &fTmp);
	m_d.m_vTransposition.x = (hr == S_OK) ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Transposition_Y", &fTmp);
	m_d.m_vTransposition.y = (hr == S_OK) ? fTmp : 0;
	hr = GetRegStringAsFloat("DefaultProps\\Primitive","Transposition_Z", &fTmp);
	m_d.m_vTransposition.z = (hr == S_OK) ? fTmp : 0;
	*/

	hr = GetRegString("DefaultProps\\Primitive","Image", m_d.m_szImage, MAXTOKEN);
	if ((hr != S_OK) && fromMouseClick)
		m_d.m_szImage[0] = 0;
	}

void Primitive::WriteRegDefaults()
	{
	char strTmp[40];
	int iTmp;

	SetRegValue("DefaultProps\\Primitive","TopColor",REG_DWORD,&m_d.m_TopColor,4);
	SetRegValue("DefaultProps\\Primitive","SideColor",REG_DWORD,&m_d.m_SideColor,4);
	iTmp = (m_d.m_TopVisible) ? 1 : 0;
	SetRegValue("DefaultProps\\Primitive","TopVisible",REG_DWORD,&iTmp,4);
	iTmp = (m_d.m_SideVisible) ? 1 : 0;
	SetRegValue("DefaultProps\\Primitive","SideVisible",REG_DWORD,&iTmp,4);

	iTmp = (m_d.m_DrawTexturesInside) ? 1 : 0;
	SetRegValue("DefaultProps\\Primitive","DrawTexturesInside",REG_DWORD,&iTmp,4);
	iTmp = (m_d.m_SmoothSideNormals) ? 1 : 0;
	SetRegValue("DefaultProps\\Primitive","SmoothSideNormals",REG_DWORD,&iTmp,4);

	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vPosition.z);
	SetRegValue("DefaultProps\\Primitive","Position_Z", REG_SZ, &strTmp,strlen(strTmp));	

	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vSize.x);
	SetRegValue("DefaultProps\\Primitive","Size_X", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vSize.y);
	SetRegValue("DefaultProps\\Primitive","Size_Y", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vSize.z);
	SetRegValue("DefaultProps\\Primitive","Size_Z", REG_SZ, &strTmp,strlen(strTmp));	

	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vAxisScaleX.y);
	SetRegValue("DefaultProps\\Primitive","AxisScaleXY", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vAxisScaleX.z);
	SetRegValue("DefaultProps\\Primitive","AxisScaleXZ", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vAxisScaleY.x);
	SetRegValue("DefaultProps\\Primitive","AxisScaleYX", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vAxisScaleY.z);
	SetRegValue("DefaultProps\\Primitive","AxisScaleYZ", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vAxisScaleZ.x);
	SetRegValue("DefaultProps\\Primitive","AxisScaleZX", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vAxisScaleZ.y);
	SetRegValue("DefaultProps\\Primitive","AxisScaleZY", REG_SZ, &strTmp,strlen(strTmp));	

	sprintf_s(&strTmp[0], 40, "%f", m_d.m_aRotAndTra[0]);
	SetRegValue("DefaultProps\\Primitive","RotAndTra0", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_aRotAndTra[1]);
	SetRegValue("DefaultProps\\Primitive","RotAndTra1", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_aRotAndTra[2]);
	SetRegValue("DefaultProps\\Primitive","RotAndTra2", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_aRotAndTra[3]);
	SetRegValue("DefaultProps\\Primitive","RotAndTra3", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_aRotAndTra[4]);
	SetRegValue("DefaultProps\\Primitive","RotAndTra4", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_aRotAndTra[5]);
	SetRegValue("DefaultProps\\Primitive","RotAndTra5", REG_SZ, &strTmp,strlen(strTmp));	

	SetRegValue("DefaultProps\\Primitive","RotAndTraType0",REG_DWORD,&m_d.m_aRotAndTraTypes[0],4);
	SetRegValue("DefaultProps\\Primitive","RotAndTraType1",REG_DWORD,&m_d.m_aRotAndTraTypes[1],4);
	SetRegValue("DefaultProps\\Primitive","RotAndTraType2",REG_DWORD,&m_d.m_aRotAndTraTypes[2],4);
	SetRegValue("DefaultProps\\Primitive","RotAndTraType3",REG_DWORD,&m_d.m_aRotAndTraTypes[3],4);
	SetRegValue("DefaultProps\\Primitive","RotAndTraType4",REG_DWORD,&m_d.m_aRotAndTraTypes[4],4);
	SetRegValue("DefaultProps\\Primitive","RotAndTraType5",REG_DWORD,&m_d.m_aRotAndTraTypes[5],4);
	/*
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vRotation.x);
	SetRegValue("DefaultProps\\Primitive","Rotation_X", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vRotation.y);
	SetRegValue("DefaultProps\\Primitive","Rotation_Y", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vRotation.z);
	SetRegValue("DefaultProps\\Primitive","Rotation_Z", REG_SZ, &strTmp,strlen(strTmp));	

	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vTransposition.x);
	SetRegValue("DefaultProps\\Primitive","Transposition_X", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vTransposition.y);
	SetRegValue("DefaultProps\\Primitive","Transposition_Y", REG_SZ, &strTmp,strlen(strTmp));	
	sprintf_s(&strTmp[0], 40, "%f", m_d.m_vTransposition.z);
	SetRegValue("DefaultProps\\Primitive","Transposition_Z", REG_SZ, &strTmp,strlen(strTmp));	
	*/
	SetRegValue("DefaultProps\\Primitive","Image", REG_SZ, &m_d.m_szImage,strlen(m_d.m_szImage));


	}


void Primitive::GetTimers(Vector<HitTimer> *pvht)
	{
	IEditable::BeginPlay();
	}


void Primitive::GetHitShapes(Vector<HitObject> *pvho)
	{
		// Here the hitshapes have to be added... lets look at other implementations.
		// OK, i need a hitprimitive class and a hitanimobject class.
		// the hitprimitive class should add itself to the HitObjectVector.
		// i think i have to look at easy hit objects and then at ramps hitobjects.
		HitPrimitive * pHitPrimitive = new HitPrimitive();
		pvho->AddElement(pHitPrimitive);
	

	}

void Primitive::GetHitShapesDebug(Vector<HitObject> *pvho)
	{
	}

void Primitive::EndPlay()
	{
	if (m_pinimage.m_pdsBuffer)
		{
		m_pinimage.FreeStuff();
		}
	}

//////////////////////////////
// Calculation
//////////////////////////////

void Primitive::RecalculateMatrices()
{
	// scale matrix
	Matrix3D Smatrix;
	Smatrix.SetIdentity();
	Smatrix._11 = m_d.m_vSize.x;
	Smatrix._22 = m_d.m_vSize.y;
	Smatrix._33 = m_d.m_vSize.z;

	// transform matrix
	Matrix3D Tmatrix;
	Tmatrix.SetIdentity();
	Tmatrix._41 = m_d.m_vPosition.x;
	Tmatrix._42 = m_d.m_vPosition.y;
	Tmatrix._43 = m_d.m_vPosition.z;
	

	Matrix3D RTmatrix;
	RTmatrix.SetIdentity();
	for (int i = 5; i >= 0; i--)
	{
		Matrix3D tempMatrix;
		switch (m_d.m_aRotAndTraTypes[i])
		{
		case RotX:
			tempMatrix.RotateXMatrix(ANGTORAD(m_d.m_aRotAndTra[i]));
			break;
		case RotY:
			tempMatrix.RotateYMatrix(ANGTORAD(m_d.m_aRotAndTra[i]));
			break;
		case RotZ:
			tempMatrix.RotateZMatrix(ANGTORAD(m_d.m_aRotAndTra[i]));
			break;
		case TraX:
			tempMatrix.SetIdentity();
			tempMatrix._41 = m_d.m_aRotAndTra[i];
			break;
		case TraY:
			tempMatrix.SetIdentity();
			tempMatrix._42 = m_d.m_aRotAndTra[i];
			break;
		case TraZ:
			tempMatrix.SetIdentity();
			tempMatrix._43 = m_d.m_aRotAndTra[i];
			break;
		}
		tempMatrix.Multiply(RTmatrix, RTmatrix);
	}

	fullMatrix = Smatrix;

	RTmatrix.Multiply(fullMatrix, fullMatrix);
	Tmatrix.Multiply(fullMatrix, fullMatrix);


}

void Primitive::RecalculateVertices() 
{
	verticesTop.RemoveAllElements();
	verticesBottom.RemoveAllElements();
	const float outherRadius = 0.5f/(cosf((float)M_PI/m_d.m_Sides));
	float currentAngle = (float)(2*M_PI/(m_d.m_Sides*2));
	const float addAngle = (float)(2*M_PI/(m_d.m_Sides));
	for (int i = 0; i < m_d.m_Sides; i++)
	{
		Vertex3D *topVert;
		topVert = new Vertex3D();
		topVert->z = 0.5f;
		topVert->x = -sinf(currentAngle)*outherRadius;
		topVert->y = -cosf(currentAngle)*outherRadius;
		verticesTop.AddElement(topVert);
		Vertex3D *bottomVert;
		bottomVert = new Vertex3D();
		bottomVert->z = -0.5f;
		bottomVert->x = -sinf (currentAngle)*outherRadius;
		bottomVert->y = -cosf(currentAngle)*outherRadius;
		verticesBottom.AddElement(bottomVert);
		currentAngle += addAngle;
	}

	RecalculateMatrices();

	for (int i = 0; i < m_d.m_Sides; i++)
	{
		Vertex3D *topVert, *bottomVert;
		topVert = (Vertex3D*)verticesTop.ElementAt(i);
		topVert->y *= 1.0f+(m_d.m_vAxisScaleX.y - 1)*(topVert->x+0.5f);
		topVert->z *= 1.0f+(m_d.m_vAxisScaleX.z - 1)*(topVert->x+0.5f);
		topVert->x *= 1.0f+(m_d.m_vAxisScaleY.x - 1)*(topVert->y+0.5f);
		topVert->z *= 1.0f+(m_d.m_vAxisScaleY.z - 1)*(topVert->y+0.5f);
		topVert->x *= 1.0f+(m_d.m_vAxisScaleZ.x - 1)*(topVert->z+0.5f);
		topVert->y *= 1.0f+(m_d.m_vAxisScaleZ.y - 1)*(topVert->z+0.5f);
		fullMatrix.MultiplyVector(topVert->x, topVert->y, topVert->z, topVert);
		bottomVert = (Vertex3D*)verticesBottom.ElementAt(i);
		bottomVert->y *= 1.0f+(m_d.m_vAxisScaleX.y - 1)*(bottomVert->x+0.5f);
		bottomVert->z *= 1.0f+(m_d.m_vAxisScaleX.z - 1)*(bottomVert->x+0.5f);
		bottomVert->x *= 1.0f+(m_d.m_vAxisScaleY.x - 1)*(bottomVert->y+0.5f);
		bottomVert->z *= 1.0f+(m_d.m_vAxisScaleY.z - 1)*(bottomVert->y+0.5f);
		bottomVert->x *= 1.0f+(m_d.m_vAxisScaleZ.x - 1)*(bottomVert->z+0.5f);
		bottomVert->y *= 1.0f+(m_d.m_vAxisScaleZ.y - 1)*(bottomVert->z+0.5f);
		fullMatrix.MultiplyVector(bottomVert->x, bottomVert->y, bottomVert->z, bottomVert);
	}
}

//////////////////////////////
// Rendering
//////////////////////////////

//2d
void Primitive::PreRender(Sur *psur)
{
	/*
	psur->SetBorderColor(-1,false,0);
	psur->SetFillColor(m_d.m_TopColor);
	psur->SetObject(this);

	psur->Line(m_d.m_vPosition.x -10.0f, m_d.m_vPosition.y,m_d.m_vPosition.x +10.0f, m_d.m_vPosition.y);
	psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y -10.0f,m_d.m_vPosition.x, m_d.m_vPosition.y +10.0f);

	*/
}

void Primitive::Render(Sur *psur)
{
	RecalculateVertices();
	//psur->SetBorderColor(RGB(0,0,0),false,2);
	psur->SetLineColor(RGB(0,0,0),false,1);
	//psur->SetFillColor(-1);
	psur->SetObject(this);
	//psur->SetObject(NULL);
	for (int i = 0; i < m_d.m_Sides; i++)
	{
		Vertex3D *topVert, *nextTopVert;
		topVert = (Vertex3D*)verticesTop.ElementAt(i);
		nextTopVert = (Vertex3D*)verticesTop.ElementAt((i+1)%m_d.m_Sides);
		psur->Line(topVert->x, topVert->y, nextTopVert->x, nextTopVert->y);
		Vertex3D *bottomVert, *nextBottomVert;
		bottomVert = (Vertex3D*)verticesBottom.ElementAt(i);
		nextBottomVert = (Vertex3D*)verticesBottom.ElementAt((i+1)%m_d.m_Sides);
		psur->Line(bottomVert->x, bottomVert->y, nextBottomVert->x, nextBottomVert->y);
		psur->Line(bottomVert->x, bottomVert->y, topVert->x, topVert->y);
	}
	psur->Line(m_d.m_vPosition.x -20.0f, m_d.m_vPosition.y,m_d.m_vPosition.x +20.0f, m_d.m_vPosition.y);
	psur->Line(m_d.m_vPosition.x, m_d.m_vPosition.y -20.0f,m_d.m_vPosition.x, m_d.m_vPosition.y +20.0f);

}

WORD rgiPrimStatic0[4] = {0,1,2,3};
WORD rgiPrimStatic1[4] = {0,3,2,1};

void Primitive::CalculateRealTimeOriginal()
{
	const float outherRadius = 0.5f/(cosf((float)M_PI/m_d.m_Sides));
	float currentAngle = (float)(2*M_PI/(m_d.m_Sides*2));
	const float addAngle = (float)(2*M_PI/(m_d.m_Sides));
	for (int i = 0; i < m_d.m_Sides; i++)
	{
		Vertex3D *topVert;
		topVert = &rgv3DTopOriginal[i];
		topVert->z = 0.5f;
		topVert->x = -sinf(currentAngle)*outherRadius;
		topVert->y = -cosf(currentAngle)*outherRadius;
		topVert->tu = (topVert->x + 1)/2;
		topVert->tv = (topVert->y + 1)/2;
		
		Vertex3D *bottomVert;
		bottomVert = &rgv3DBottomOriginal[i];
		bottomVert->z = -0.5f;
		bottomVert->x = -sinf (currentAngle)*outherRadius;
		bottomVert->y = -cosf(currentAngle)*outherRadius;
		bottomVert->tu = (bottomVert->x + 1)/2;
		bottomVert->tv = (bottomVert->y + 1)/2;
		
		currentAngle += addAngle;
	}

}
void Primitive::CalculateRealTime()
{
	RecalculateMatrices();

	for (int i = 0; i < m_d.m_Sides; i++)
	{
		Vertex3D *topVert, *bottomVert;
		topVert = &rgv3DTop[i];
		topVert->x = rgv3DTopOriginal[i].x;
		topVert->y = rgv3DTopOriginal[i].y;
		topVert->z = rgv3DTopOriginal[i].z;
		topVert->tu = rgv3DTopOriginal[i].tu;
		topVert->tv = rgv3DTopOriginal[i].tv;
		topVert->nx = 0;
		topVert->ny = 0;
		topVert->nz = -1;
		topVert->y *= 1.0f+(m_d.m_vAxisScaleX.y - 1)*(topVert->x+0.5f);
		topVert->z *= 1.0f+(m_d.m_vAxisScaleX.z - 1)*(topVert->x+0.5f);
		topVert->x *= 1.0f+(m_d.m_vAxisScaleY.x - 1)*(topVert->y+0.5f);
		topVert->z *= 1.0f+(m_d.m_vAxisScaleY.z - 1)*(topVert->y+0.5f);
		topVert->x *= 1.0f+(m_d.m_vAxisScaleZ.x - 1)*(topVert->z+0.5f);
		topVert->y *= 1.0f+(m_d.m_vAxisScaleZ.y - 1)*(topVert->z+0.5f);
		fullMatrix.MultiplyVector(topVert->x, topVert->y, topVert->z, topVert);
		bottomVert = &rgv3DBottom[i];
		bottomVert->x = rgv3DBottomOriginal[i].x;
		bottomVert->y = rgv3DBottomOriginal[i].y;
		bottomVert->z = rgv3DBottomOriginal[i].z;
		bottomVert->tu = rgv3DBottomOriginal[i].tu;
		bottomVert->tv = rgv3DBottomOriginal[i].tv;
		bottomVert->nx = 0;
		bottomVert->ny = 0;
		bottomVert->nz = -1;
		bottomVert->y *= 1.0f+(m_d.m_vAxisScaleX.y - 1)*(bottomVert->x+0.5f);
		bottomVert->z *= 1.0f+(m_d.m_vAxisScaleX.z - 1)*(bottomVert->x+0.5f);
		bottomVert->x *= 1.0f+(m_d.m_vAxisScaleY.x - 1)*(bottomVert->y+0.5f);
		bottomVert->z *= 1.0f+(m_d.m_vAxisScaleY.z - 1)*(bottomVert->y+0.5f);
		bottomVert->x *= 1.0f+(m_d.m_vAxisScaleZ.x - 1)*(bottomVert->z+0.5f);
		bottomVert->y *= 1.0f+(m_d.m_vAxisScaleZ.y - 1)*(bottomVert->z+0.5f);
		fullMatrix.MultiplyVector(bottomVert->x, bottomVert->y, bottomVert->z, bottomVert);
	}
}

//3d
void Primitive::PostRenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
{
	CalculateRealTimeOriginal();
	CalculateRealTime();

	// This is the drawing function we need... Realtime drawing.
	Pin3D * const ppin3d = &g_pplayer->m_pin3d;
	/*
	RECT * rect;
	rect = new RECT();
	rect->left = 100;
	rect->right = 1000;
	rect->top = 100;
	rect->bottom = 700;
	g_pplayer->InvalidateRect(rect);
	*/

	g_pplayer->m_ptable->SetDirtyDraw();

	PinImage * const pin = m_ptable->GetImage(m_d.m_szImage);
	float maxtu = 0;
	float maxtv = 0;

	D3DMATERIAL7 mtrl;
	mtrl.specular.r = mtrl.specular.g =	mtrl.specular.b = mtrl.specular.a =
	mtrl.emissive.r = mtrl.emissive.g =	mtrl.emissive.b = mtrl.emissive.a =
	mtrl.power = 0;
	mtrl.diffuse.a = mtrl.ambient.a = 1.0f;

	if (m_d.m_TopVisible)
	{
		if (pin)
		{
			// OK, Top is visible, and we have a image
			//lets draw
			pin->EnsureColorKey();

			pd3dDevice->SetTexture(ePictureTexture, pin->m_pdsBufferColorKey);     //rlc  alpha channel support
			pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, TRUE); 	
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

			pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE); 
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAREF, (DWORD)0x00000001);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHAFUNC, D3DCMP_GREATEREQUAL);

			pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,   D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,  D3DBLEND_INVSRCALPHA); 
			
			
			pd3dDevice->SetRenderState(D3DRENDERSTATE_COLORKEYENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);

			g_pplayer->m_pin3d.SetTextureFilter ( ePictureTexture, TEXTURE_MODE_TRILINEAR );

			mtrl.diffuse.r = mtrl.ambient.r =
			mtrl.diffuse.g = mtrl.ambient.g =
			mtrl.diffuse.b = mtrl.ambient.b = 1.0f;
		}
		else
		{
			const float r = (m_d.m_TopColor & 255) * (float)(1.0/255.0);
			const float g = (m_d.m_TopColor & 65280) * (float)(1.0/65280.0);
			const float b = (m_d.m_TopColor & 16711680) * (float)(1.0/16711680.0);

			mtrl.diffuse.r = mtrl.ambient.r = r;
			mtrl.diffuse.g = mtrl.ambient.g = g;
			mtrl.diffuse.b = mtrl.ambient.b = b;
		}
		pd3dDevice->SetMaterial(&mtrl);

		Vertex3D rgv3D[4];

		rgv3D[0].x = 300;
		rgv3D[0].y = 700;
		rgv3D[0].z = 100;
		rgv3D[1].x = 300;
		rgv3D[1].y = 1090;
		rgv3D[1].z = 100;
		rgv3D[2].x = 690;
		rgv3D[2].y = 1090;
		rgv3D[2].z = 100;
		rgv3D[3].x = 690;
		rgv3D[3].y = 700;
		rgv3D[3].z = 100;

		rgv3D[0].tu = 0;
		rgv3D[0].tv = 0;
		rgv3D[1].tu = 0;
		rgv3D[1].tv = 1;
		rgv3D[2].tu = 1;
		rgv3D[2].tv = 1;
		rgv3D[3].tu = 1;
		rgv3D[3].tv = 0;

		rgv3D[0].nx = 0;
		rgv3D[0].ny = 0;
		rgv3D[0].nz = -1;
		rgv3D[1].nx = 0;
		rgv3D[1].ny = 0;
		rgv3D[1].nz = -1;
		rgv3D[2].nx = 0;
		rgv3D[2].ny = 0;
		rgv3D[2].nz = -1;
		rgv3D[3].nx = 0;
		rgv3D[3].ny = 0;
		rgv3D[3].nz = -1;
		
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3DTop, 4,rgiPrimStatic0, 4, 0);
		pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, MY_D3DFVF_VERTEX,rgv3DTop, 4,rgiPrimStatic1, 4, 0);

	}
}

void Primitive::RenderStatic(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}
	
//seems to be called to set up the initial backbuffer
void Primitive::RenderMovers(LPDIRECT3DDEVICE7 pd3dDevice)
	{
	}

void Primitive::RenderMoversFromCache(Pin3D *ppin3d)
	{
	}


//////////////////////////////
// Positioning
//////////////////////////////

void Primitive::SetObjectPos()
	{
	g_pvp->SetObjectPosCur(m_d.m_vPosition.x, m_d.m_vPosition.y);
	}

void Primitive::MoveOffset(const float dx, const float dy)
	{
	m_d.m_vPosition.x += dx;
	m_d.m_vPosition.y += dy;

	m_ptable->SetDirtyDraw();
	}

void Primitive::GetCenter(Vertex2D *pv)
	{
		pv->x = m_d.m_vPosition.x;
		pv->y = m_d.m_vPosition.y;
	}

void Primitive::PutCenter(Vertex2D *pv)
	{
		m_d.m_vPosition.x = pv->x;
		m_d.m_vPosition.y = pv->y;

		m_ptable->SetDirtyDraw();
	}
//////////////////////////////
// Save and Load
//////////////////////////////

HRESULT Primitive::SaveData(IStream *pstm, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	BiffWriter bw(pstm, hcrypthash, hcryptkey);

#ifdef VBA
	bw.WriteInt(FID(PIID), ApcControl.ID());
#endif
	bw.WriteStruct(FID(VPOS), &m_d.m_vPosition, sizeof(Vertex3D));
	bw.WriteStruct(FID(VSIZ), &m_d.m_vSize, sizeof(Vertex3D));
	bw.WriteStruct(FID(AXSX), &m_d.m_vAxisScaleX, sizeof(Vertex3D));
	bw.WriteStruct(FID(AXSY), &m_d.m_vAxisScaleY, sizeof(Vertex3D));
	bw.WriteStruct(FID(AXSZ), &m_d.m_vAxisScaleZ, sizeof(Vertex3D));
	bw.WriteFloat(FID(RTV0), m_d.m_aRotAndTra[0]);
	bw.WriteFloat(FID(RTV1), m_d.m_aRotAndTra[1]);
	bw.WriteFloat(FID(RTV2), m_d.m_aRotAndTra[2]);
	bw.WriteFloat(FID(RTV3), m_d.m_aRotAndTra[3]);
	bw.WriteFloat(FID(RTV4), m_d.m_aRotAndTra[4]);
	bw.WriteFloat(FID(RTV5), m_d.m_aRotAndTra[5]);
	int iTmp = m_d.m_aRotAndTraTypes[0];
	bw.WriteInt(FID(RTT0), iTmp);
	iTmp = m_d.m_aRotAndTraTypes[1];
	bw.WriteInt(FID(RTT1), iTmp);
	iTmp = m_d.m_aRotAndTraTypes[2];
	bw.WriteInt(FID(RTT2), iTmp);
	iTmp = m_d.m_aRotAndTraTypes[3];
	bw.WriteInt(FID(RTT3), iTmp);
	iTmp = m_d.m_aRotAndTraTypes[4];
	bw.WriteInt(FID(RTT4), iTmp);
	iTmp = m_d.m_aRotAndTraTypes[5];
	bw.WriteInt(FID(RTT5), iTmp);
	bw.WriteString(FID(IMAG), m_d.m_szImage);
	bw.WriteInt(FID(SIDS), m_d.m_Sides);
	bw.WriteWideString(FID(NAME), (WCHAR *)m_wzName);
	bw.WriteInt(FID(TCOL), m_d.m_TopColor);
	bw.WriteInt(FID(SCOL), m_d.m_SideColor);
	bw.WriteInt(FID(TVIS), (m_d.m_TopVisible) ? 1 : 0);
	bw.WriteInt(FID(SVIS), (m_d.m_SideVisible) ? 1 : 0);
	bw.WriteInt(FID(DTXI), (m_d.m_DrawTexturesInside) ? 1 : 0);
	bw.WriteInt(FID(SSNO), (m_d.m_SmoothSideNormals) ? 1 : 0);

	ISelect::SaveData(pstm, hcrypthash, hcryptkey);

	bw.WriteTag(FID(ENDB));

	return S_OK;
	}

HRESULT Primitive::InitLoad(IStream *pstm, PinTable *ptable, int *pid, int version, HCRYPTHASH hcrypthash, HCRYPTKEY hcryptkey)
	{
	SetDefaults(false);
#ifndef OLDLOAD
	BiffReader br(pstm, this, pid, version, hcrypthash, hcryptkey);

	m_ptable = ptable;

	br.Load();
	return S_OK;
#else
	ULONG read = 0;
	HRESULT hr = S_OK;

	m_ptable = ptable;

	DWORD dwID;
	if(FAILED(hr = pstm->Read(&dwID, sizeof dwID, &read)))
		return hr;

	if(FAILED(hr = pstm->Read(&m_d, sizeof(PrimitiveData), &read)))
		return hr;

	*pid = dwID;

	return hr;
#endif
	}

BOOL Primitive::LoadToken(int id, BiffReader *pbr)
	{
	if (id == FID(PIID))
		{
		pbr->GetInt((int *)pbr->m_pdata);
		}
	else if (id == FID(VPOS))
		{
		pbr->GetStruct(&m_d.m_vPosition, sizeof(Vertex3D));
		}
	else if (id == FID(VSIZ))
		{
		pbr->GetStruct(&m_d.m_vSize, sizeof(Vertex3D));
		}
	else if (id == FID(AXSX))
		{
		pbr->GetStruct(&m_d.m_vAxisScaleX, sizeof(Vertex3D));
		}
	else if (id == FID(AXSY))
		{
		pbr->GetStruct(&m_d.m_vAxisScaleY, sizeof(Vertex3D));
		}
	else if (id == FID(AXSZ))
		{
		pbr->GetStruct(&m_d.m_vAxisScaleZ, sizeof(Vertex3D));
		}
	else if (id == FID(RTV0))
		{
			pbr->GetFloat(&m_d.m_aRotAndTra[0]);
		}
	else if (id == FID(RTT0))
		{
			int iTmp;
			pbr->GetInt(&iTmp);
			m_d.m_aRotAndTraTypes[0] = (RotAndTraTypeEnum)iTmp;
		}
	else if (id == FID(RTV1))
		{
			pbr->GetFloat(&m_d.m_aRotAndTra[1]);
		}
	else if (id == FID(RTT1))
		{
			int iTmp;
			pbr->GetInt(&iTmp);
			m_d.m_aRotAndTraTypes[1] = (RotAndTraTypeEnum)iTmp;
		}
	else if (id == FID(RTV2))
		{
			pbr->GetFloat(&m_d.m_aRotAndTra[2]);
		}
	else if (id == FID(RTT2))
		{
			int iTmp;
			pbr->GetInt(&iTmp);
			m_d.m_aRotAndTraTypes[2] = (RotAndTraTypeEnum)iTmp;
		}
	else if (id == FID(RTV3))
		{
			pbr->GetFloat(&m_d.m_aRotAndTra[3]);
		}
	else if (id == FID(RTT3))
		{
			int iTmp;
			pbr->GetInt(&iTmp);
			m_d.m_aRotAndTraTypes[3] = (RotAndTraTypeEnum)iTmp;
		}
	else if (id == FID(RTV4))
		{
			pbr->GetFloat(&m_d.m_aRotAndTra[4]);
		}
	else if (id == FID(RTT4))
		{
			int iTmp;
			pbr->GetInt(&iTmp);
			m_d.m_aRotAndTraTypes[4] = (RotAndTraTypeEnum)iTmp;
		}
	else if (id == FID(RTV5))
		{
			pbr->GetFloat(&m_d.m_aRotAndTra[5]);
		}
	else if (id == FID(RTT5))
		{
			int iTmp;
			pbr->GetInt(&iTmp);
			m_d.m_aRotAndTraTypes[5] = (RotAndTraTypeEnum)iTmp;
		}
	else if (id == FID(IMAG))
		{
		pbr->GetString(m_d.m_szImage);
		}
	else if (id == FID(SIDS))
		{
		pbr->GetInt(&m_d.m_Sides);
		}
	else if (id == FID(NAME))
		{
		pbr->GetWideString((WCHAR *)m_wzName);
		}
	else if (id == FID(TCOL))
		{
		pbr->GetInt(&m_d.m_TopColor);
		}
	else if (id == FID(SCOL))
		{
		pbr->GetInt(&m_d.m_SideColor);
		}
	else if (id == FID(TVIS))
		{
		int iTmp;
		pbr->GetInt(&iTmp);
		m_d.m_TopVisible = (iTmp==1);
		}
	else if (id == FID(SVIS))
		{
		int iTmp;
		pbr->GetInt(&iTmp);
		m_d.m_SideVisible = (iTmp==1);
		}
	else if (id == FID(DTXI))
		{
		int iTmp;
		pbr->GetInt(&iTmp);
		m_d.m_DrawTexturesInside = (iTmp==1);
		}
	else if (id == FID(SSNO))
		{
		int iTmp;
		pbr->GetInt(&iTmp);
		m_d.m_SmoothSideNormals = (iTmp==1);
		}
	else
		{
		ISelect::LoadToken(id, pbr);
		}

	return fTrue;
	}

HRESULT Primitive::InitPostLoad()
	{

	return S_OK;
	}

//////////////////////////////
// Standard methods
//////////////////////////////

STDMETHODIMP Primitive::get_Image(BSTR *pVal)
{
	WCHAR wz[512];

	MultiByteToWideChar(CP_ACP, 0, m_d.m_szImage, -1, wz, 32);
	*pVal = SysAllocString(wz);

	return S_OK;
}

STDMETHODIMP Primitive::put_Image(BSTR newVal)
{
	STARTUNDO

	WideCharToMultiByte(CP_ACP, 0, newVal, -1, m_d.m_szImage, 32, NULL, NULL);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_Sides(int *pVal)
{
	*pVal = m_d.m_Sides;

	return S_OK;
}

STDMETHODIMP Primitive::put_Sides(int newVal)
{
	if (newVal <= Max_Primitive_Sides)
	{
	STARTUNDO

		m_d.m_Sides = newVal;
		RecalculateVertices();
	STOPUNDO
	}

	return S_OK;
}

STDMETHODIMP Primitive::get_TopColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_TopColor;

	return S_OK;
}

STDMETHODIMP Primitive::put_TopColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_TopColor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_SideColor(OLE_COLOR *pVal)
{
	*pVal = m_d.m_SideColor;

	return S_OK;
}

STDMETHODIMP Primitive::put_SideColor(OLE_COLOR newVal)
{
	STARTUNDO

	m_d.m_SideColor = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_TopVisible(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_TopVisible);

	return S_OK;
}

STDMETHODIMP Primitive::put_TopVisible(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_TopVisible = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_SideVisible(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_SideVisible);

	return S_OK;
}

STDMETHODIMP Primitive::put_SideVisible(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_SideVisible = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_DrawTexturesInside(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_DrawTexturesInside);

	return S_OK;
}

STDMETHODIMP Primitive::put_DrawTexturesInside(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_DrawTexturesInside = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_SmoothSideNormals(VARIANT_BOOL *pVal)
{
	*pVal = (VARIANT_BOOL)FTOVB(m_d.m_SmoothSideNormals);

	return S_OK;
}

STDMETHODIMP Primitive::put_SmoothSideNormals(VARIANT_BOOL newVal)
{
	STARTUNDO

	m_d.m_SmoothSideNormals = VBTOF(newVal);

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_Z(float *pVal)
{
	*pVal = m_d.m_vPosition.z;

	return S_OK;
}

STDMETHODIMP Primitive::put_Z(float newVal)
{
	STARTUNDO

	m_d.m_vPosition.z = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_X(float *pVal)
{
	*pVal = m_d.m_vPosition.x;

	return S_OK;
}

STDMETHODIMP Primitive::put_X(float newVal)
{
	STARTUNDO

	m_d.m_vPosition.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_Y(float *pVal)
{
	*pVal = m_d.m_vPosition.y;

	return S_OK;
}

STDMETHODIMP Primitive::put_Y(float newVal)
{
	STARTUNDO

	m_d.m_vPosition.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_Size_X(float *pVal)
{
	*pVal = m_d.m_vSize.x;

	return S_OK;
}

STDMETHODIMP Primitive::put_Size_X(float newVal)
{
	STARTUNDO

	m_d.m_vSize.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_Size_Y(float *pVal)
{
	*pVal = m_d.m_vSize.y;

	return S_OK;
}

STDMETHODIMP Primitive::put_Size_Y(float newVal)
{
	STARTUNDO

	m_d.m_vSize.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_Size_Z(float *pVal)
{
	*pVal = m_d.m_vSize.z;

	return S_OK;
}

STDMETHODIMP Primitive::put_Size_Z(float newVal)
{
	STARTUNDO

	m_d.m_vSize.z = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleX_Y(float *pVal)
{
	*pVal = m_d.m_vAxisScaleX.y;

	return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleX_Y(float newVal)
{
	STARTUNDO

	m_d.m_vAxisScaleX.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleX_Z(float *pVal)
{
	*pVal = m_d.m_vAxisScaleX.z;

	return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleX_Z(float newVal)
{
	STARTUNDO

	m_d.m_vAxisScaleX.z = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleY_X(float *pVal)
{
	*pVal = m_d.m_vAxisScaleY.x;

	return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleY_X(float newVal)
{
	STARTUNDO

	m_d.m_vAxisScaleY.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleY_Z(float *pVal)
{
	*pVal = m_d.m_vAxisScaleY.z;

	return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleY_Z(float newVal)
{
	STARTUNDO

	m_d.m_vAxisScaleY.z = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleZ_X(float *pVal)
{
	*pVal = m_d.m_vAxisScaleZ.x;

	return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleZ_X(float newVal)
{
	STARTUNDO

	m_d.m_vAxisScaleZ.x = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_AxisScaleZ_Y(float *pVal)
{
	*pVal = m_d.m_vAxisScaleZ.y;

	return S_OK;
}

STDMETHODIMP Primitive::put_AxisScaleZ_Y(float newVal)
{
	STARTUNDO

	m_d.m_vAxisScaleZ.y = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra0(float *pVal)
{
	*pVal = m_d.m_aRotAndTra[0];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra0(float newVal)
{
	STARTUNDO

	m_d.m_aRotAndTra[0] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra1(float *pVal)
{
	*pVal = m_d.m_aRotAndTra[1];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra1(float newVal)
{
	STARTUNDO

	m_d.m_aRotAndTra[1] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra2(float *pVal)
{
	*pVal = m_d.m_aRotAndTra[2];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra2(float newVal)
{
	STARTUNDO

	m_d.m_aRotAndTra[2] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra3(float *pVal)
{
	*pVal = m_d.m_aRotAndTra[3];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra3(float newVal)
{
	STARTUNDO

	m_d.m_aRotAndTra[3] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra4(float *pVal)
{
	*pVal = m_d.m_aRotAndTra[4];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra4(float newVal)
{
	STARTUNDO

	m_d.m_aRotAndTra[4] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTra5(float *pVal)
{
	*pVal = m_d.m_aRotAndTra[5];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTra5(float newVal)
{
	STARTUNDO

	m_d.m_aRotAndTra[5] = newVal;

	STOPUNDO

	return S_OK;
}


STDMETHODIMP Primitive::get_RotAndTraType0(RotAndTraTypeEnum *pVal)
{
	*pVal = m_d.m_aRotAndTraTypes[0];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTraType0(RotAndTraTypeEnum newVal)
{
	STARTUNDO

	m_d.m_aRotAndTraTypes[0] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTraType1(RotAndTraTypeEnum *pVal)
{
	*pVal = m_d.m_aRotAndTraTypes[1];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTraType1(RotAndTraTypeEnum newVal)
{
	STARTUNDO

	m_d.m_aRotAndTraTypes[1] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTraType2(RotAndTraTypeEnum *pVal)
{
	*pVal = m_d.m_aRotAndTraTypes[2];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTraType2(RotAndTraTypeEnum newVal)
{
	STARTUNDO

	m_d.m_aRotAndTraTypes[2] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTraType3(RotAndTraTypeEnum *pVal)
{
	*pVal = m_d.m_aRotAndTraTypes[3];

	// slintf("get: %d \n", m_d.m_aRotAndTraTypes[3]);
	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTraType3(RotAndTraTypeEnum newVal)
{
	STARTUNDO

	m_d.m_aRotAndTraTypes[3] = newVal;
	// slintf("put: %d \n", m_d.m_aRotAndTraTypes[3]);
	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTraType4(RotAndTraTypeEnum *pVal)
{
	*pVal = m_d.m_aRotAndTraTypes[4];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTraType4(RotAndTraTypeEnum newVal)
{
	STARTUNDO

	m_d.m_aRotAndTraTypes[4] = newVal;

	STOPUNDO

	return S_OK;
}

STDMETHODIMP Primitive::get_RotAndTraType5(RotAndTraTypeEnum *pVal)
{
	*pVal = m_d.m_aRotAndTraTypes[5];

	return S_OK;
}

STDMETHODIMP Primitive::put_RotAndTraType5(RotAndTraTypeEnum newVal)
{
	STARTUNDO

	m_d.m_aRotAndTraTypes[5] = newVal;

	STOPUNDO

	return S_OK;
}

void Primitive::GetDialogPanes(Vector<PropertyPane> *pvproppane)
	{
	PropertyPane *pproppane;

	pproppane = new PropertyPane(IDD_PROP_NAME, NULL);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPPRIMITIVE_VISUALS, IDS_VISUALS);
	pvproppane->AddElement(pproppane);

	pproppane = new PropertyPane(IDD_PROPPRIMITIVE_POSITION, IDS_POSITION);
	pvproppane->AddElement(pproppane);
	}
