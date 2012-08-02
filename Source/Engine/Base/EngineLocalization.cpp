

#include "Base.h"
#include "ScriptDefs.h"
#include "ScriptCompiler.h"


LocaleStringLookup *locale=NULL;


struct StringLookupNode
{
    String str;
    List<StringLookupNode*> subNodes;
    LocaleStringItem *leaf;

    inline ~StringLookupNode()
    {
        for(int i=0; i<subNodes.Num(); i++)
            delete subNodes[i];
    }

    inline StringLookupNode* FindSubNodeByChar(TCHAR ch)
    {
        for(int i=0; i<subNodes.Num(); i++)
        {
            StringLookupNode *node = subNodes[i];
            if(node->str.IsValid() && node->str[0] == ch)
                return subNodes[i];
        }

        return NULL;
    }

    inline UINT FindSubNodeID(CTSTR lpLookup)
    {
        for(int i=0; i<subNodes.Num(); i++)
        {
            StringLookupNode *node = subNodes[i];
            if(scmpi_n(node->str, lpLookup, node->str.Length()) == 0)
                return i;
        }

        return INVALID;
    }

    inline StringLookupNode* FindSubNode(CTSTR lpLookup)
    {
        for(int i=0; i<subNodes.Num(); i++)
        {
            StringLookupNode *node = subNodes[i];
            if(scmpi_n(node->str, lpLookup, node->str.Length()) == 0)
                return subNodes[i];
        }

        return NULL;
    }
};


LocaleStringLookup::LocaleStringLookup()
{
    top = new StringLookupNode;
    curLanguage = AppConfig->GetString(TEXT("Engine"), TEXT("Language"), TEXT("en"));

    locale = this;
}

LocaleStringLookup::~LocaleStringLookup()
{
    for(int i=0; i<loadedFiles.Num(); i++)
        loadedFiles[i].FreeData();
    delete top;

    locale = NULL;
}


BOOL LocaleStringLookup::AddLookup(CTSTR lookupVal, LocaleStringItem *item, StringLookupNode *node)
{
    if(!node) node = top;

    if(!lookupVal || !*lookupVal)
        return FALSE;

    StringLookupNode *child = node->FindSubNodeByChar(*lookupVal);

    if(child)
    {
        UINT len;

        for(len=0; len<child->str.Length(); len++)
        {
            TCHAR val1 = child->str[len],
                  val2 = lookupVal[len];

            if((val1 >= 'A') && (val1 <= 'Z'))
                val1 += 0x20;
            if((val2 >= 'A') && (val2 <= 'Z'))
                val2 += 0x20;

            if(val1 != val2)
                break;
        }

        if(len == child->str.Length())
            return AddLookup(lookupVal+len, item, child);
        else
        {
            StringLookupNode *childSplit = new StringLookupNode;
            childSplit->str = child->str.Array()+len;
            childSplit->leaf = child->leaf;
            childSplit->subNodes.TransferFrom(child->subNodes);

            child->leaf = NULL;
            child->str.SetLength(len);

            child->subNodes << childSplit;

            if(lookupVal[len] != 0)
            {
                StringLookupNode *newNode = new StringLookupNode;
                newNode->leaf = item;
                newNode->str  = lookupVal+len;

                child->subNodes << newNode;
            }
            else
                child->leaf = item;
        }
    }
    else
    {
        StringLookupNode *newNode = new StringLookupNode;
        newNode->leaf = item;
        newNode->str  = lookupVal;

        node->subNodes << newNode;
    }

    return TRUE;
}

void LocaleStringLookup::RemoveLookup(CTSTR lookupVal, StringLookupNode *node)
{
    if(!node) node = top;

    UINT childID = node->FindSubNodeID(lookupVal);
    if(childID == INVALID)
    {
        assert(0);
        return;
    }

    StringLookupNode *child = node->subNodes[childID];

    lookupVal += child->str.Length();
    TCHAR ch = *lookupVal;
    if(ch)
        RemoveLookup(lookupVal, child);

    if(!ch)
    {
        if(!child->subNodes.Num())
        {
            node->subNodes.Remove(childID);
            delete child;
        }
        else
            child->leaf = NULL;
    }
    else if(!child->subNodes.Num() && !child->leaf)
    {
        node->subNodes.Remove(childID);
        delete child;
    }

    //if not a leaf node and only have one child node, then merge with child node
    if(!node->leaf && node->subNodes.Num() == 1)
    {
        node->str += node->subNodes[0]->str;
        node->leaf = node->subNodes[0]->leaf;
        delete node->subNodes[0];
        node->subNodes.Clear();
    }
}

