#ifndef __HDMAYA_DELEGATE_BASE_H__
#define __HDMAYA_DELEGATE_BASE_H__

#include <pxr/pxr.h>

#include <pxr/imaging/hd/renderIndex.h>
#include <pxr/imaging/hd/sceneDelegate.h>
#include <pxr/imaging/hd/rprimCollection.h>

#include <pxr/usd/sdf/path.h>


PXR_NAMESPACE_OPEN_SCOPE

class HdMayaDelegateCtx : protected HdSceneDelegate {
protected:
    HdMayaDelegateCtx(
        HdRenderIndex* renderIndex,
        const SdfPath& delegateID);
public:
    using HdSceneDelegate::GetRenderIndex;
    HdChangeTracker& GetChangeTracker() { return GetRenderIndex().GetChangeTracker(); }

    void InsertRprim(const TfToken& typeId, const SdfPath& id, HdDirtyBits initialBits);
    void InsertSprim(const TfToken& typeId, const SdfPath& id, HdDirtyBits initialBits);
    const HdRprimCollection& GetRprimCollection() { return _rprimCollection; }
private:
    HdRprimCollection _rprimCollection;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // __HDMAYA_DELEGATE_BASE_H__
