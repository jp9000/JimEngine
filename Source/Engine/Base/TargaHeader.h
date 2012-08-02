
#pragma pack(push, 1)

struct TargaHeader
{
    BYTE IDSize;
    BYTE bColorMapped;
    BYTE ImageType:3;
    BYTE bUsesRLE:1;
    BYTE bVoid1:4;
    char z1[5];
    WORD xOrigin;
    WORD yOrigin;
    WORD width;
    WORD height;
    BYTE depth;
    BYTE nAttributes:4;
    BYTE PixelOrder:2;
    BYTE z2:2;
};

#pragma pack(pop)

