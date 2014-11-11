#pragma once


class WLFSpell {

public:

	WLFSpell();

	VOID setCaster(WLFCharacter* _caster) { caster = _caster; }
	VOID setTarget(WLFCharacter* _target) { target = _target; }

	virtual VOID takeEffect() = 0;

	static WLFSpell* getSpellByID(INT spellID);

protected:

	WLFCharacter* caster;
	WLFCharacter* target;

};


class WLFMortalStrike : public WLFSpell {

public:

	VOID takeEffect();

	static const INT spellID = 1;

};


class WLFOverpower : public WLFSpell {

public:

	VOID takeEffect();

	static const INT spellID = 2;

};