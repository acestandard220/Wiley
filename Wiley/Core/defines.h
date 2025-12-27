#pragma once


#define WILEY_SAFE_DELETE(ptr) delete ptr
#define WILEY_SAFE_DELETE_ARR(ptr) delete [] ptr
#define WILEY_SIZEOF(type) sizeof(type)

#ifdef _DEBUG
    #define WILEY_NAME_D3D12_OBJECT(object, name) wchar_t lName_##object[512];            \
                                   swprintf_s(lName_##object, 512, L"%hs", name.c_str()); \
                                   object->SetName(lName_##object);
#else
    #define WILEY_NAME_D3D12_OBJECT(object, name)
#endif //_DEBUG

#define WILEY_MUSTBE_UINTSIZE(size) assert(size <= UINT_MAX && "Buffer exceeds 4GB");
#define WILEY_MUSTBE_FLOATSIZE(size) assert(size <= UINT_MAX && "Buffer exceeds float size");

#define WILEY_MAYBE_UNUSED [[maybe_unused]]