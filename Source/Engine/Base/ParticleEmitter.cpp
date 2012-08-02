/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  ParticleEmitter.cpp

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

//#include "Base.h"


/*======================================================================
  ParticleEmitter Class Macros
=======================================================================*/

//----------------------------------------------------------------------
//  This macro is used to define class data automatically for
//    each class
//DefineClass(ParticleEmitter);

/*======================================================================
  ParticleEmitter Function Definitions
=======================================================================*/

/*ParticleEmitter::ParticleEmitter()
{
    ParticleClass           = TEXT("SpriteParticle");
    ParticleTexture         = TEXT("Base:Default/default_particle.tga");
    ParticleColor           = Color_White;

    ParticleSize            = 5.0f;
    ParticleSpeed           = 20.0f;
    ParticleFriction        = 0.0f;
    ParticleRotation        = 0.0f;
    ParticleRotationRate    = 0.0;
    ParticleMass            = 3.0f;
    ParticleGrowth          = 0.0f;
    ParticleLifetime        = 1000;
    ParticleSpawnTime       = 400;

    ParticleRandSpeed       = 0.0f;
    ParticleRandRotation    = 0.0f;
    ParticleRandRotationRate= 0.0f;
    ParticleRandGrowth      = 0.0f;
    ParticleRandLifetime    = 0;
    ParticleRandDirection   = 0;
    ParticleRandSpawnTime   = 0;

    bFade = TRUE;
}

void ParticleEmitter::Init()
{
    Super::Init();

    curSpawnTime = ParticleSpawnTime;

    if(!ParticleTexture.IsEmpty())
        sprite = GetTexture(ParticleTexture);
}

void ParticleEmitter::Destroy()
{
    if(sprite)
        ReleaseTexture(sprite);

    Super::Destroy();
}

void ParticleEmitter::Reinitialize()
{
    this->Destroy();
    this->Init();
}


void ParticleEmitter::Tick(DWORD dwTicks, float fSeconds)
{
    Super::Tick(dwTicks, fSeconds);

    curTime += dwTicks;

    if(dwTicks > 2000)
    {
        curTime = 0;
    }

    if(ParticleClass && (curTime >= curSpawnTime))
    {
        Particle *newParticle = (Particle*)CreateFactoryObject(ParticleClass, FALSE);
        //newParticle->Attach(this);
        newParticle->SetPos(GetWorldPos());
        newParticle->UpdatePositionalData();
        newParticle->mass = ParticleMass;
        newParticle->curSize = ParticleSize;
        //newParticle->bVisible = TRUE;

        Vect dir(0.0f, 0.0f, -1.0f);
        if(ParticleRandDirection)
        {
            float fRand;
            Vect randDir;

            if(ParticleRandDirection > 100)
                ParticleRandDirection = 100;

            float fMul = ((float)ParticleRandDirection)*0.01;

            fRand = ((((float)rand())/32767.0f)*2.0f)-1.0f;
            randDir.x = fRand;

            fRand = ((((float)rand())/32767.0f)*2.0f)-1.0f;
            randDir.y = fRand;

            fRand = ((((float)rand())/32767.0f)*2.0f)-1.0f;
            randDir.z = fRand;

            dir *= 1.0f-fMul;
            dir += (randDir*fMul);

            dir.Norm();
        }

        newParticle->Vel = (dir*(ParticleSpeed));
        if(ParticleRandSpeed)
        {
            float fRand = ((float)rand())/32767.0f;
            newParticle->Vel.z -= (fRand*ParticleRandSpeed);
        }

        newParticle->endTime = ParticleLifetime;
        if(ParticleRandLifetime)
            newParticle->endTime += (rand()%ParticleRandLifetime);

        newParticle->growth = ParticleGrowth;
        if(ParticleRandGrowth)
        {
            float fRand = ((float)rand())/32767.0f;
            newParticle->growth += (fRand*ParticleRandGrowth);
        }

        newParticle->rotAngle = ParticleRotation;
        if(ParticleRandRotation != 0.0f)
        {
            float fRand = ((float)rand())/32767.0f;
            newParticle->rotAngle += (fRand*ParticleRandRotation);
        }

        newParticle->rotRate = ParticleRotationRate;
        if(ParticleRandRotationRate != 0.0f)
        {
            float fRand = ((float)rand())/32767.0f;
            fRand = (fRand-0.5f)*2.0f;
            newParticle->rotRate += (fRand*ParticleRandRotationRate);
        }

        newParticle->friction = ParticleFriction;
        newParticle->bFade = bFade;
        newParticle->sprite = sprite;
        newParticle->color = ParticleColor;

        Matrix PSTransform(GetLocalRot());
        newParticle->Vel.TransformVector(PSTransform.Transpose());

        curTime = 0;

        curSpawnTime = ParticleSpawnTime;
        if(ParticleRandSpawnTime)
            curSpawnTime += (rand()%ParticleRandSpawnTime);

        newParticle->Init();
    }
}
*/
