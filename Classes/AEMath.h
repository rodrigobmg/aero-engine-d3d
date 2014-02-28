/**
 * @file	AEMath.h
 * @brief	Containing items that are commonly used in other files.
 *
 * @author	CaiyZ (Mr.IroN)
 * @version	0.1
 * @date	2013
*/

#pragma once

#include <d3d11_1.h>
#include <directxmath.h>
#include <cmath>
#include <string>

using namespace DirectX;


struct AEPoint {

	FLOAT x, y;

	AEPoint() {
		x = y = 0.0f;
	}

	AEPoint(FLOAT _x, FLOAT _y) {
		x = _x;  y = _y;
	}

};


struct AERect {

	FLOAT x1, y1, x2, y2;

	AERect() {
		x1 = y1 = x2 = y2 = 0.0f;
	}

	AERect(FLOAT _x1, FLOAT _y1, FLOAT _x2, FLOAT _y2) {
		x1 = _x1;  y1 = _y1;  x2 = _x2;  y2 = _y2;
	}

};


struct AEBiasRect {

	FLOAT x1, y1, x2, y2, x3, y3, x4, y4;

	AEBiasRect() {
		x1 = y1 = x2 = y2 = x3 = y3 = x4 = y4 = 0.0f;
	}

	AEBiasRect(FLOAT _x1, FLOAT _y1, FLOAT _x2, FLOAT _y2, FLOAT _x3, FLOAT _y3, FLOAT _x4, FLOAT _y4) {
		x1 = _x1;  y1 = _y1;  x2 = _x2;  y2 = _y2; x3 = _x3;  y3 = _y3;  x4 = _x4;  y4 = _y4;
	}

};


struct SimpleVertex {

	XMFLOAT3 Pos;
	XMFLOAT2 Tex;

	SimpleVertex() {
		Pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
		Tex = XMFLOAT2(0.0f, 0.0f);
	}

	SimpleVertex(XMFLOAT3 _Pos, XMFLOAT2 _Tex) {
		Pos = _Pos;
		Tex = _Tex;
	}

};


class AEVector2 {

public:

	AEVector2(FLOAT _x, FLOAT _y) { x = _x;  y = _y; }

	static AEVector2 normalize(AEVector2 v);
	static FLOAT crossProduct(AEVector2 v1, AEVector2 v2);
	static FLOAT dotProduct(AEVector2 v1, AEVector2 v2);
	static FLOAT angleBetween(AEVector2 v1, AEVector2 v2);
	static AEVector2 rad2vec(FLOAT angleRad);
	static FLOAT vec2rad(AEVector2 v);

	FLOAT model() { return sqrtf(x * x + y * y); }

	FLOAT x, y;

};


namespace AENSMath {

	const FLOAT PI = 3.14159265f;

	enum AEFlipDirection {
		HORIZONTAL, VERTICAL
	};

	INT randomIntBetween(INT start, INT end);
	
	FLOAT randomClampf();

	FLOAT deg2rad(FLOAT deg);
	FLOAT rad2deg(FLOAT rad);
	
	FLOAT euclideanDistance(AEPoint p1, AEPoint p2);

	INT log2(INT x);
	
	AERect flipRect(AERect rect, AEFlipDirection direction);
	AEBiasRect flipRect(AEBiasRect rect, AEFlipDirection direction);

};

VOID paintRect(INT tex, AERect texClip, AERect paintArea);
VOID paintRect(INT tex, AERect texClip, AEBiasRect paintArea);
VOID paintRectByCenter(INT tex, AERect texClip, AEPoint center, FLOAT width, FLOAT height, FLOAT angleDeg);