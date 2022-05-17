#include "types.h"
#include <stdlib.h>
#include "debug.h"
#include "utils.h"
#include <avr/pgmspace.h>

void ParseSection_Type(wasm_module_ptr module, bytes start, bytes end);
void ParseSection_Import(wasm_module_ptr module, bytes start, bytes end);
void ParseSection_Function(wasm_module_ptr module, bytes start, bytes end);
void ParseSection_Memory(wasm_module_ptr module, bytes start, bytes end);
void ParseSection_Global(wasm_module_ptr module, bytes start, bytes end);
// void ParseSection_Start(wasm_module_ptr module, bytes start, bytes end);
void ParseSection_Code(wasm_module_ptr module, bytes start, bytes end);
void ParseSection_Data(wasm_module_ptr module, bytes start, bytes end);


wasm_module_ptr wasm_load_module(wasm_code_ptr code)
{
    int result = 0;
    wasm_module_ptr module;
    // 为wasm模块分配空间
    module = sys_malloc(sizeof(wasm_module));

    log(parse, "load module: %d bytes at %p", code->length,code->ptr);

    bytes pos = code->ptr;
    bytes end = pos + code->length;

    u32 magic, version;
    // 读取wasm版本和magic number，确认是否是wasm字节码文件
    Read_u32(&magic, &pos, end);
    Read_u32(&version, &pos, end);
    #if compile_time_check
    // [check] wasm版本检查
    if (magic != 0x6d736100 || version != 1)
    {
        panicf("Wasm Malformed");
    }
    #endif

    u8 previousSection = 0;

    while (pos < end)
    {
        u8 section;
        ReadLEB_u7(&section, &pos, end);
        #if compile_time_check
        // [check] 检查wasm段是否按顺序进行读取
        if (section > previousSection || section == 0)
        #endif
        {
            u32 sectionLength;
            // 首先读取section的长度
            ReadLEB_u32(&sectionLength, &pos, end);
            // 根据长度处理该section
            ParseModuleSection(module, section, pos, sectionLength);

            pos += sectionLength;

            if (section)
            {
                // 记录section序号，因为wasm解析有顺序约束，不允许乱序解析
                previousSection = section;
            }
        }
        #if compile_time_check
        // else
        // {
        //     panicf("Error! Misordered Wasm Section");
        // }
        #endif
    }

    if (result)
    {
        log(parse, "err, sys_free module\r\n");
        sys_free(module);
        module = NULL;
        return NULL;
    }

    log(parse, "Parsing done.");
    return module;
}

void ParseModuleSection(wasm_module_ptr module, u8 i_sectionType, bytes start, u32 i_numBytes)
{
    typedef void (*Parser)(wasm_code_ptr, bytes, bytes);
    // 仅解析目前支持的seciton，其余section跳过处理
    static Parser s_parsers[] =
        {
            NULL,                  // 0 ParseSection_Custom
            ParseSection_Type,     // 1
            ParseSection_Import,   // 2
            ParseSection_Function, // 3
            NULL,                  // 4: TODO Table
            ParseSection_Memory,   // 5
            ParseSection_Global,   // 6
            NULL,                  //ParseSection_Export,   // 7
            // ParseSection_Start,    // 8
            NULL,    // 8
            NULL,                  //ParseSection_Element,  // 9
            ParseSection_Code,     // 10
            ParseSection_Data,     // 11
        };

    Parser parser = NULL;

    if (i_sectionType <= 12)
    {
        parser = s_parsers[i_sectionType];
        log(parse, " sectionType = %d", (u32)i_sectionType);
    }

    if (parser)
    {
        bytes end = start + i_numBytes;
        parser(module, start, end);
    }
    else
    {
        log(parse, " skipped section type: %d", (u32)i_sectionType);
    }
}
void AllocFuncType(func_type_ptr *o_functionType, u32 i_numArgs)
{
    size_t funcTypeSize = sizeof(func_type) - 2 /* sizeof (argTypes [2])，因为结构体本身预留了两个参数的空间*/ + i_numArgs;

    *o_functionType = sys_malloc(funcTypeSize);
}

