#include <d3d11_1.h>
#include <DirectXColors.h>
#include <string>
#include <cmath>
#include "AEMath.h"
#include "AEPhysics.h"
#include "AETable.h"
#include "AEResource.h"
#include "AEBackground.h"
#include "AEPlatform.h"
#include "AESprite.h"

extern AEConstantTable<AEObject>			ae_ObjectTable;
extern SpriteBatch*							xtk_SpriteBatch;

AESprite::AESprite(AERO_SPRITE_DESC desc) {
	obj = desc.obj;
	action = desc.action;
	team = desc.team;
	cx = desc.cx;
	cy = desc.cy;
	layerDepth = desc.layerDepth;
	flip = desc.flip;
	if (action > 0) {
		changeAction(desc.action);
	}
	timeToLive = TIME_TO_LIVE_UNLIMITED;
	deadFlag = FALSE;
}

AEPoint AESprite::calcRotatedPoint(AEPoint point, FLOAT cx, FLOAT cy, AEFrame* f, FLOAT angle, BYTE flip) {
	FLOAT cosA = cos(angle), sinA = sin(angle);
	FLOAT x, y;
	if (flip == FACING_RIGHT) {
		x = cx + (point.x - f->getCenterx()) * cosA - (point.y - f->getCentery()) * sinA;
		y = cy + (point.x - f->getCenterx()) * sinA + (point.y - f->getCentery()) * cosA;
	}
	else {
		x = cx - (point.x - f->getCenterx()) * cosA + (point.y - f->getCentery()) * sinA;
		y = cy + (point.x - f->getCenterx()) * sinA + (point.y - f->getCentery()) * cosA;
	}
	return AEPoint(x, y);
}

AERect AESprite::calcSpriteRect(FLOAT cx, FLOAT cy, AEFrame* f, BYTE flip) {
	INT centerx = f->getCenterx(), centery = f->getCentery();
	INT width = f->getWidth(), height = f->getHeight();
	FLOAT x1, y1, x2, y2;
	if (flip == FACING_RIGHT) {
		x1 = cx - centerx;
		y1 = cy - centery;
		x2 = x1 + width;
		y2 = y1 + height;
	}
	else {
		x2 = cx + centerx;
		y1 = cy - centery;
		x1 = x2 - width;
		y2 = y1 + height;
	}
	return AERect(x1, y1, x2, y2);
}

RECT AESprite::calcSpriteRectInRECT(FLOAT cx, FLOAT cy, AEFrame* f, BYTE flip) {
	INT centerx = f->getCenterx(), centery = f->getCentery();
	INT width = f->getWidth(), height = f->getHeight();
	FLOAT x1, y1, x2, y2;
	if (flip == FACING_RIGHT) {
		x1 = cx - centerx;
		y1 = cy - centery;
		x2 = x1 + width;
		y2 = y1 + height;
	}
	else {
		x2 = cx + centerx;
		y1 = cy - centery;
		x1 = x2 - width;
		y2 = y1 + height;
	}
	return RECT{ (LONG)x1, (LONG)y1, (LONG)x2, (LONG)y2 };
}

AEBiasRect AESprite::calcRotatedSpriteRect(FLOAT cx, FLOAT cy, AEFrame* f, FLOAT angle, BYTE flip) {
	FLOAT cosA = cos(angle), sinA = sin(angle);
	INT centerx = f->getCenterx(), centery = f->getCentery();
	INT width = f->getWidth(), height = f->getHeight();
	FLOAT x1, y1, x2, y2, x3, y3, x4, y4;
	if (flip == FACING_RIGHT) {
		x1 = cx - centerx * cosA + centery * sinA;
		y1 = cy - centerx * sinA - centery * cosA;
		x2 = x1 + width * cosA;
		y2 = y1 + width * sinA;
		x3 = x2 - height * sinA;
		y3 = y2 + height * cosA;
		x4 = x1 - height * sinA;
		y4 = y1 + height * cosA;
	}
	else {
		x2 = cx + centerx * cosA - centery * sinA;
		y2 = cy - centerx * sinA - centery * cosA;
		x1 = x2 - width * cosA;
		y1 = y2 + width * sinA;
		x3 = x2 + height * sinA;
		y3 = y2 + height * cosA;
		x4 = x1 + height * sinA;
		y4 = y1 + height * cosA;
	}
	return AEBiasRect(x1, y1, x2, y2, x3, y3, x4, y4);
}

