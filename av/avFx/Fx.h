#pragma once

//
//  FoamSreamFx interface
//

// base info
struct particles_effect_info
{
	virtual float getMaxParticleLifetime() const = 0;
	virtual bool  isQueueEmpty() const = 0;
};

// FoamStream
struct FoamStreamSfxNode : particles_effect_info
{
	virtual void  setIntensity( float inten ) = 0;
	virtual float getIntensity() const = 0;

	virtual void setEmitWorldDir( cg::point_3f const & dir ) = 0;
	virtual cg::point_3f const & getEmitWorldDir() const = 0;

	virtual void setEmitterWorldSpeed( cg::point_3f const & speed ) = 0;
	virtual cg::point_3f const & getEmitterWorldSpeed() const = 0;

	virtual void setFactor( float factor ) = 0;
	virtual float getFactor() const = 0;
};

//
//  SmokeFx interface
//

// Smoke
struct SmokeSfxNode : particles_effect_info
{
	virtual void  setIntensity( float inten ) = 0;
	virtual float getIntensity() const = 0;

	virtual void setEmitWorldDir( cg::point_3f const & dir ) = 0;
	virtual cg::point_3f const & getEmitWorldDir() const = 0;

	virtual void setEmitterWorldSpeed( cg::point_3f const & speed ) = 0;
	virtual cg::point_3f const & getEmitterWorldSpeed() const = 0;

	virtual void setFactor( float factor ) = 0;
	virtual float getFactor() const = 0;
};


// Sparks
struct SparksSfxNode : particles_effect_info
{
	virtual void setContactFlag( bool flag ) = 0;
	virtual bool getContactFlag() const = 0;


	virtual void setEmitterWorldSpeed( cg::point_3f const & speed ) = 0;
	virtual cg::point_3f const & getEmitterWorldSpeed() const = 0;

	virtual void setFactor( float factor ) = 0;
	virtual float getFactor() const = 0;
};

// Friction dust
struct FrictionDustSfxNode : particles_effect_info
{
	virtual void setContactFlag( bool flag ) = 0;
	virtual bool getContactFlag() const = 0;

	virtual void setEmitterWorldSpeed( cg::point_3f const & speed ) = 0;
	virtual cg::point_3f const & getEmitterWorldSpeed() const = 0;
};

// Landing dust
struct LandingDustSfxNode : particles_effect_info
{
	// inject particles
	virtual void makeContactDust( float timestamp, cg::point_3f const & contact_pos, cg::point_3f const & contact_speed ) = 0;
};