void PrintFuncTypeSignature(func_type_ptr functype)
{
    printf("(");
    for (int i = 0; i < functype->args_num; i++)
    {
        if (i == 0)
        {
            printf("%s", wasm_types_names[functype->args_type_list[i]]);
        }
        else
        {
            printf(", %s", wasm_types_names[functype->args_type_list[i]]);
        }
    }
    printf(") -> %s", wasm_types_names[functype->returnType]);
}
// 解析类型段，段中内容主要是函数的类型定义，包括参数数量和类型以及返回值类型
void ParseSection_Type(wasm_module_ptr module, bytes start, bytes end)
{
    u32 numTypes;
    ReadLEB_u32(&numTypes, &start, end);
    log(parse, "** Type [%d]", numTypes);

    if (numTypes)
    {
        // 创建函数定义列表并分配空间
        module->func_type_list = sys_malloc(numTypes * sizeof(func_type_ptr));
        module->func_type_num = numTypes;

        for (u32 i = 0; i < numTypes; ++i)
        {
            i8 form;
            ReadLEB_i7(&form, &start, end);
            #if compile_time_check
            // [check] 类型段定义检查
            if (form != -32) // -32即0x60 functype类型，该段中只应该包含该类型的定义
            {
                panicf("Wasm Malformed");
            }
            #endif

            u32 numArgs;
            // 读取该函数的参数数量
            ReadLEB_u32(&numArgs, &start, end);
            // 根据参数数量重新分配函数定义结构体的大小（因为函数定义结构体大小需要随参数数量变化，因此需要重新分配空间）
            AllocFuncType(&module->func_type_list[i], numArgs);

            func_type_ptr ftype = module->func_type_list[i];
            ftype->args_num = numArgs;

            for (u32 a = 0; a < numArgs; ++a)
            {
                i8 wasmType;
                u8 argType;
                // 读取参数类型
                ReadLEB_i7(&wasmType, &start, end);
                // 将类型翻译为我们定义的枚举类型，增加可读性（因为储存的类型定义换算比较复杂，因此专门写了一个函数进行翻译）
                NormalizeType(&argType, wasmType);

                ftype->args_type_list[a] = argType;
            }

            u8 returnCount;
            ReadLEB_u7(&returnCount, &start, end); //目前规范中是返回值只能有1个

            // 如果有返回值，则读取并记录返回值类型
            if (returnCount)
            {
                i8 returnType;
                ReadLEB_i7(&returnType, &start, end);
                // 同样这里需要翻译类型
                NormalizeType(&ftype->returnType, returnType);
            }
            log(parse, "    type %2d: ", i);
            logif(parse, PrintFuncTypeSignature(ftype));

            // Environment_AddFuncType(module->environment, &ftype);//TODO 如果需要环境，则补充这句
            module->func_type_list[i] = ftype;
        }
    }
}
void FreeImportInfo(import_info_ptr i_info)
{
    if (i_info->fieldUtf8 && i_info->moduleUtf8)
    {
        sys_free(i_info->moduleUtf8);
        sys_free(i_info->fieldUtf8);
    }
}
// 解析import段
void ParseSection_Import(wasm_module_ptr module, bytes start, bytes end)
{
    import_info import = {NULL, NULL}, clearImport = {NULL, NULL};

    u32 numImports;
    // 读取import数量
    ReadLEB_u32(&numImports, &start, end);
    log(parse, "** Import [%d]", numImports);

    for (u16 i = 0; i < numImports; ++i)
    {
        u8 importKind;
        // 读取import模块名
        Read_utf8(&import.moduleUtf8, &start, end);
        // 读取import域名
        Read_utf8(&import.fieldUtf8, &start, end);
        // 读取import种类：函数、table、内存、全局变量
        Read_u8(&importKind, &start, end);
        log(parse, "    kind: %d '%s.%s' ",
            importKind, import.moduleUtf8, import.fieldUtf8);
        switch (importKind)
        {
        // 引入外部函数
        case d_externalKind_function:
        {
            u32 typeIndex;
            // 读取函数索引
            ReadLEB_u32(&typeIndex, &start, end);
            Module_AddFunction(module, typeIndex, &import);
            import = clearImport;

            module->import_num++;
        }
        break;

        case d_externalKind_table:
            //                  result = ParseType_Table (& start, end);
            break;

        case d_externalKind_memory:
        {
            ParseType_Memory(&module->memory, &start, end);
            module->memory_imported = true;
        }
        break;

        case d_externalKind_global:
        {
            i8 waType;
            u8 type, isMutable;

            ReadLEB_i7(&waType, &start, end);
            NormalizeType(&type, waType);
            ReadLEB_u7(&isMutable, &start, end);
            // log(parse, "     global: %s mutable=%d", wasm_types_names[type], isMutable);
            logif(parse, printf("     global: %s ", wasm_types_names[type]); printf("mutable=%d", isMutable));

            wasm_global_ptr global;
            Module_AddGlobal(module, module->global_num, &global, type, isMutable, true /* isImport */);
            global->import = import;
            import = clearImport;
        }
        break;

        default:
            panicf("Wasm Malformed");
        }

        FreeImportInfo(&import);
    }
}
void Module_AddFunction(wasm_module_ptr module, u32 i_typeIndex, import_info_ptr i_importInfo)
{
    // 动态分配函数列表空间
    if (module->function_num == 0)
    {
        module->function_list = sys_malloc(sizeof(wasm_function_ptr));
        module->function_list[0] = sys_malloc(sizeof(wasm_function));
        module->function_num = 1;
    }
    else
    {
        module->function_num++;
        log(parse,"bytes to alloc: %d",module->function_num * sizeof(wasm_function_ptr));
        module->function_list = sys_realloc(module->function_list, module->function_num * sizeof(wasm_function_ptr));
        module->function_list[module->function_num - 1] = sys_malloc(sizeof(wasm_function));
    }
    #if compile_time_check
    // [check] 检查函数索引是否超出函数总数
    if (i_typeIndex < module->func_type_num)
    #endif
    {
        wasm_function_ptr func = module->function_list[module->function_num - 1];
        func->funcType = module->func_type_list[i_typeIndex];
        // 如果是import函数，则根据import信息获取函数名
        if (i_importInfo)
        {
            func->import = *i_importInfo;
            func->name = i_importInfo->fieldUtf8;
        }
        else
        {
            // 否则以函数索引进行命名，根据索引大小分配函数名空间，如10号函数需要3字节空间 ‘1’ ‘0’ ‘\0'
            if(module->function_num - module->import_num<10){
                func->name = sys_malloc(2);
            }else if(module->function_num - module->import_num<100){
                func->name = sys_malloc(3);
            }else{
                func->name = sys_malloc(4);
            }
            
            snprintf(func->name, 10, "%d", module->function_num - module->import_num);
        }
        // log(parse,"   added function %3d: %s; sig: %d;",module->function_num-1,func->name, i_typeIndex);
        logif(parse, {printf("   added function %3d:",module->function_num-1);printf(" %s;",func->name);printf(" sig: %d;", i_typeIndex); });
    }
    #if compile_time_check
    else
    {
        panicf("type sig index out of bounds");
    }
    #endif
}
// 解析函数段，需要注意的是这里仅能读取出函数索引所对应的函数类型（具体类型在类型段定义）
void ParseSection_Function(wasm_module_ptr module, bytes start, bytes end)
{
    u32 numFunctions;
    // 获取函数数量
    ReadLEB_u32(&numFunctions, &start, end);
    log(parse, "** Function [%d]", numFunctions);

    // 逐个加入函数
    for (u32 i = 0; i < numFunctions; ++i)
    {
        u32 funcTypeIndex;
        ReadLEB_u32(&funcTypeIndex, &start, end);
        Module_AddFunction(module, funcTypeIndex, NULL /* import info */);
    }
}
// 解析内存类型，即内存的初始设置：初始页数，最大页数（可选，由flag标记）
void ParseType_Memory(memory_info_ptr o_memory, bytes *io_bytes, bytes end)
{
    u8 flag;

    ReadLEB_u7(&flag, io_bytes, end); // really a u1
    ReadLEB_u32(&o_memory->init_page_num, io_bytes, end);

    o_memory->max_page_num = 0;
    if (flag)
        ReadLEB_u32(&o_memory->max_page_num, io_bytes, end);
}
// 解析内存段，一个模块可包含多个内存区，目前本运行时仅支持单个内存区
void ParseSection_Memory(wasm_module_ptr module, bytes start, bytes end)
{
    // TODO: MVP; assert no memory imported
    u32 numMemories;
    // 读取内存区数量
    ReadLEB_u32(&numMemories, &start, end);
    log(parse, "** Memory [%d]", numMemories);
    #if compile_time_check
    // [check] 内存区数量检查：当前仅支持单个内存区
    if (numMemories != 1)
    {
        panicf("Too Many Memory Sections");
    }
    #endif
    // 读取内存区类型
    ParseType_Memory(&module->memory, &start, end);
}
// 添加全局变量
void Module_AddGlobal(wasm_module_ptr module, u32 index, wasm_global_ptr *o_global, u8 i_type, bool i_mutable, bool i_isImported)
{
    // if (index == module->global_num)
    // {
    //     module->global_num++;
    //     sys_realloc(module->global_list, module->global_num * sizeof(wasm_global));
    // }
    wasm_global_ptr global = &module->global_list[index];

    global->type = i_type;
    global->imported = i_isImported;
    global->isMutable = i_mutable;

    if (o_global)
        *o_global = global;
}
//TODO 目前直接略过，在编译时替换为编译过程
void Parse_InitExpr(wasm_module_ptr module, bytes *start, bytes end)
{
    while (pgm_read_byte_far(*start) != 0x0B && *start < end)
    {
        (*start)++;
    }
    if (pgm_read_byte_far(*start) == 0x0B)
    {
        (*start)++;
    }
}
// 解析全局变量段
void ParseSection_Global(wasm_module_ptr module, bytes start, bytes end)
{
    u32 numGlobals;
    // 读取全局变量数量
    ReadLEB_u32(&numGlobals, &start, end);
    log(parse, "** Global [%d]", numGlobals);
    module->global_num = numGlobals;
    // 根据数量分配全局变量结构体数组
    module->global_list = sys_malloc(numGlobals * sizeof(wasm_global));
    for (u32 i = 0; i < numGlobals; ++i)
    {
        i8 waType;
        u8 type, isMutable;
        // 读取全局变量类型
        ReadLEB_i7(&waType, &start, end);
        // 类型翻译
        NormalizeType(&type, waType);
        // 读取可写标志，若为1则该变量可写，否则该变量只读，需要阻止对该变量的写行为
        ReadLEB_u7(&isMutable, &start, end);
        // log(parse,"    global: [%d] %s mutable=%d",i,wasm_types_names[type],isMutable);
        logif(parse, printf("    global: [%d] ",i);printf("%s ", wasm_types_names[type]); printf("mutable=%d", isMutable));
        wasm_global_ptr global;
        // 设置该全局变量结构体
        Module_AddGlobal(module, i, &global, type, isMutable, false /* isImport */);
        global->initExpr = start;
        // 读取该变量初始值，wasm支持以可执行代码赋初始值，因此解析阶段该函数实际操作是略过该段，由编译阶段处理
        Parse_InitExpr(module, &start, end);
        global->initExprSize = (u32)(start - global->initExpr);
        #if compile_time_check
        // [check] 全局变量初始值检查
        if (global->initExprSize <= 1)
        {
            panicf("Wasm Missing Init Expression");
        }
        #endif
    }
}
// void ParseSection_Start(wasm_module_ptr module, bytes start, bytes end)
// {
//     u32 startFuncIndex;
//     ReadLEB_u32(&startFuncIndex, &start, end);
//     log(parse, "** Start Function: %d", startFuncIndex);

