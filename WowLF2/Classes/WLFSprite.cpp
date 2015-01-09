#include "AEroEngine.h"
#include "WLFAI.h"
#include "WLFAnimation.h"
#include "WLFBuff.h"
#include "WLFSprite.h"
#include "WLFHeadUpDisplay.h"
#include "WLFScene.h"

extern AEConstantTable<AEObject>			ae_ObjectTable;
extern AECamera								ae_Camera;
extern SpriteBatch*							xtk_SpriteBatch;
extern SpriteFont*							xtk_SpriteFont_Arial_7;
extern SpriteFont*							xtk_SpriteFont_Arial_10;


WLFCharacter::WLFCharacter(AERO_SPRITE_DESC desc) : AESprite(desc) {
	portraitIndex = 0;
	targetIndexHash = 0;
	target = nullptr;
	onPlatform = nullptr;
	onPlatformTailIndex = 0;
	attackLock = FALSE;
	hudItems = { 0, 0, 0 };
	createAttachmentTable(10);
	buffTable = new AEHashedTable<WLFBuff>(50);
	shakeTimer = shakeAmp = 0;
}

VOID WLFCharacter::platformCollision(AEPlatform* platform, INT tailNodeIndex, AECollisionResult collisionResult) {
	AESprite::platformCollision(platform, tailNodeIndex, collisionResult);
	onPlatform = platform;
	onPlatformTailIndex = tailNodeIndex;
	state = STATE_ON_GROUND;
	WLFAnimation* anim = (WLFAnimation*)(obj->getAnim(action));
	INT hitGroundAction = anim->getHitGroundAction();
	changeAction(hitGroundAction < 0 ? WLFCharacter::ACTION_LAND_DEFAULT : hitGroundAction);
	ax = 0.0f;
	ay = 0.0f;
	vx = 0.0f;
	vy = 0.0f;
	adsorbToPlatform();
}

