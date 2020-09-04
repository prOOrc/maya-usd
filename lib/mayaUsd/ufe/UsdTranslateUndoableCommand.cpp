//
// Copyright 2020 Autodesk
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#include "UsdTranslateUndoableCommand.h"

#include "Utils.h"
#include "private/Utils.h"

#include <pxr/usd/sdf/primSpec.h>
#include <pxr/usd/sdf/attributeSpec.h>

#include <mayaUsd/base/debugCodes.h>

#include <mayaUsdUtils/TransformManipulator.h>
#include <mayaUsdUtils/SIMD.h>
#include <iostream>

MAYAUSD_NS_DEF {
namespace ufe {

extern MayaUsdUtils::TransformManipulator::Space CurrentTranslateManipulatorSpace();
extern MayaUsdUtils::TransformManipulator::Space CurrentManipulatorSpace();

static bool ExistingOpHasSamples(const UsdGeomXformOp& op)
{
	return op.GetNumTimeSamples() != 0;
}

//------------------------------------------------------------------------------
UsdTranslateUndoableCommand::UsdTranslateUndoableCommand(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode
) : Ufe::TranslateUndoableCommand(item),
	fPrim(ufePathToPrim(item->path())),
	fPrevValue(0,0,0),
	fNewValue(x, y, z),
	fPath(item->path()),
	fTimeCode(timeCode)
{
    try 
    {
        MayaUsdUtils::TransformManipulator proc(prim(), TfToken(""), MayaUsdUtils::TransformManipulator::kTranslate, timeCode);
        fOp = proc.op();
		// only write time samples if op already has samples
		if(!ExistingOpHasSamples(fOp))
		{
			fTimeCode = UsdTimeCode::Default();
		}
        fPrevValue = proc.Translation();
        fCreatedOp = false;
        fCreatedOrderedAttr = false;
    }
    catch(const std::exception& e)
    {
		// use default time code if using a new op?
		fTimeCode = UsdTimeCode::Default();
        auto xformOpOrderAttr = prim().GetAttribute(TfToken("xformOpOrder"));
        fCreatedOrderedAttr = xformOpOrderAttr ? !xformOpOrderAttr.HasAuthoredValue() : true;
        
        //
        // So I'm going to make the assumption here that you *probably* want to manipulate the very 
        // first translate in the xform op stack?
        // 
        // uniform token[] xformOpOrder = ["xformOp:translate", "xformOp:translate:rotatePivotTranslate", 
        //                                 "xformOp:translate:rotatePivot", "xformOp:rotateXYZ", 
        //                                 "!invert!xformOp:translate:rotatePivot", "xformOp:translate:scalePivotTranslate", 
        //                                 "xformOp:translate:scalePivot", "xformOp:scale", "!invert!xformOp:translate:scalePivot"]
        // 
        // 
        UsdGeomXformable xform(prim());
        bool reset;
        std::vector<UsdGeomXformOp> ops = xform.GetOrderedXformOps(&reset);
        fOp = xform.AddTranslateOp(UsdGeomXformOp::PrecisionDouble);
        ops.insert(ops.begin(), fOp);
        xform.SetXformOpOrder(ops, reset);

        fCreatedOp = true;
        auto stage = prim().GetStage();
        fEditTarget = stage->GetEditTarget();
    }
}

//------------------------------------------------------------------------------
UsdTranslateUndoableCommand::~UsdTranslateUndoableCommand()
{}

//------------------------------------------------------------------------------
UsdTranslateUndoableCommand::Ptr UsdTranslateUndoableCommand::create(
    const UsdSceneItem::Ptr& item, double x, double y, double z, const UsdTimeCode& timeCode
)
{
    auto cmd = std::make_shared<MakeSharedEnabler<UsdTranslateUndoableCommand>>(
        item, x, y, z, timeCode);
    return cmd;
}

//------------------------------------------------------------------------------
void UsdTranslateUndoableCommand::undo()
{
    if(fCreatedOp)
    {
        SdfPrimSpecHandle specHandle = fEditTarget.GetPrimSpecForScenePath(prim().GetPath());
        if(specHandle)
        {
            // annoyingly, we have to get the xform ops first, otherwise removal of the 
            // attribute spec will cause a bother later on.
            bool reset;
            auto ops = UsdGeomXformable(prim()).GetOrderedXformOps(&reset);

            auto opName = fOp.GetName(); 
            {
                auto attrSpecView = specHandle->GetAttributes();
                for(auto spec : attrSpecView)
                {
                    if(opName == spec->GetName())
                    {
                        specHandle->RemoveProperty(spec);
                        break;
                    }
                }
            }

            // if when creating the original translate op we added a new xformOpOrder attribute
            // as a side effect, be sure we remove that here.
            if(fCreatedOrderedAttr)
            {
                auto attrSpecView = specHandle->GetAttributes();
                for(auto spec : attrSpecView)
                {
                    if("xformOpOrder" == spec->GetName())
                    {
                        specHandle->RemoveProperty(spec);
                        break;
                    }
                }
            }
            else
            // otherwise hunt for the xformOp in the list, and remove it.
            {
                for(auto it = ops.begin(); it != ops.end(); ++it)
                {
                    if(it->GetName() == opName)
                    {
                        ops.erase(it);
                        UsdGeomXformable(prim()).SetXformOpOrder(ops, reset);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        // do nothing
        if(GfIsClose(fNewValue, fPrevValue, 1e-5f))
        {
            return;
        }
        switch(fOp.GetOpType())
        {
        case UsdGeomXformOp::TypeTranslate:
            {
                switch(fOp.GetPrecision())
                {
                case UsdGeomXformOp::PrecisionHalf:
                    {
                        fOp.Set(GfVec3h(fPrevValue[0], fPrevValue[1], fPrevValue[2]), fTimeCode);
                    }
                    break;

                case UsdGeomXformOp::PrecisionFloat:
                    {
                        fOp.Set(GfVec3f(fPrevValue[0], fPrevValue[1], fPrevValue[2]), fTimeCode);
                    }
                    break;
                    
                case UsdGeomXformOp::PrecisionDouble:
                    {
                        fOp.Set(fPrevValue, fTimeCode);
                    }
                    break;
                }
            }
            break;

        case UsdGeomXformOp::TypeTransform:
            {
                GfMatrix4d M;
                fOp.Get(&M, fTimeCode);
                M[3][0] = fPrevValue[0];
                M[3][1] = fPrevValue[1];
                M[3][2] = fPrevValue[2];
                fOp.Set(M, fTimeCode);
            }
            break;

        default:
            break;
        }
    }
}

//------------------------------------------------------------------------------
void UsdTranslateUndoableCommand::redo()
{
}

// rotate an offset vector by the coordinate frame
inline MayaUsdUtils::d256 rotate(const MayaUsdUtils::d256 offset, const MayaUsdUtils::d256 frame[4])
{
  const MayaUsdUtils::d256 xxx = MayaUsdUtils::permute4d<0, 0, 0, 0>(offset);
  const MayaUsdUtils::d256 yyy = MayaUsdUtils::permute4d<1, 1, 1, 1>(offset);
  const MayaUsdUtils::d256 zzz = MayaUsdUtils::permute4d<2, 2, 2, 2>(offset);
  return MayaUsdUtils::fmadd4d(zzz, frame[2], 
         MayaUsdUtils::fmadd4d(yyy, frame[1], 
         MayaUsdUtils::mul4d(xxx, frame[0])));
}

// rotate an offset vector by the coordinate frame
inline MayaUsdUtils::d256 transform4d(const MayaUsdUtils::d256 offset, const MayaUsdUtils::d256 frame[4])
{
  const MayaUsdUtils::d256 xxx = MayaUsdUtils::permute4d<0, 0, 0, 0>(offset);
  const MayaUsdUtils::d256 yyy = MayaUsdUtils::permute4d<1, 1, 1, 1>(offset);
  const MayaUsdUtils::d256 zzz = MayaUsdUtils::permute4d<2, 2, 2, 2>(offset);
  const MayaUsdUtils::d256 www = MayaUsdUtils::permute4d<3, 3, 3, 3>(offset);
  return MayaUsdUtils::fmadd4d(www, frame[3], 
         MayaUsdUtils::fmadd4d(zzz, frame[2], 
         MayaUsdUtils::fmadd4d(yyy, frame[1], 
         MayaUsdUtils::mul4d(xxx, frame[0]))));
}

// frame *= childTransform
inline void multiply4x4(MayaUsdUtils::d256 frame[4], const MayaUsdUtils::d256 childTransform[4])
{
  const MayaUsdUtils::d256 mx = transform4d(childTransform[0], frame);
  const MayaUsdUtils::d256 my = transform4d(childTransform[1], frame);
  const MayaUsdUtils::d256 mz = transform4d(childTransform[2], frame);
  frame[3] = transform4d(childTransform[3], frame);
  frame[0] = mx;
  frame[1] = my;
  frame[2] = mz;
}
// frame *= childTransform
inline void multiply4x4(MayaUsdUtils::d256 output[4], const MayaUsdUtils::d256 childTransform[4], const MayaUsdUtils::d256 parentTransform[4])
{
  const MayaUsdUtils::d256 mx = transform4d(childTransform[0], parentTransform);
  const MayaUsdUtils::d256 my = transform4d(childTransform[1], parentTransform);
  const MayaUsdUtils::d256 mz = transform4d(childTransform[2], parentTransform);
  output[3] = transform4d(childTransform[3], parentTransform);
  output[0] = mx;
  output[1] = my;
  output[2] = mz;
}

//------------------------------------------------------------------------------
// Ufe::TranslateUndoableCommand overrides
//------------------------------------------------------------------------------

bool UsdTranslateUndoableCommand::translate(double x, double y, double z)
{
    fNewValue = GfVec3d(x, y, z);
    try
    {
        MayaUsdUtils::TransformManipulator proc(prim(), TfToken(""), MayaUsdUtils::TransformManipulator::kTranslate, fTimeCode);
        GfMatrix4d m = MayaUsdUtils::TransformManipulator::EvaluateCoordinateFrameForIndex(proc.ops(), proc.ops().size(), fTimeCode);
        switch(CurrentTranslateManipulatorSpace())
        {
        case MayaUsdUtils::TransformManipulator::kPreTransform:
            {
                //------------------------------------------------------------------------------------------------------
                auto diff = MayaUsdUtils::set4d(fNewValue[0], fNewValue[1], fNewValue[2], 1.0);
                diff = MayaUsdUtils::sub4d(diff, MayaUsdUtils::load4d(m[3]));
                // should nicely be handled here
                proc.Translate(GfVec3d(MayaUsdUtils::get<0>(diff), MayaUsdUtils::get<1>(diff), MayaUsdUtils::get<2>(diff)), MayaUsdUtils::TransformManipulator::kPreTransform);
                //------------------------------------------------------------------------------------------------------
            }   
            break;

        case MayaUsdUtils::TransformManipulator::kPostTransform:
            {
                //------------------------------------------------------------------------------------------------------
                // first evaluate the difference according to maya (effectively this is in parent space)
                MayaUsdUtils::d256 diff = MayaUsdUtils::sub4d(MayaUsdUtils::set4d(fNewValue[0], fNewValue[1], fNewValue[2], 0), MayaUsdUtils::loadu4d(m[3]));
                diff = MayaUsdUtils::select4d<1, 1, 1, 0>(diff, MayaUsdUtils::zero4d());

                // rotate into coordinate frame of xform
                diff = rotate(diff, (MayaUsdUtils::d256*)&m);
                
                // should nicely be handled here
                proc.Translate(GfVec3d(MayaUsdUtils::get<0>(diff), MayaUsdUtils::get<1>(diff), MayaUsdUtils::get<2>(diff)), MayaUsdUtils::TransformManipulator::kPreTransform);
                //------------------------------------------------------------------------------------------------------
            }   
            break;

        case MayaUsdUtils::TransformManipulator::kWorld:
            {
                //------------------------------------------------------------------------------------------------------
                auto diff = MayaUsdUtils::set4d(fNewValue[0], fNewValue[1], fNewValue[2], 1.0);
                auto procc = proc.Translation();
                diff = MayaUsdUtils::sub4d(diff, MayaUsdUtils::set4d(procc[0], procc[1], procc[2], 0));
                
                // should nicely be handled here
                proc.Translate(GfVec3d(MayaUsdUtils::get<0>(diff), MayaUsdUtils::get<1>(diff), MayaUsdUtils::get<2>(diff)), MayaUsdUtils::TransformManipulator::kPreTransform);
                //------------------------------------------------------------------------------------------------------
            }
            break;

        case MayaUsdUtils::TransformManipulator::kTransform:
            //------------------------------------------------------------------------------------------------------
            proc.Translate(fNewValue - proc.Translation());
            //------------------------------------------------------------------------------------------------------
            break;
        }
    }
    catch(const std::exception& e)
    {
        return false;
    }
    return true;
}

} // namespace ufe
} // namespace MayaUsd