//     if (startFuncIndex < module->function_num)
//     {
//         module->startFunction = startFuncIndex;
//     }
//     else
//         panicf("start function index out of bounds");
// }
// 解析代码段，其中包含了各个函数的代码
void ParseSection_Code(wasm_module_ptr module, bytes start, bytes end)
{
    u32 numFunctions;
    // 读取函数数量（总函数数量-import函数数量）
    ReadLEB_u32(&numFunctions, &start, end);
    log(parse, "** Code [%d]", numFunctions);

    for (u32 f = 0; f < numFunctions; ++f)
    {
        u32 size;
        // 读取该函数代码长度
        ReadLEB_u32(&size, &start, end);
        if (size)
        {
            bytes ptr = start;
            start += size;

            if (start <= end)
            {
                bytes begin = ptr;

                u32 numLocalBlocks;
                // 读取代码块数量
                ReadLEB_u32(&numLocalBlocks, &ptr, end);
                log(parse, "    code size: %-4d", size);

                u32 numLocals = 0;
                u32 numLocalBytes = 0;

                for (u32 l = 0; l < numLocalBlocks; ++l)
                {
                    u32 varCount;
                    i8 wasmType;
                    u8 normalType;
                    // 读取局部变量数量
                    ReadLEB_u32(&varCount, &ptr, end);
                    // 读取代码块返回值类型
                    ReadLEB_i7(&wasmType, &ptr, end);
                    NormalizeType(&normalType, wasmType);
                    // 开始计算局部变量所需空间
                    numLocals += varCount;
                    // 目前仅支持同一类型的局部变量，要么全32位，要么全64位（后续执行阶段只考虑了32位）
                    switch (normalType)
                    {
                    case WASM_Type_i32:
                    case WASM_Type_f32:
                        numLocalBytes += 4 * varCount;
                        break;
                    case WASM_Type_i64:
                    case WASM_Type_f64:
                        numLocalBytes += 8 * varCount;
                        break;
                    default:
                        panicf("unsupported local type");
                        break;
                    }
                    log(parse, "      %2ld locals; type: '%s'", varCount, wasm_types_names[normalType]);
                }
                // 记录代码区所在位置和局部变量数量及所需空间
                wasm_function_ptr func = module->function_list[f + module->import_num];
                // wasm_function_ptr func = &module->function_list[f + module->import_num];
                func->module = module;
                func->wasm = ptr;
                func->wasmEnd = start;
                func->numLocals = numLocals;
                func->numLocalBytes = numLocalBytes;
            }
            else
                panicf("Wasm Section Overrun");
        }
    }
}
// 解析数据段
void ParseSection_Data(wasm_module_ptr module, bytes start, bytes end)
{
    u32 numDataSegments;
    // 获取数据段数量
    ReadLEB_u32(&numDataSegments, &start, end);
    log(parse, "** Data [%d]", numDataSegments);
    // 分配数据段数组空间
    module->data_segment_list = sys_malloc(numDataSegments * sizeof(data_segment));

    module->data_segment_num = numDataSegments;

    for (u32 i = 0; i < numDataSegments; ++i)
    {
        data_segment_ptr segment = &module->data_segment_list[i];
        // 读取所在内存区
        ReadLEB_u32(&segment->memoryRegion, &start, end);

        // 读取数据段偏移
        segment->initExpr = start;
        Parse_InitExpr(module, &start, end);
        segment->initExprSize = (u16)(start - segment->initExpr);
        #if compile_time_check
        // [check] 数据段偏移表达式长度检查
        if (segment->initExprSize <= 1)
        {
            panicf("Wasm Missing Init Expression");
        }
        #endif
        // 读取数据段初始值长度
        ReadLEB_u32(&segment->size, &start, end);
        segment->data = start;
        // log(parse, "    segment [%u]  memory: %u;  expr-size: %d;  size: %d",
        //     i, segment->memoryRegion, segment->initExprSize, segment->size);
        logif(parse, printf("    segment [%u]  ", i); printf("memory: %u;  ",
                                                             segment->memoryRegion);
              printf("expr-size: %d;  ", segment->initExprSize);
              printf("size: %d", segment->size););
        start += segment->size;
    }
}