// Override AESprite::update() completely
VOID WLFCharacter::update(AEHashedTable<AEPlatform>* platformTable) {
	if (shakeTimer > 0) {
		shakeTimer--;
	}
	if (scene->getStandstill() > 0 && obj->getType() != AEObject::OBJ_HIT_SPARK) {
		return;
	}
	updateBuffTable();
	if (state == STATE_IN_AIR) {
		//changeAction(WLFCharacter::ACTION_IN_AIR);
		onPlatform = nullptr;
		onPlatformTailIndex = 0;
		ay = ((WLFShrineCaveScene*)scene)->GRAVITY;
	}
	if (state == STATE_ON_GROUND) {
		if (action == WLFCharacter::ACTION_STAND) {
			if (isKeyPressed(KEY_RIGHT) && !isKeyPressed(KEY_LEFT)) {
				if (flip == SpriteEffects::SpriteEffects_FlipHorizontally) {
					changeAction(WLFCharacter::ACTION_TURN);
				}
				else {
					changeAction(WLFCharacter::ACTION_WALK);
					flip = SpriteEffects::SpriteEffects_None;
					setVx(2.0f);
				}
			}
			else if (!isKeyPressed(KEY_RIGHT) && isKeyPressed(KEY_LEFT)) {
				if (flip == SpriteEffects::SpriteEffects_None) {
					changeAction(WLFCharacter::ACTION_TURN);
				}
				else {
					changeAction(WLFCharacter::ACTION_WALK);
					flip = SpriteEffects::SpriteEffects_FlipHorizontally;
					setVx(2.0f);
				}
			}
		}
		else if (action == WLFCharacter::ACTION_WALK) {
			if (isKeyPressed(KEY_RIGHT) == isKeyPressed(KEY_LEFT)) {
				changeAction(WLFCharacter::ACTION_STAND);
				setVx(0.0f);
			}
			else if (isKeyPressed(KEY_RIGHT) && !isKeyPressed(KEY_LEFT) && flip == SpriteEffects::SpriteEffects_FlipHorizontally) {
				setVx(0.0f);
				changeAction(WLFCharacter::ACTION_TURN);
			}
			else if (!isKeyPressed(KEY_RIGHT) && isKeyPressed(KEY_LEFT) && flip == SpriteEffects::SpriteEffects_None) {
				setVx(0.0f);
				changeAction(WLFCharacter::ACTION_TURN);
			}
		}
	}
	if (ai) {
		ai->execute();
	}
	WLFAnimation* anim = (WLFAnimation*)(obj->getAnim(action));
	if (timeToLive == 0) {
		changeAction(anim->getNext());
	}
	if (timeToLive > 0) {
		timeToLive--;
	}
	time++;
	BOOLEAN isFrameChange = FALSE;
	INT fac = (flip ? -1 : 1);
	if ((time == 1 && frameNum == 0) || time >= anim->getEndTime(frameNum)) {
		if (time >= anim->getEndTime(frameNum)) {
			isFrameChange = TRUE;
			if (attackLock) attackLock = FALSE;
			frameNum++;
			if (time >= anim->getEndTime(anim->getFrameCount() - 1)) {
				time = 0;
			}
			if (frameNum == anim->getFrameCount()) {
				frameNum = 0;
				if (anim->isTurnAfterAnim()) {
					flip = (flip == SpriteEffects_None ? SpriteEffects_FlipHorizontally : SpriteEffects_None);
				}
				if (!anim->isLoop()) {
					changeAction(anim->getNext());
				}
			}
			vx += anim->getXShift(frameNum);
		}
		WLFSpriteCreatePoint* spriteCreate = anim->getSpriteCreate(frameNum);
		if (spriteCreate != nullptr) {
			spriteCreate->descSprite.obj = ae_ObjectTable.getItem(spriteCreate->oid);
			spriteCreate->descSprite.cx = cx - fac * (getCurrentFrame()->getCenterx() - spriteCreate->x);
			spriteCreate->descSprite.cy = cy - getCurrentFrame()->getCentery() + spriteCreate->y;
			switch (spriteCreate->flip) {
			case WLFSpriteCreatePoint::FLIP_ALWAYS_RIGHT:
				spriteCreate->descSprite.flip = SpriteEffects_None;
				break;
			case WLFSpriteCreatePoint::FLIP_ALWAYS_LEFT:
				spriteCreate->descSprite.flip = SpriteEffects_FlipHorizontally;
				break;
			case WLFSpriteCreatePoint::FLIP_OPPOSITE_TO_HOST:
				spriteCreate->descSprite.flip = (flip == SpriteEffects_None ? SpriteEffects_FlipHorizontally : SpriteEffects_None);
				break;
			default:
				spriteCreate->descSprite.flip = flip;
				break;
			}
			scene->addSprite(new AESprite(spriteCreate->descSprite));
		}
		WLFCameraShakeOptions* cameraShake = anim->getCameraShakeOptions(frameNum);
		if (cameraShake != nullptr) {
			ae_Camera.shake(cameraShake->time, cameraShake->amplitude);
		}
		vx += anim->getGivenSpeed(frameNum).x;
		vy += anim->getGivenSpeed(frameNum).y;	
	}
	if (state == STATE_IN_AIR) {
		FLOAT cx_old = cx, cy_old = cy;
		cx += (fac * vx);
		cy += vy;
		if (platformTable != nullptr) {
			platformCollisionCheck(cx_old, cy_old, platformTable);
		}
	}
	else if (state == STATE_ON_GROUND) {
		cx += (fac * vx) * cosf(onPlatform->getAngleRad(onPlatformTailIndex));
		cy += (fac * vx) * sinf(onPlatform->getAngleRad(onPlatformTailIndex));
		if (cx > onPlatform->getNode(onPlatformTailIndex + 1).x) {
			onPlatformTailIndex++;
			if (onPlatformTailIndex >= onPlatform->getNodeCount() - 1) {
				// TODO: Fall out
			}
			adsorbToPlatform();
		}
		else if (cx < onPlatform->getNode(onPlatformTailIndex).x) {
			onPlatformTailIndex--;
			if (onPlatformTailIndex < 0) {
				// TODO: Fall out
			}
			adsorbToPlatform();
		}
		if (vy < 0.0f) {
			// TODO: Jump out of a platform
			state = STATE_IN_AIR;
		}
	}
	vx += ax;
	vy += ay;
	if (isFrameChange) {
		vx -= anim->getXShift(frameNum);
		isFrameChange = FALSE;
	}
	angle += (fac * vAngle);
	if (angle < -AENSMath::PI) angle += 2.0f * AENSMath::PI;
	if (angle >= AENSMath::PI) angle -= 2.0f * AENSMath::PI;
	angleDisplay += (fac * vAngleDisplay);
	if (angleDisplay < -AENSMath::PI) angleDisplay += 2.0f * AENSMath::PI;
	if (angleDisplay >= AENSMath::PI) angleDisplay -= 2.0f * AENSMath::PI;
}

