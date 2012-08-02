/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ParticleEmitter.h

  Copyright (c) 2001-2007, Hugh Bailey
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * The name of Hugh Bailey may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

  THIS SOFTWARE IS PROVIDED BY HUGH BAILEY "AS IS" AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED.  IN NO EVENT SHALL HUGH BAILEY BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUISNESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  Blah blah blah blah.  To the fricken' code already!
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#ifndef PARTICLEEMITTER_HEADER
#define PARTICLEEMITTER_HEADER



/*======================================================================
  ParticleEmitter Class Declaration
=======================================================================*/

/*class BASE_EXPORT ParticleEmitter : public Entity
{
    //------------------------------------------------------------------
    //  This macro is used to declare class data automatically,
    //    used in combination with DefineClass.
    //
    //  Use: DeclareClass(This class, Parent class);
    //
    DeclareClass(ParticleEmitter, Entity);

    DWORD curTime;              //how much time has gone by since the last
                                //particle was created


public:

    //----------------------------------------------------------------------
    BeginScriptVars(ParticleEmitter)
        String ParticleClass;               //Class to be used for each particle
        String ParticleTexture;             //sprite texture name
        DWORD  ParticleColor;               //color
        float  ParticleSize;                //size
        float  ParticleSpeed;               //speed
        float  ParticleRotation;            //rotation
        float  ParticleRotationRate;        //rotation rate
        float  ParticleFriction;            //friction
        float  ParticleMass;                //mass (can be negative as well)
        float  ParticleGrowth;              //growth (units per second)
        DWORD  ParticleLifetime;            //life time
        DWORD  ParticleSpawnTime;           //Spawn time between particles

        float  ParticleRandSpeed;           //Maximum particle speed randomization
        float  ParticleRandRotation;        //Maximum particle rotation randomization
        float  ParticleRandRotationRate;    //Maximum particle rotation rate randomization
        float  ParticleRandGrowth;          //Maximum particle growth randomization
        DWORD  ParticleRandLifetime;        //Maximum particle life time randomization
        DWORD  ParticleRandDirection;       //0-100% randomizing of direction
        DWORD  ParticleRandSpawnTime;       //Maximum spawn time randomization

        BOOL   bFade;                       //Whether to have particle fade out
    EndScriptVars()

    ParticleEmitter();

    virtual void Init();
    virtual void Destroy();
    virtual void Reinitialize();

    virtual void Tick(DWORD dwMS, float fSeconds);


    Texture *sprite;                //sprite texture
    DWORD curSpawnTime;             //Spawn time, with randomization
};
*/


#endif