UINT LocaleStringLookup::GetLocaleFileID(CTSTR lpResource)
{
    traceIn(LocaleStringLookup::GetLocaleFileID);

    for(int i=0; i<loadedFiles.Num(); i++)
    {
        if(loadedFiles[i].name.CompareI(lpResource))
            return i;
    }

    return INVALID;

    traceOut;
}

//ugh yet more string parsing, you think you escape it for one minute and then bam!  you discover yet more string parsing code needs to be written
BOOL LocaleStringLookup::LoadStringFile(CTSTR lpResource)
{
    traceIn(LocaleStringLookup::LoadStringFile);

    if(GetLocaleFileID(lpResource) != INVALID)
        return FALSE;

    //------------------------

    XFile file;
    String filePath;
    engine->ConvertResourceName(lpResource, TEXT("strings"), filePath);

    if(!file.Open(filePath, XFILE_READ, XFILE_OPENEXISTING))
        return FALSE;

    String fileString;
    file.ReadFileToString(fileString);
    file.Close();

    if(fileString.IsEmpty())
        return FALSE;

    LocaleStringFile *resource = loadedFiles.CreateNew();
    resource->name = lpResource;

    LocaleStringCache &cache = resource->cache;

    //------------------------

    fileString.FindReplace(TEXT("\r"), TEXT(" "));

    TSTR lpTemp = fileString.Array()-1;
    TSTR lpNextLine;

    do
    {
        ++lpTemp;
        lpNextLine = schr(lpTemp, '\n');

        while(*lpTemp == ' ' || *lpTemp == L'@' || *lpTemp == '\t')
            ++lpTemp;

        if(!*lpTemp || *lpTemp == '\n') continue;

        if(lpNextLine) *lpNextLine = 0;

        //----------

        TSTR lpValueStart = lpTemp;
        while(*lpValueStart && *lpValueStart != ' ' && *lpValueStart != L'@' && *lpValueStart != '\t')
            ++lpValueStart;

        LocaleStringItem *item = new LocaleStringItem;

        TCHAR prevChar = *lpValueStart;
        *lpValueStart = 0;
        item->lookup = lpTemp;
        *lpValueStart = prevChar;

        String value = lpValueStart;
        value.KillSpaces();
        if(value.IsValid() && value[0] == '"')
        {
            CodeTokenizer ct;
            ct.SetCodeStart(value);

            do
            {
                if(!ct.GetNextToken(value)) break;

                value = String::RepresentationToString(value);
                item->strings << value;

                if(!ct.GetNextToken(value)) break;
            }while(value[0] == ',');
        }
        else
            item->strings << value;

        cache << item;

        //----------

        if(lpNextLine) *lpNextLine = '\n';
    }while(lpTemp = lpNextLine);

    //------------------------

    for(int i=0; i<cache.Num(); i++)
        AddLookup(cache[i]->lookup, cache[i]);

    return TRUE;

    traceOut;
}

void LocaleStringLookup::UnloadStringFile(CTSTR lpResource)
{
    traceIn(LocaleStringLookup::UnloadStringFile);

    UINT index = GetLocaleFileID(lpResource);
    if(index == INVALID)
        return;

    LocaleStringFile &resource = loadedFiles[index];
    LocaleStringCache &cache = resource.cache;

    for(int i=0; i<cache.Num(); i++)
        RemoveLookup(cache[i]->lookup);

    resource.FreeData();
    loadedFiles.Remove(index);

    traceOut;
}


CTSTR LocaleStringLookup::LookupString(CTSTR lookupVal, StringLookupNode *node)
{
    traceIn(LocaleStringLookup::LookupString);

    if(!node) node = top;

    StringLookupNode *child = node->FindSubNode(lookupVal);
    if(!child)
        return TEXT("(string not found)");

    lookupVal += child->str.Length();
    TCHAR ch = *lookupVal;
    if(ch)
        return LookupString(lookupVal, child);

    if(!child->leaf)
        return TEXT("(string not found)");

    return child->leaf->strings[0];

    traceOut;
}

BOOL LocaleStringLookup::LookupStringSequence(CTSTR lookupVal, List<CTSTR> &strSequence, StringLookupNode *node)
{
    traceIn(LocaleStringLookup::LookupStringSequence);

    if(!node) node = top;

    StringLookupNode *child = node->FindSubNode(lookupVal);
    if(!child)
        return FALSE;

    lookupVal += child->str.Length();
    TCHAR ch = *lookupVal;
    if(ch)
        return LookupStringSequence(lookupVal, strSequence, child);

    if(!child->leaf)
        return FALSE;

    strSequence.Clear();
    for(int i=0; i<child->leaf->strings.Num(); i++)
        strSequence << child->leaf->strings[i].Array();

    return TRUE;

    traceOut;
}