VOID AESprite::changeAction(INT _action) {
	if (_action == ACTION_NUM_DEAD) {
		deadFlag = TRUE;
		return;
	}
	action = _action;
	AEAnimation* anim = obj->getAnim(action);
	state = anim->getState();
	timeToLive = anim->getTTL();
	frameNum = time = 0;
	INT dvx = anim->getFrame(frameNum)->getDvx();
	if (dvx == 999)
		vx = 0;
	else
		vx += dvx;
	INT dvy = anim->getFrame(frameNum)->getDvy();
	if (dvy == 999)
		vy = 0;
	else
		vy += dvy;
}

XMVECTOR AESprite::getFacingVector(INT option) {
	XMFLOAT2 vec2;
	FLOAT facingAngle = 0.0f;
	if (option == ANGLE_DIRECTION) {
		facingAngle = angle;
	}
	else if (option == ANGLE_DISPLAY) {
		facingAngle = angleDisplay;
	}
	if (facingAngle == AENSMath::PI / 2) {
		vec2 = { 0.0f, 1.0f };
	}
	else if (facingAngle == -AENSMath::PI / 2) {
		vec2 = { 0.0f, -1.0f };
	}
	else if (facingAngle > -AENSMath::PI / 2 && facingAngle < AENSMath::PI / 2) {
		vec2 = { 1.0f, tanf(facingAngle) };
	}
	else {
		vec2 = { -1.0f, -tanf(facingAngle) };
	}
	if (flip == SpriteEffects_FlipHorizontally) {
		vec2.x = -vec2.x;
		vec2.y = -vec2.y;
	}
	return XMVector2Normalize(XMLoadFloat2(&vec2));
}

XMVECTOR AESprite::getVelocityVector() {
	XMFLOAT2 vec2 = { vx, vy };
	return XMVector2Normalize(XMLoadFloat2(&vec2));
}

VOID AESprite::applyControl() {

}

VOID AESprite::platformCollisionCheck(FLOAT cx_old, FLOAT cy_old, AEHashedTable<AEPlatform>* platformTable) {
	XMFLOAT2 sprOldPos = { cx_old, cy_old };
	XMFLOAT2 sprNewPos = { cx_old + (flip ? -1 : 1) * vx, cy_old + vy };
	for (INT iHash = 0; iHash < platformTable->getHashCount(); iHash++) {
		AEPlatform* platform = platformTable->getItemByHash(iHash);
		for (INT i = 0; i < platform->getSegmentCount(); i++) {
			AECollisionResult collisionResult = AENSCollision::vectorAndSegment(sprOldPos, sprNewPos, platform->getNode(i), platform->getNode(i + 1));
			if (collisionResult.isCollided) {
				platformCollision(platformTable, collisionResult);
				// TODO: One-side collision
				// TODO: On-the-platform check and process
			}
		}
	}

}

VOID AESprite::platformCollision(AEHashedTable<AEPlatform>* platformTable, AECollisionResult collisionResult) {
	XMVECTOR normalizedFacingVec = getVelocityVector();
	FLOAT groundAdjust = 0.1f;
	cx = collisionResult.point.x - groundAdjust * XMVectorGetX(normalizedFacingVec);
	cy = collisionResult.point.y - groundAdjust * XMVectorGetY(normalizedFacingVec);
}

VOID AESprite::update(AEHashedTable<AEPlatform>* platformTable) {
	if (ai) {
		ai->execute();
	}
	AEAnimation* anim = obj->getAnim(action);
	if (timeToLive == 0) {
		changeAction(anim->getNext());
		return;
	}
	if (timeToLive > 0) timeToLive--;
	INT fac = (flip ? -1 : 1);
	FLOAT cx_old = cx, cy_old = cy;
	cx += (fac * vx);
	cy += vy;
	vx += ax;
	vy += ay;
	if (platformTable != nullptr) {
		platformCollisionCheck(cx_old, cy_old, platformTable);
	}
	angle += (fac * vAngle);
	if (angle < -AENSMath::PI) angle += 2.0f * AENSMath::PI;
	if (angle >= AENSMath::PI) angle -= 2.0f * AENSMath::PI;
	angleDisplay += (fac * vAngleDisplay);
	if (angleDisplay < -AENSMath::PI) angleDisplay += 2.0f * AENSMath::PI;
	if (angleDisplay >= AENSMath::PI) angleDisplay -= 2.0f * AENSMath::PI;
	time++;
	if (time >= anim->getEndTime(frameNum)) {
		cx += (fac * anim->getFrame(frameNum)->getShiftx());
		cy += anim->getFrame(frameNum)->getShifty();
		frameNum++;
		if (time >= anim->getEndTime(anim->getFrameCount() - 1)) {
			time = 0;
		}
		if (frameNum == anim->getFrameCount()) {
			frameNum = 0;
			if (!anim->isLoop()) {
				changeAction(anim->getNext());
				return;
			}
		}
	}
	if (attachments) {
		updateAttachments(fac* vx, vy);
	}
}

