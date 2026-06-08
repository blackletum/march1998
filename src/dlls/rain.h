/***
*
*	2023, Magic Nipples.
*
*	Use and modification of this code is allowed as long
*	as credit to the original creators is provided! Enjoy!
*
****/


#ifndef RAIN_H
#define RAIN_H


//=========================================================
// G-Cont - env_rain, use triAPI
//=========================================================
class CEnvWeather : public CBaseEntity
{
public:
	void	Spawn(void);
	void	KeyValue(KeyValueData* pkvd);

	int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	static	TYPEDESCRIPTION m_SaveData[];

	int Rain_Drips;
	float Rain_windX, Rain_windY;
	float Rain_randX, Rain_randY;
	float fadeTime;
	float Rain_Distance;
	int Rain_Mode;
};

#endif