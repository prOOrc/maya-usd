//
// Copyright 2019 Autodesk
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
#pragma once

#include "../base/api.h"

#include "ufe/transform3dUndoableCommands.h"

#include "pxr/usd/usd/attribute.h"

PXR_NAMESPACE_USING_DIRECTIVE

MAYAUSD_NS_DEF {
namespace ufe {

//! \brief Absolute translation command of the given prim's rotate pivot.
/*!
	Ability to perform undo to restore the original pivot value.
 */
class MAYAUSD_CORE_PUBLIC UsdScalePivotTranslateUndoableCommand : public Ufe::TranslateUndoableCommand
{
public:
	typedef std::shared_ptr<UsdScalePivotTranslateUndoableCommand> Ptr;

	UsdScalePivotTranslateUndoableCommand(const UsdPrim& prim, const Ufe::Path& ufePath, const Ufe::SceneItem::Ptr& item, UsdTimeCode timeCode);
	~UsdScalePivotTranslateUndoableCommand() override;

	// Delete the copy/move constructors assignment operators.
	UsdScalePivotTranslateUndoableCommand(const UsdScalePivotTranslateUndoableCommand&) = delete;
	UsdScalePivotTranslateUndoableCommand& operator=(const UsdScalePivotTranslateUndoableCommand&) = delete;
	UsdScalePivotTranslateUndoableCommand(UsdScalePivotTranslateUndoableCommand&&) = delete;
	UsdScalePivotTranslateUndoableCommand& operator=(UsdScalePivotTranslateUndoableCommand&&) = delete;

	//! Create a UsdRotatePivotTranslateUndoableCommand from a USD prim, UFE path and UFE scene item.
	static UsdScalePivotTranslateUndoableCommand::Ptr create(const UsdPrim& prim, const Ufe::Path& ufePath, const Ufe::SceneItem::Ptr& item, UsdTimeCode timeCode);

	// Ufe::TranslateUndoableCommand overrides
	void undo() override;
	void redo() override;
	bool translate(double x, double y, double z) override;

private:
	UsdPrim fPrim;
	GfVec3f fPrevPivotValue;
	Ufe::Path fPath;
	UsdTimeCode fTimeCode;

}; // UsdRotatePivotTranslateUndoableCommand

} // namespace ufe
} // namespace MayaUsd