VOID AESprite::render() {
	AEFrame* f = obj->getAnim(action)->getFrame(frameNum);
	FLOAT fwidth = (FLOAT)(f->getWidth()), fcenterx = (FLOAT)(f->getCenterx()), fcentery = (FLOAT)(f->getCentery());
	AEResource* res = f->getResource();
	RECT texClipInTexel = res->getTexClipInTexel(f->getImgOffset(), f->getImgCells());
	FLOAT flipAdjust = flip * (fwidth - 2.0f * fcenterx);
	FLOAT flipAdjustX = flipAdjust * cosf(angleDisplay), flipAdjustY = flipAdjust * sinf(angleDisplay);
	xtk_SpriteBatch->Draw(
		res->getTexture(), // Texture
		XMFLOAT2(cx - flipAdjustX, cy - flipAdjustY), // Drawing Position (Origin Point)
		&texClipInTexel, // Texture Clip Rectangle
		XMVectorSet(1.0f, 1.0f, 1.0f, alpha), // Tilting Color
		angleDisplay, // Rotation
		XMFLOAT2(fcenterx, fcentery), // Rotation Origin / Drawing Center
		1.0f, // Scale
		flip, // Sprite Effects
		layerDepth // Z Value
	);
	if (attachments) {
		renderAttachments();
	}
	
	// Old drawing method not using SpriteBatch
	/*
	AERect texClip = res->getTexClip(f->getImgOffset(), f->getImgCells(), flip);
	if (angle == 0.0f) {
		AERect paintArea = calcSpriteRect(cx, cy, f, flip);
		res->addToRenderBuffer(paintArea, texClip, alpha, zValue);
	}
	else {
		AEBiasRect paintArea = calcRotatedSpriteRect(cx, cy, f, angle, flip);
		res->addToRenderBuffer(paintArea, texClip, alpha, zValue);
	}
	*/
}

FLOAT AESprite::getAngle(INT option) {
	if (option == ANGLE_DIRECTION) {
		return angle;
	}
	else {
		return angleDisplay;
	}
}

FLOAT AESprite::getVAngle(INT option) {
	if (option == ANGLE_DIRECTION) {
		return vAngle;
	}
	else {
		return vAngleDisplay;
	}
}

VOID AESprite::setAngleRad(FLOAT rad, INT option) {
	if (option != ANGLE_DISPLAY) {
		angle = rad;
	}
	if (option != ANGLE_DIRECTION) {
		angleDisplay = rad;
	}
}

VOID AESprite::setAngleDeg(FLOAT degree, INT option) {
	setAngleRad(AENSMath::deg2rad(degree), option);
}

VOID AESprite::setVAngleRad(FLOAT rad, INT option){
	if (option != ANGLE_DISPLAY) {
		vAngle = rad;
	}
	if (option != ANGLE_DIRECTION) {
		vAngleDisplay = rad;
	}
}

VOID AESprite::setVAngleDeg(FLOAT degree, INT option){
	setVAngleRad(AENSMath::deg2rad(degree), option);
}

VOID AESprite::rotateRad(FLOAT rad, INT option) {
	if (option != ANGLE_DISPLAY) {
		angle += rad;
	}
	if (option != ANGLE_DIRECTION) {
		angleDisplay += rad;
	}
}

VOID AESprite::rotateDeg(FLOAT degree, INT option) {
	rotateRad(AENSMath::deg2rad(degree), option);
}

VOID AESprite::createAttachmentTable(INT size) {
	attachments = new AEHashedTable<AESprite>(size);
	attachmentTableSize = size;
}

VOID AESprite::addAttachment(INT slot, AESprite* attachment) {
	if (attachmentTableSize == 0 || attachments == nullptr) {
		AENSGameControl::exitGame("Cannot add attachment to sprite.");
		return;
	}
	attachment->setScene(scene);
	attachments->addAt(slot, attachment);
}

VOID AESprite::updateAttachments(FLOAT hostdx, FLOAT hostdy) {
	for (INT iHash = 0; iHash < attachments->getHashCount(); iHash++) {
		AESprite* sprite = attachments->getItemByHash(iHash);
		if (sprite->isDead()) {
			attachments->remove(attachments->getHash(iHash));
		}
		else {
			sprite->move(hostdx, hostdy);
			sprite->update();
		}
	}
}

VOID AESprite::renderAttachments() {
	for (INT iHash = 0; iHash < attachments->getHashCount(); iHash++) {
		AESprite* sprite = attachments->getItemByHash(iHash);
		if (!(sprite->isDead())) {
			sprite->render();
		}
	}
}