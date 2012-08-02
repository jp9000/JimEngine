/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  Script

  Copyright (c) 2009, Hugh Bailey
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
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


#ifndef SCRIPTBYTECODE_HEADER
#define SCRIPTBYTECODE_HEADER


enum ByteCodeInstruction
{
    BCNop,

    BCPush,         //in: [int size][value], push (value)
    BCPop,          //in: [int size], pop value from the stack and into thin air

    BCPushDup,      //in: [int size], push (duplicated stack val)
    BCPushDupInit,  //in: [string type][string subType], push (duplicated stack val)

    BCAPushThis,    //apush: [curObj*]

    BCAPushDup,     //apush: [address]
    BCAPush,        //in: [int scope][int pos][int arrayOffset], apush: [address]
    BCAPop,         //apop: [address]
    BCAPushSVar,    //in: [int pos][bool istempstack][int arrayOffset], apop: [struct*], apush: [address]
    BCAPushCVar,    //in: [int pos][bool istempstack][int arrayOffset], apop: [obj**], apush: [address]
    BCAPushListItem,//in: [string subType], apop: [obj**], pop: [element], apush: [address]
    BCPushAData,    //in: [int size][bool noremove], apop: [address], push: [data]
    BCPushADataInit,//in: [string type][string subType][bool noremove], apop: [address], push: [data]
    BCAPopVarInit,  //in: [string type][string subType], apop: [address], pop: [data], out (data)
    BCAPopVar,      //in: [int size], apop: [address], pop: [data], out (data)
    BCAPushStack,   //apush: [address to data stack]

    BCTPushStack,   //in: [int size], tpush [data], pop: [data]
    BCTPushZero,    //in: [int size], tpush (value)
    BCTPop,         //in: [int size], tpop: [data]
    BCTPopFree,     //in: [string type][string subType], tpop: [data]
    BCPushTData,    //in: [int size], tpop: [data], push [data]
    BCAPushT,       //apush: [address to temp data stack]

    BCDynamicCast,  //in: [string classname], alter stack: [curObj* or null if uncastable]

    BCCastFloat,    //alter stack: [int val], (float)val;
    BCCastInt,      //alter stack: [float val], (int)val;

    BCNegate,       //alter stack: [int val], -val
    BCNegateF,      //alter stack: [float val], -val

    BCCallFunc,     //in: [int funcID], pop: [paramN-1, paramN-2, ...], tpush (return value)
    BCCallStructOp, //in: [string struct][func funcID], pop: [paramN-1, paramN-2, ...], apop: [struct*], tpush (return value)
    BCCallStructFunc,//in: [string struct][func funcID], pop: [paramN-1, paramN-2, ...], apop: [struct*], tpush (return value)
    BCCallTypeFunc,
    BCCallClassStFn,//in: [string class][int funcID], pop: [paramN-1, paramN-2, ...], tpush (return value)
    BCCallClassFunc,//in: [int funcID][bool bSuper], pop: [paramN-1, paramN-2, ...], apop: [obj**], tpush (return value)

    BCCreateObj,    //in: [string className], tpush: [obj]
    BCDestroyObj,   //pop: [Object *obj]

    BCReturn,       //in: [int size], pop: [value]

    BCJump,         //in: [int offset]
    BCJumpIf,       //in: [int offset], pop: [bool value]
    BCJumpIfNot,    //in: [int offset], pop: [bool value]

    BCMultiply,     //pop: [int param2, int param1], push (param1*param2)
    BCDivide,       //pop: [int param2, int param1], push (param1/param2)
    BCAdd,          //pop: [int param2, int param1], push (param1+param2)
    BCSubtract,     //pop: [int param2, int param1], push (param1-param2)
    BCMod,          //pop: [int param2, int param1], push (param1%param2)
                                                 
    BCEqual,        //pop: [int param2, int param1], push (param1 == param2)
    BCNotEqual,     //pop: [int param2, int param1], push (param1 != param2)
    BCGreaterEqual, //pop: [int param2, int param1], push (param1 >= param2)
    BCLessEqual,    //pop: [int param2, int param1], push (param1 <= param2)
    BCGreater,      //pop: [int param2, int param1], push (param1 > param2)
    BCLess,         //pop: [int param2, int param1], push (param1 < param2)

    BCMultiplyF,    //pop: [float param2, float param1], push (param1*param2)
    BCDivideF,      //pop: [float param2, float param1], push (param1/param2)
    BCAddF,         //pop: [float param2, float param1], push (param1+param2)
    BCSubtractF,    //pop: [float param2, float param1], push (param1-param2)
    BCModF,         //pop: [float param2, float param1], push (param1%param2)

    BCEqualF,       //pop: [float param2, float param1], push (param1 == param2)
    BCNotEqualF,    //pop: [float param2, float param1], push (param1 != param2)
    BCGreaterEqualF,//pop: [float param2, float param1], push (param1 >= param2)
    BCLessEqualF,   //pop: [float param2, float param1], push (param1 <= param2)
    BCGreaterF,     //pop: [float param2, float param1], push (param1 > param2)
    BCLessF,        //pop: [float param2, float param1], push (param1 < param2)

    BCPushNullObj,  //push (null obj)
    BCPushNullStr,  //push (null str)
                                                 
    BCEqualO,       //pop: [Object* param2, Object* param1], push (param1 == param2)
    BCNotEqualO,    //pop: [Object* param2, Object* param1], push (param1 != param2)

    BCBitwiseNot,   //alter stack: [int val], ~val
    BCBitwiseOr,    //pop: [int param2, int param1], push (param1 | param2)
    BCBitwiseAnd,   //pop: [int param2, int param1], push (param1 & param2)
    BCShiftLeft,    //pop: [int param2, int param1], push (param1 << param2)
    BCShiftRight,   //pop: [int param2, int param1], push (param1 >> param2)

    BCLogicalNot,   //alter stack: [int val], !val

    BCReturnS,      //pop: [String str]

    BCPushS,        //in: [String str], push (value)
    BCPopS,         //pop: [String str]
    BCPushDupS,     //push (dup string)

    BCPushADataS,   //in: [bool noremove], apop: [address], push: [data]
    BCAPopVarS,     //apop: [address], pop: [data], out (data)
    BCPushTDataS,   //tpop: [data], push [data]
    BCTPopS,        //tpop: [string]

    BCAddS,         //pop: [String string2, String string1], push (string1+string2)
    BCEqualS,       //pop: [String string2, String string1], push (string1.Compare(string2))
    BCNotEqualS,    //pop: [String string2, String string1], push (string1.CompareI(string2))
};



#endif