VOID WLFCharacter::render(INT renderOption, ...) {
	if (shakeTimer > 0) {
		INT xOffset = shakeAmp * (2 * (shakeTimer % 4) / 2 - 1);
		AESprite::render(AESprite::RENDER_OPTION_OFFSET, xOffset, 0);
	}
	else {
		AESprite::render(AESprite::RENDER_OPTION_NORMAL);
	}
}

VOID WLFCharacter::toBattleStance() {
	this->changeAction(WLFCharacter::ACTION_BATTLE_STANCE_START);
}

VOID WLFCharacter::toStand() {
	this->changeAction(WLFCharacter::ACTION_BATTLE_STANCE_END);
}

VOID WLFCharacter::attack_1() {
	this->changeAction(13);
}

VOID WLFCharacter::attack_2() {
	this->changeAction(14);
}

VOID WLFCharacter::changeTarget() {
	AEHashedTable<AESprite>* spriteTable = scene->getSpriteTable();
	if (target == nullptr) {
		for (INT iHash = 0; iHash < spriteTable->getHashCount(); iHash++) {
			AESprite* sprite = spriteTable->getItemByHash(iHash);
			if (dynamic_cast<WLFCharacter*>(sprite) && sprite->getTeam() != this->team) {
				targetIndexHash = iHash;
				target = dynamic_cast<WLFCharacter*>(sprite);
				AERO_SPRITE_DESC descSpr;
				descSpr.obj = ae_ObjectTable.getItem(40);
				descSpr.team = this->team;
				descSpr.action = 6;
				descSpr.flip = SpriteEffects_None;
				descSpr.cx = target->getCx();
				descSpr.cy = target->getCy();
				descSpr.layerDepth = target->getLayerDepth() + 0.001f;
				scene->addSpriteAttachment(target, new AESprite(descSpr));
				//dynamic_cast<WLFShrineCaveScene*>(scene)->addNamepadToHUD(target, 11, WLFShrineCaveScene::NAMEPAD_SLOT_TARGET);
				dynamic_cast<WLFHeadUpDisplay*>(scene->getHUD())->setNamepad(WLFHeadUpDisplay::NAMEPAD_SLOT_TARGET, target);
				break;
			}
		}
	}
	else {
		INT iHash = targetIndexHash;
		do {
			iHash++;
			if (iHash >= spriteTable->getHashCount()) {
				iHash = 0;
			}
			AESprite* sprite = spriteTable->getItemByHash(iHash);
			if (dynamic_cast<WLFCharacter*>(sprite) && sprite->getTeam() != this->team) {
				targetIndexHash = iHash;
				target = dynamic_cast<WLFCharacter*>(sprite);
				AERO_SPRITE_DESC descSpr;
				descSpr.obj = ae_ObjectTable.getItem(40);
				descSpr.team = this->team;
				descSpr.action = 6;
				descSpr.flip = SpriteEffects_None;
				descSpr.cx = target->getCx();
				descSpr.cy = target->getCy();
				descSpr.layerDepth = target->getLayerDepth() + 0.001f;
				scene->addSpriteAttachment(target, new AESprite(descSpr));
				//dynamic_cast<WLFShrineCaveScene*>(scene)->addNamepadToHUD(target, 11, WLFShrineCaveScene::NAMEPAD_SLOT_TARGET);
				dynamic_cast<WLFHeadUpDisplay*>(scene->getHUD())->setNamepad(WLFHeadUpDisplay::NAMEPAD_SLOT_TARGET, target);
				break;
			}
		} while (iHash != targetIndexHash);
	}
}

VOID WLFCharacter::adsorbToPlatform() {
	XMFLOAT2 tail = onPlatform->getNode(onPlatformTailIndex);
	XMFLOAT2 head = onPlatform->getNode(onPlatformTailIndex + 1);
	if (cx == head.x) {
		cy = head.y;
		return;
	}
	else {
		FLOAT ratio = (tail.x - cx) / (head.x - cx);
		cy = (ratio * head.y - tail.y) / (ratio - 1);
	}
}


VOID WLFCharacter::addBuff(WLFBuff* buff, WLFCharacter* caster) {
	BOOLEAN isBuffDuplicated = FALSE;
	for (INT iHash = 0; iHash < buffTable->getHashCount(); iHash++) {
		WLFBuff* oldBuff = buffTable->getItemByHash(iHash);
		if ((oldBuff->getCaster() == caster) && (oldBuff->getName() == buff->getName())) {
			oldBuff->setTimeRemain(buff->getTimeRemain());
			isBuffDuplicated = TRUE;
			break;
		}
	}
	if (!isBuffDuplicated) {
		buffTable->add(buff);
		buff->setHost(this);
		buff->setCaster(caster);
	}
}

