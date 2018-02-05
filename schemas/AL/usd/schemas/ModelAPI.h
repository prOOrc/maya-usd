//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef AL_USDMAYASCHEMAS_GENERATED_MODELAPI_H
#define AL_USDMAYASCHEMAS_GENERATED_MODELAPI_H

/// \file AL_USDMayaSchemas/ModelAPI.h

#include "pxr/pxr.h"
#include "./api.h"
#include "pxr/usd/usd/modelAPI.h"
#include "pxr/usd/usd/prim.h"
#include "pxr/usd/usd/stage.h"

#include "./tokens.h"


#include "pxr/base/vt/value.h"

#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/matrix4d.h"

#include "pxr/base/tf/token.h"
#include "pxr/base/tf/type.h"

PXR_NAMESPACE_OPEN_SCOPE

class SdfAssetPath;

// -------------------------------------------------------------------------- //
// ALMODELAPI                                                                 //
// -------------------------------------------------------------------------- //

/// \class AL_usd_ModelAPI
///
/// Data used to import a maya reference.
///
class AL_usd_ModelAPI : public UsdModelAPI
{
public:
    /// Compile-time constant indicating whether or not this class corresponds
    /// to a concrete instantiable prim type in scene description.  If this is
    /// true, GetStaticPrimDefinition() will return a valid prim definition with
    /// a non-empty typeName.
    static const bool IsConcrete = false;

    /// Construct a AL_usd_ModelAPI on UsdPrim \p prim .
    /// Equivalent to AL_usd_ModelAPI::Get(prim.GetStage(), prim.GetPath())
    /// for a \em valid \p prim, but will not immediately throw an error for
    /// an invalid \p prim
    explicit AL_usd_ModelAPI(const UsdPrim& prim=UsdPrim())
        : UsdModelAPI(prim)
    {
    }

    /// Construct a AL_usd_ModelAPI on the prim held by \p schemaObj .
    /// Should be preferred over AL_usd_ModelAPI(schemaObj.GetPrim()),
    /// as it preserves SchemaBase state.
    explicit AL_usd_ModelAPI(const UsdSchemaBase& schemaObj)
        : UsdModelAPI(schemaObj)
    {
    }

    /// Destructor.
    AL_USDMAYASCHEMAS_API
    virtual ~AL_usd_ModelAPI();

    /// Return a vector of names of all pre-declared attributes for this schema
    /// class and all its ancestor classes.  Does not include attributes that
    /// may be authored by custom/extended methods of the schemas involved.
    AL_USDMAYASCHEMAS_API
    static const TfTokenVector &
    GetSchemaAttributeNames(bool includeInherited=true);

    /// Return a AL_usd_ModelAPI holding the prim adhering to this
    /// schema at \p path on \p stage.  If no prim exists at \p path on
    /// \p stage, or if the prim at that path does not adhere to this schema,
    /// return an invalid schema object.  This is shorthand for the following:
    ///
    /// \code
    /// AL_usd_ModelAPI(stage->GetPrimAtPath(path));
    /// \endcode
    ///
    AL_USDMAYASCHEMAS_API
    static AL_usd_ModelAPI
    Get(const UsdStagePtr &stage, const SdfPath &path);


private:
    // needs to invoke _GetStaticTfType.
    friend class UsdSchemaRegistry;
    AL_USDMAYASCHEMAS_API
    static const TfType &_GetStaticTfType();

    static bool _IsTypedSchema();

    // override SchemaBase virtuals.
    AL_USDMAYASCHEMAS_API
    virtual const TfType &_GetTfType() const;

public:
    // ===================================================================== //
    // Feel free to add custom code below this line, it will be preserved by 
    // the code generator. 
    //
    // Just remember to: 
    //  - Close the class declaration with }; 
    //  - Close the namespace with PXR_NAMESPACE_CLOSE_SCOPE
    //  - Close the include guard with #endif
    // ===================================================================== //
    // --(BEGIN CUSTOM CODE)--

    /// Set the Selectability of the prim
    void SetSelectability(const TfToken& selectability);

    /// Get the current Selectability value on the current prim. If you want to
    /// determine the current selectability
    TfToken GetSelectability() const;

    /// Compute this Prims selectability value by looking at it own and it's
    /// through it's ancestor Prims to determine the hierarhical value.
    ///
    /// If one of the ancestors is found to be 'unselectable' then the 'unselectable'
    /// value is returned and the search stops.
    ///
    /// If no selectability value is found in the hierarchy, then the 'inherited' value
    /// is returned and should be considered 'selectable'.
    TfToken ComputeSelectabilty() const;

    /// Set the al_usdmaya_lock metadata of the prim.
    void SetLock(const TfToken& lock);

    /// Get the current value of prim's al_usdmaya_lock metadata. If no value
    /// defined on the prim, "inherited" is returned by default.
    TfToken GetLock() const;

    /// Compute current prim's lock value by inspecting its own metadata and
    /// walking up the prim hierarchy recursively.
    ///
    /// If a prim is found to be "inherited", this API keeps searching its
    /// parent prim's metadata till it's either "transform" or "unlocked" and
    /// returns with that value. If the whole hierarchy defining "inherited",
    /// "inherited" is returned and should be considered as "unlocked".
    TfToken ComputeLock() const;
private:
    typedef std::function<bool(const UsdPrim&, TfToken&)> ComputeLogic;
    TfToken ComputeHierarchical(const UsdPrim& prim, const ComputeLogic& logic) const;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif
