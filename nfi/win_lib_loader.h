#pragma once
#define VerifyMsg(x,y) if(x){};

inline std::string formated_error_msg(DWORD err_code = GetLastError())
{
    LPSTR error_txt = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&error_txt,
        0,
        NULL);

    if (error_txt != 0)
    {
        std::string res = error_txt;
        LocalFree(error_txt);

        return res;
    }

    return "";
}

typedef HMODULE lib_descriptor_t;
typedef FARPROC sym_descriptor_t;

inline lib_descriptor_t load_library(std::string const& lib_name)
{
    lib_descriptor_t lib = LoadLibraryA(lib_name.c_str());

    VerifyMsg(lib != 0, "nfi error, unable to load " << lib_name << " library, reason:" << formated_error_msg());
    return lib;
}

inline void unload_library(lib_descriptor_t lib)
{
    VerifyMsg(FreeLibrary(lib) != 0, "nfi error, unable to unload library, reason: " << formated_error_msg());
}

inline sym_descriptor_t get_symbol(lib_descriptor_t lib, std::string const& symb_name)
{
    GetLastError(); //just clear error flag

    sym_descriptor_t symb = GetProcAddress(lib, symb_name.c_str());
    DWORD err_code = GetLastError();

    VerifyMsg(
        symb != 0 || err_code == ERROR_PROC_NOT_FOUND,
        "nfi error, unable to load symbol " << symb_name << ", reason: " << formated_error_msg(err_code));

    return symb;
}

inline const char* lib_extension()
{
    return "dll";
}

inline std::list<std::string> sym_enum(lib_descriptor_t lib)
{
    // HMODULE lib = LoadLibraryEx(lib_name.c_str(), NULL, /*DONT_RESOLVE_DLL_REFERENCES*/0);
    assert(((PIMAGE_DOS_HEADER)lib)->e_magic == IMAGE_DOS_SIGNATURE);
    PIMAGE_NT_HEADERS header = (PIMAGE_NT_HEADERS)((BYTE *)lib + ((PIMAGE_DOS_HEADER)lib)->e_lfanew);
    assert(header->Signature == IMAGE_NT_SIGNATURE);
    assert(header->OptionalHeader.NumberOfRvaAndSizes > 0);
    PIMAGE_EXPORT_DIRECTORY exports = (PIMAGE_EXPORT_DIRECTORY)((BYTE *)lib + header->
        OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    PVOID names = (BYTE *)lib + exports->AddressOfNames;
    std::list<std::string> sym_list;
    for (unsigned i = 0; i < exports->NumberOfNames; i++)
    {
        // printf("Export: %s\n", (BYTE *)lib + ((DWORD *)names)[i]);
        sym_list.push_back(reinterpret_cast<char*>((BYTE *)lib + ((DWORD *)names)[i]));
    }

    return sym_list;
}


//inline string extract_libname(boost::filesystem::path const& file)
//{
//    return file.stem().string();
//}
//
//inline fs::path libs_path()
//{
//    char buf [MAX_PATH];
//    VerifyMsg(GetModuleFileNameA(0, buf, MAX_PATH) != 0, "Can not retrieve current module path");
//
//    return fs::path(string(buf)).parent_path();
//}
