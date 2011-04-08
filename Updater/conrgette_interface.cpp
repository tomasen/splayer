#include "base/basictypes.h"

#include "courgette/third_party/bsdiff.h"
#include "courgette/courgette.h"
#include "courgette/streams.h"

#include "targetver.h"
#include <afxwin.h>         // MFC core and standard components
#include "cupdatenetlib.h"
#include "conrgette_interface.h"

bool ApplyEnsemblePatch(const CString old_file,
                        const CString patch_file,
                        const CString new_file) 
{
    BYTE* old_buffer = NULL ;
    BYTE* patch_buffer = NULL;
    DWORD dwOld;
    DWORD dwPatch;

    if (!ReadFileToBuffer(old_file, old_buffer, &dwOld))
        return false;
    if (!ReadFileToBuffer(patch_file, patch_buffer, &dwPatch))
        return false;
    courgette::SourceStream old_stream;
    courgette::SourceStream patch_stream;
    old_stream.Init(old_buffer, dwOld);
    patch_stream.Init(patch_buffer, dwPatch);
    courgette::SinkStream new_stream;
    courgette::Status status =
        courgette::ApplyEnsemblePatch(&old_stream, &patch_stream, &new_stream);

    if (status != courgette::C_OK) 
        return false;

    return WriteBufferToFile(new_file, reinterpret_cast<const BYTE*>(new_stream.Buffer()),
        new_stream.Length());
} 
