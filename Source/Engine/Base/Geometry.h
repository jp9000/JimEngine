/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Geometry.h

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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#ifndef GEOMETRY_HEADER
#define GEOMETRY_HEADER


//-----------------------------------------
// Triangle
struct Face
{
    union
    {
        struct{DWORD A,B,C;};
        DWORD ptr[3];
    };

    inline Face& Set(DWORD newA, DWORD newB, DWORD newC)
    {
        A = newA;
        B = newB;
        C = newC;

        return *this;
    }
};


//-----------------------------------------
// Edge
struct Edge
{
    union
    {
        struct
        {
            DWORD v1;
            DWORD v2;
        };
        DWORD vptr[2];
    };
    union
    {
        struct
        {
            DWORD f1;
            DWORD f2;
        };
        DWORD fptr[2];
    };

    inline BOOL EdgeConnects(DWORD e2v1, DWORD e2v2) const
    {
        return ((e2v1 == v1) && (e2v2 == v2)) ||
               ((e2v1 == v2) && (e2v2 == v1));
    }

    inline BOOL EdgeConnects(const Edge &edge2) const
    {
        return ((edge2.v1 == v1) && (edge2.v2 == v2)) ||
               ((edge2.v1 == v2) && (edge2.v2 == v1));
    }
};


//-----------------------------------------
// PolyFace
struct PolyFace
{
    List<DWORD> Faces;
};


//-----------------------------------------
// SimpleEdge
struct SimpleEdge
{
    DWORD v1, v2;

    inline BOOL operator==(const SimpleEdge &e2) const
    {
        return ((e2.v1 == v1) && (e2.v2 == v2)) ||
               ((e2.v1 == v2) && (e2.v2 == v1));
    }
};


//-----------------------------------------
// PolyEdge
struct PolyEdge
{
    DWORD v1, v2;
    DWORD f1, f2;
    DWORD f1e, f2e;  //tells which edges on the polys that are connecting

    inline BOOL EdgeConnects(DWORD e2v1, DWORD e2v2) const
    {
        return ((e2v1 == v1) && (e2v2 == v2)) ||
               ((e2v1 == v2) && (e2v2 == v1));
    }
};


//-----------------------------------------
// Poly
struct Poly
{
    inline Poly(const Face &face)
    {
        verts.Add(face.A);
        verts.Add(face.B);
        verts.Add(face.C);
    }

    inline Poly(DWORD v1, DWORD v2, DWORD v3)
    {
        verts.Add(v1);
        verts.Add(v2);
        verts.Add(v3);
    }

    inline Poly(List<DWORD> &newVerts)
    {
        verts.CopyList(newVerts);
    }

    List<DWORD> verts;
};


//-----------------------------------------
// Polyhedron (convex space defined by planes)
struct Polyhedron
{
    inline ~Polyhedron()
    {
        Clear();
    }

    inline void Clear()
    {
        for(DWORD i=0; i<Polys.Num(); i++)
            Polys[i].verts.Clear();
        Polys.Clear();
    }

    inline void CopyList(const Polyhedron &PH)
    {
        for(DWORD i=0; i<PH.Polys.Num(); i++)
            Polys.CreateNew()->verts.CopyList(PH.Polys.Array()[i].verts);
    }

    inline void BuildEdges(List<PolyEdge> &EdgeList) const
    {
        EdgeList.Clear();

        for(DWORD i=0; i<Polys.Num(); i++)
        {
            Poly &poly = Polys.Array()[i];

            for(DWORD j=0; j<poly.verts.Num(); j++)
            {
                DWORD jp1 = (j == (poly.verts.Num()-1)) ? 0 : j+1;
                BOOL bFoundEdge = 0;
                PolyEdge *testEdge = NULL;

                DWORD v1 = poly.verts[j];
                DWORD v2 = poly.verts[jp1];

                for(DWORD k=0; k<EdgeList.Num(); k++)
                {
                    testEdge = &EdgeList[k];

                    if(testEdge->EdgeConnects(v1, v2))
                    {
                        bFoundEdge = 1;
                        break;
                    }
                }

                if(bFoundEdge)
                {
                    testEdge->f2 = i;
                    testEdge->f2e = j;
                }
                else
                {
                    PolyEdge newEdge = {v1, v2, i, INVALID, j, INVALID};
                    EdgeList.Add(newEdge);
                }
            }
        }
    }

    List<Poly> Polys;
};


#endif
