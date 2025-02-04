#include <ida.idc>

static main()
{
    auto curAddr, xref;

    // DB2Load
	//40 53 48 83 EC 50 48 89 51 08 48 8D 05 ? ? ? ? 48 89 01
	//3.4.1.46722T 4C 8B DC 53 57 48 81 EC ?? ?? ?? ?? 48 89 51 08 48 8D 05 ?? ?? ?? ?? 48 89 01 48 8B D9 33 C0 48
	//search ChrClasses, click x find the linking, the next call is DB2load
    curAddr = FindBinary(0, SEARCH_DOWN, "4C 8B DC 53 57 48 81 EC ?? ?? ?? ?? 48 89 51 08 48 8D 05 ?? ?? ?? ?? 48 89 01 48 8B D9 33 C0 48");

    if (curAddr == BADADDR)
    {
        Message("Can't find DB2Load, aborting...\n");
        return;
    }

    Message("OK------>curAddr at 0x%X\n", curAddr);
    
     // time to loop through and find all cross references to the wow DB_Common_Load function we found above!
    for (xref = RfirstB(curAddr); xref != BADADDR; xref = RnextB(curAddr, xref))
    {
        auto prevFunc, nextFunc, disasm, disasmAddr, dbAddress, dbNameAddress;

        prevFunc = PrevFunction(xref);
        nextFunc = NextFunction(xref);
        disasmAddr = xref;

        disasmAddr = PrevHead(disasmAddr, prevFunc);
        //Message("OK------>GetOperandValue at 0x%X\n", disasmAddr);
        disasm = GetDisasm(disasmAddr);
        //Message("OK------>disasm at %s\n", disasm);
        if (strstr(disasm, "lea") > -1 && strstr(disasm, "rcx") > -1)
        {
            dbAddress = GetOperandValue(disasmAddr, 1);
            if (dbAddress == BADADDR)
            {
                continue;
            }
        }
        else
        {
            continue;
        }

        disasmAddr = PrevHead(disasmAddr, prevFunc);
        //Message("OK------>disasmAddr at 0x%X\n", disasmAddr);
        //disasm = GetDisasm(disasmAddr);
        //Message("OK------>disasm at %s\n", disasm);
        if (strstr(disasm, "lea") > -1 && strstr(disasm, "rcx") > -1)
        {
            dbNameAddress = GetOperandValue(disasmAddr, 1);
            //Message("OK------>dbNameAddress at 0x%X\n", dbNameAddress);
            if (dbNameAddress == BADADDR)
            {
                continue;
            }
        }
        else
        {
            continue;
        }
        auto dbName;
        //Message("OK------>dbNameAddress at 0x%X\n", dbNameAddress);
        dbName = WoWDb_GetName(dbNameAddress);
        //Message("OK------>%s = 0x%x\n", dbName, dbAddress);
        if (strlen(dbName) == 0)
        {
            break;
        }

        RenameFunc(dbAddress, form("%sDB", dbName));
        Message("OK------>%s = 0x%x\n", dbName, dbAddress);
    }
}


// 1 = Success, 0 = Failure
static RenameFunc(dwAddress, sFunction)
{
    auto dwRet;

    dwRet = MakeNameEx(dwAddress, sFunction, SN_NOWARN);

    if (dwRet == 0)
    {
        auto sTemp, i;
        for (i = 0; i < 32; i++)
        {
            sTemp = form("%s_%i", sFunction, i);

            if ((dwRet = MakeNameEx(dwAddress, sTemp, SN_NOWARN)) != 0)
            {
                Message("Info: Renamed to %s instead of %s\n", sTemp, sFunction);
                break;
            }
        }
    }
    return dwRet;
}

static WoWDb_GetName(dbBase)
{
    auto dbName;

    dbName = GetString(Dword(dbBase), -1, ASCSTR_C);

    return substr(dbName, strstr(dbName, "\\") + 1, 30);
}
