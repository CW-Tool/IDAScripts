#include <idc.idc>

/************************************************************************
   Desc:		Label each cvar variable with its appropriate name
   Author:  kynox
   Credit:	bobbysing for RenameFunc
   Website: http://www.gamedeception.net
*************************************************************************/

// 1 = Success, 0 = Failure
static RenameFunc(dwAddress, sFunction)
{
    auto dwRet;
    auto part = substr(GetFunctionName(dwAddress), 0, 7);

    if (part != "Script_")
    {
        auto oldName = GetFunctionName(dwAddress);

        dwRet = MakeNameEx(dwAddress, sFunction, SN_NOWARN);

        if (dwRet == 0)
        {
            auto sTemp, i;

            for (i = 1; i < 32; i++)
            {
                sTemp = form("%s_%i", sFunction, i);

                if ((dwRet = MakeNameEx(dwAddress, sTemp, SN_NOWARN)) != 0)
                {
                    // Message( "Info: Renamed to %s instead of %s\n", sTemp, sFunction );
                    break;
                }
            }

            if (i == 31)
                Message("-- Error --: Failed to rename %s -> %s\n", oldName, sFunction);
        }
        else
            Message("OK------>%s 0x%X\n", sFunction, dwAddress);
    }

    return dwRet;
}

static ExtractCvarDest(xRef)
{
    auto head, maxLoops;
    maxLoops = 20;
    head = PrevHead(xRef, 8);
    while (maxLoops--)
    {
        auto operandValue;
        if (head == BADADDR)
            break;
        operandValue = GetOperandValue(head, 0);
        if (GetMnem(head) == "mov" && SegName(operandValue) == ".data" && GetOpnd(head, 1) == "rax")
        {
            if (strstr(GetOpnd(head, 0), "[") > -1)
            {
                Message("LabelCvars: [%X] Array registrations unsupported\n", head);
                break;
            }

            return operandValue;
        }

        head = NextHead(head, head + 8);
    }

    return 0;
}

static ExtractCvarName(xRef)
{
    auto head;
    head = PrevHead(xRef - 0xc, 8);
    while (1)
    {
        auto operandValue;
        operandValue = GetOperandValue(head, 1);
        if (GetMnem(head) == "lea")
        {
            if (SegName(operandValue) != ".rdata")
                break;
            return GetString(operandValue, -1, ASCSTR_C);
        }

        head = PrevHead(head, 8);
    }
}

static main()
{
    auto cvarRegister, xRef;
    //2.5.2.41446  4C 8B DC 49 89 5B 08 49 89 6B 10 49 89 73 18 57 41 56 41 57 48 81 EC F0 00 00 00
    //3.4.0.46368T 4C 8B DC 49 89 5B 08 49 89 6B 10 49 89 73 18 57 41 54 41 55 41 56 41 57 48 81 EC D0 00 00 00
    //3.4.1.46722T 4C 8B DC 49 89 5B 08 49 89 6B 10 49 89 73 18 57 41 54 41 55 41 56 41 57 48 81 EC ?? ?? ?? ?? 48 8B BC 24 ?? ?? ?? ?? 4C 8D 25
    cvarRegister = FindBinary(0, SEARCH_DOWN, "4C 8B DC 49 89 5B 08 49 89 6B 10 49 89 73 18 57 41 54 41 55 41 56 41 57 48 81 EC ?? ?? ?? ?? 48 8B BC 24 ?? ?? ?? ?? 4C 8D 25");
    Message("OK------>cvarRegister_RegisterFunction at 0x%X\n", cvarRegister);

    for (xRef = RfirstB(cvarRegister); xRef != BADADDR; xRef = RnextB(cvarRegister, xRef))
    {
        auto cvarDest, cvarName;
        cvarName = ExtractCvarName(xRef);
        cvarDest = ExtractCvarDest(xRef);
        Message("OK------>cvarName:%s\n", cvarName);

        if (cvarDest)
        {
            RenameFunc(cvarDest, form("s_Cvar_%s", cvarName));
        }
    }

	 Message("Done \n");
}
