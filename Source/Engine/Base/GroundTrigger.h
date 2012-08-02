/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  GroundTrigger

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#ifndef GROUNDTRIGGER_HEADER
#define GROUNDTRIGGER_HEADER


class BASE_EXPORT GroundTrigger : public Trigger
{
    DeclareClass(GroundTrigger, Trigger);

    List<Poly> ConvexPolygons;

public:
};


#endif