VOID WLFCharacter::updateBuffTable() {
	for (INT iHash = 0; iHash < buffTable->getHashCount(); iHash++) {
		WLFBuff* buff = buffTable->getItemByHash(iHash);
		if (buff->isDead()) {
			buffTable->removeItemByHash(iHash);
		}
		else {
			buff->update();
		}
	}
}

VOID WLFCharacter::shake(INT time, INT amplitude) {
	shakeTimer = time;
	shakeAmp = amplitude;
}


WLFWarrior::WLFWarrior(AERO_SPRITE_DESC desc) : WLFCharacter(desc) {
	chargeTargetPosX = 0.0f;
	rage = 0;
	rageMax = 100;
	portraitIndex = 10;
}

VOID WLFWarrior::update(AEHashedTable<AEPlatform>* platformTable) {
	if (action == 10 && fabs(cx - target->getCx()) < 40.0f) {
		changeAction(12);
		setVx(0.0f);
	}
	WLFCharacter::update(platformTable);
}

VOID WLFWarrior::mortalStrike() {
	changeAction(15);
}

VOID WLFWarrior::overpower() {
	changeAction(AENSMath::randomIntBetween(13, 14));
}

VOID WLFWarrior::slam() {
	changeAction(19);
}

VOID WLFWarrior::colossusSmash() {
	changeAction(16);
}

VOID WLFWarrior::thunderClap() {
	changeAction(17);
}

VOID WLFWarrior::charge() {
	if (fabs(cx - target->getCx()) < 120.0f) {
		return;
	}
	changeAction(11);
	chargeTargetPosX = target->getCx();
	if (cx < chargeTargetPosX && flip == SpriteEffects_FlipHorizontally) {
		flip = SpriteEffects_None;
	}
	else if (cx > chargeTargetPosX && flip == SpriteEffects_None) {
		flip = SpriteEffects_FlipHorizontally;
	}
	setVx(5.0f);
}


WLFBar::WLFBar(AERO_SPRITE_DESC desc, WLFCharacter* _host, INT _type) : AESprite(desc) {
	host = _host;
	type = _type;
}

VOID WLFBar::update(AEHashedTable<AEPlatform>* platformTable) {

}

VOID WLFBar::render(INT renderOption, ...) {
	LPTSTR strBarValue = new TCHAR[1024];
	switch (type) {
	case TYPE_HP:
		AESprite::render(AESprite::RENDER_OPTION_WIPE, AESprite::RENDER_WIPE_LEFT, host->getHPProportion());
		wsprintf(strBarValue, L"%d / %d", host->getHP(), host->getHPMax());
		break;
	case TYPE_ENERGY:
		if (dynamic_cast<WLFWarrior*>(host)) {
			AESprite::render(AESprite::RENDER_OPTION_WIPE, AESprite::RENDER_WIPE_LEFT, dynamic_cast<WLFWarrior*>(host)->getRageProportion());
			wsprintf(strBarValue, L"%d / %d", dynamic_cast<WLFWarrior*>(host)->getRage(), dynamic_cast<WLFWarrior*>(host)->getRageMax());
		}
		else {
			wsprintf(strBarValue, L"");
		}
		break;
	default:

		break;
	}
	AEFrame* f = getCurrentFrame();
	xtk_SpriteFont_Arial_7->DrawString(xtk_SpriteBatch, strBarValue, XMFLOAT2(cx - f->getCenterx(), cy - f->getCentery() - 2.0f), XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
}


WLFBuffIcon::WLFBuffIcon(AERO_SPRITE_DESC desc, WLFBuff* _buff) : AESprite(desc) {
	buff = _buff;
}

VOID WLFBuffIcon::render(INT renderOption, ...) {
	AESprite::render(AESprite::RENDER_OPTION_SCALE, 0.25f);
	LPTSTR strBuffTimeRemain = new TCHAR[1024];
	CHAR* narrowCharBuffer = new CHAR[1024];
	INT sec = buff->getTimeRemain() / 60;
	sprintf_s(narrowCharBuffer, 1024, "%s %02d:%02d", buff->getName().c_str(), sec / 60, sec % 60);
	mbstowcs(strBuffTimeRemain, narrowCharBuffer, 1024);
	xtk_SpriteFont_Arial_10->DrawString(xtk_SpriteBatch, strBuffTimeRemain, XMFLOAT2(cx + 10.0f, cy - 8.0f), XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f));
}