#pragma once


class BulletAI : public AEAI {
public:
	BulletAI(AESprite* _host);
	VOID execute();
private:

};


class BigSmokeEffectUnivAI : public AEAI {
public:
	BigSmokeEffectUnivAI(AESprite* _host);
	VOID execute();
private:
	INT smokeCount;
};


class HomingAI : public AEAI {
public:
	HomingAI(AESprite* _host, AESprite* _target);
	VOID execute();
private:
	AESprite* target = nullptr;
	BOOLEAN isLocked = FALSE;
};


class TurretBaseAI : public AEAI {
public:
	TurretBaseAI(AESprite* _host);
	
	VOID execute();
private:

};


class TurretAI : public AEAI {
public:
	TurretAI(AESprite* _host);
	AESprite* getTarget() { return target; }
	VOID execute();
private:
	INT rocketCooldown = 0, homingFired = 0;;
	AESprite* target = nullptr;
};


class FlakCannonAI : public AEAI {
public:
	FlakCannonAI(AESprite* _host);
	VOID execute();
private:
	INT flakTimer = 0;
	FLOAT pitchAngleMax, pitchAngleMin;
	AESprite* target = nullptr;
};