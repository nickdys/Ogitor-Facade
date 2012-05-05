// /*/////////////////////////////////////////////////////////////////////////////////
// /// An
// ///    ___   ____ ___ _____ ___  ____
// ///   / _ \ / ___|_ _|_   _/ _ \|  _ \
// ///  | | | | |  _ | |  | || | | | |_) |
// ///  | |_| | |_| || |  | || |_| |  _ <
// ///   \___/ \____|___| |_| \___/|_| \_\
// ///                              File
// ///
// /// Copyright (c) 2008-2011 Ismail TARIM <ismail@royalspor.com> and the Ogitor Team
// //
// /// The MIT License
// ///
// /// Permission is hereby granted, free of charge, to any person obtaining a copy
// /// of this software and associated documentation files (the "Software"), to deal
// /// in the Software without restriction, including without limitation the rights
// /// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// /// copies of the Software, and to permit persons to whom the Software is
// /// furnished to do so, subject to the following conditions:
// ///
// /// The above copyright notice and this permission notice shall be included in
// /// all copies or substantial portions of the Software.
// ///
// /// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// /// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// /// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// /// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// /// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// /// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// /// THE SOFTWARE.
// ////////////////////////////////////////////////////////////////////////////////*/
// 

#include "OgitorsPrerequisites.h"
#include "BaseEditor.h"
#include "OgitorsRoot.h"
#include "OgitorsSystem.h"
#include "NodeEditor.h"
#include "EntityEditor.h"
#include "BuildingEditor.h"
#include "SceneManagerEditor.h"
#include "OBBoxRenderable.h"
#include "tinyxml.h"

#include "OgreSceneManager.h"

namespace Ogitors {
    
const float CBuildingEditor::SEARCH_RADIUS = 10;

std::list<CBuildingEditor*> CBuildingEditorFactory::mBuildings;
    
OBB::OBB() {}
    
OBB::OBB(Ogre::Vector3 bbl, Ogre::Vector3 btl, Ogre::Vector3 btr, Ogre::Vector3 bbr, Ogre::Vector3 ftr, Ogre::Vector3 ftl, Ogre::Vector3 fbl, Ogre::Vector3 fbr) : 
bbl(bbl),  btl(btl),  btr(btr),  bbr(bbr),  ftr(ftr),  ftl(ftl),  fbl(fbl),  fbr(fbr) 
{}

CBuildingEditor::CBuildingEditor(CBaseEditorFactory *factory) : CEntityEditor(factory), 
mLeft(0),
mRight(0),
scale_const(0)
{
    mEntityHandle = 0;
    mUsesGizmos = true;
    mUsesHelper = false;
    mUsingPlaceHolderMesh = false;
}

CBuildingEditor::~CBuildingEditor() {
    if(mRight) {
        mRight->mLeft = 0;
    }
    if(mLeft) {
        mLeft->mRight = 0;
    }
}

Ogre::AxisAlignedBox CBuildingEditor::getAABB() { 
    Ogre::AxisAlignedBox ret; 
    if(mEntityHandle) {
        ret = mEntityHandle->getBoundingBox();
    }
    if(ret.isNull() || ret.isInfinite()) {
        return CNodeEditor::getAABB();
    }
    return ret;
}

Ogre::AxisAlignedBox CBuildingEditor::getWorldAABB() {
    Ogre::AxisAlignedBox ret; 
    if(mEntityHandle) {
        ret = mEntityHandle->getWorldBoundingBox();
    }
    if(ret.isNull() || ret.isInfinite()) {
        return CNodeEditor::getWorldAABB();
    }
    return ret;
}

bool CBuildingEditor::getObjectContextMenu(UTFStringVector &menuitems) 
{
    menuitems.clear();

    if(isLinked())
    {
        menuitems.push_back(OTR("Unlink building"));
        if(mLeft && mRight) {
            menuitems.push_back(OTR("Unlink and join building"));
        }
    } else if(OgitorsRoot::getSingleton().GetBuildingEditor()->findNearestBuilding(this, SEARCH_RADIUS)) {
        menuitems.push_back(OTR("Link building"));
    }

    return true;
}

void CBuildingEditor::onObjectContextMenu(int menuresult) 
{
    if(isLinked()) {
        if(menuresult == 0) {
            unlinkBuilding();
        } else if (menuresult == 1) {
            unlinkAndJoinBuilding();
        }
    } else {
        CBuildingEditor* building = OgitorsRoot::getSingleton().GetBuildingEditor()->findNearestBuilding(this, SEARCH_RADIUS);
        if(building) {
            Ogre::Vector3 final_position = mPosition->get();
            building->addBuilding(this, final_position);
            mProperties.setValue("position", final_position);
        }
    }
}

void CBuildingEditor::unlinkBuilding() {
    if(mLeft) {
        unlinkLeft();
    }
    if(mRight) {
        unlinkRight();
    }
}

void CBuildingEditor::unlinkAndJoinBuilding() {
    CBuildingEditor *left, *right;
    left = right = 0;
    if(mLeft) {
        left = unlinkLeft();
    }
    if(mRight) {
        right = unlinkRight(); 
    }
    if(left && right) {
        left->linkRight(right);
        left->updateGeometry();
    }
}

CBuildingEditor* CBuildingEditor::unlinkLeft() {
    mLeft->mRight = 0;
    mLeft->mRightBuildingName->set("");
    mLeftBuildingName->set("");
    CBuildingEditor* old = mLeft;
    mLeft = 0;
    return old;
}

CBuildingEditor* CBuildingEditor::unlinkRight() {
    mRight->mLeft = 0;
    mRight->mLeftBuildingName->set("");
    mRightBuildingName->set("");
    CBuildingEditor* old = mRight;
    mRight = 0;
    return old;
}

bool CBuildingEditor::load(bool async)
{
    if(CEntityEditor::load(async)) {
        CBuildingEditorFactory* building_editor_factory = OgitorsRoot::getSingleton().GetBuildingEditor();
        CBuildingEditor* building = building_editor_factory->findBuilding(mLeftBuildingName->get());
        if(building) {
            building->linkRight(this);
        }
        building = building_editor_factory->findBuilding(mRightBuildingName->get());
        if(building) {
            building->linkLeft(this);
        }
        
        return true;
    } else {
        return false;
    }
}
// //-----------------------------------------------------------------------------------------
bool CBuildingEditor::unLoad()
{
    if(!mLoaded->get())
        return true;
    
    CEntityEditor::unLoad();
    return true;
}

void CBuildingEditor::setDerivedPosition(Ogre::Vector3 val) {
    if(getParent())
    {
        Ogre::Quaternion qParent = getParent()->getDerivedOrientation().Inverse();
        Ogre::Vector3 vParent = getParent()->getDerivedPosition();
        Ogre::Vector3 newPos = (val - vParent);
        newPos /= getParent()->getDerivedScale();
        val = qParent * newPos;
    }
    
    if(isLinked()) {
        if(mLeft) {
            mLeft->updateGeometry();
        }
        if(mRight) {
            mRight->updateGeometry();
        }
    }
    
    mProperties.setValue("position", val);
}

void CBuildingEditor::setDerivedPositionAndLinkBuilding(Ogre::Vector3 val) {
    if(getParent())
    {
        Ogre::Quaternion qParent = getParent()->getDerivedOrientation().Inverse();
        Ogre::Vector3 vParent = getParent()->getDerivedPosition();
        Ogre::Vector3 newPos = (val - vParent);
        newPos /= getParent()->getDerivedScale();
        val = qParent * newPos;
    }

    Ogre::Vector3 final_position = val;
    
    if(isLinked()) {
        if(mLeft) {
            mLeft->updateGeometry();
        }
        if(mRight) {
            mRight->updateGeometry();
        }
    } else {
        // if there is a near building, add this to it.
        // check query and result lifetime
        CBuildingEditor* building = OgitorsRoot::getSingleton().GetBuildingEditor()->findNearestBuilding(this, SEARCH_RADIUS);
        
        if(building) {
            building->addBuilding(this, final_position);
            lockAdiacentBuilding();
        } else {
            
        }
        
    }
    
    mProperties.setValue("position", final_position);
}
    
void   CBuildingEditor::setDerivedOrientation(Ogre::Quaternion val) {
    CBaseEditor::setDerivedOrientation(val);
    if(isLinked()) {
        if(mLeft) {
            mLeft->updateGeometry();
        }
        if(mRight) {
            mRight->updateGeometry();
        }
    } 
}

void CBuildingEditor::setDerivedScale(Ogre::Vector3 val) {
    CBaseEditor::setDerivedScale(val);
    if(isLinked()) {
        if(mLeft) {
            std::cout << "updating left " << std::endl ;
            mLeft->updateGeometry();
        }
        if(mRight) {
            std::cout << "updating right " << std::endl ;
            mRight->updateGeometry();
        }
    } 
}

void CBuildingEditor::createProperties(OgitorsPropertyValueMap &params)
{
    PROPERTY_PTR(mLeftBuildingName,   "leftBuildingName",  Ogre::String, "",    0, 0);
    PROPERTY_PTR(mRightBuildingName,  "rightBuildingName", Ogre::String, "",    0, 0);
    PROPERTY_PTR(mFixed,              "fixed",             bool,         false, 0, 0);
    
    CEntityEditor::createProperties(params);
}

OBB CBuildingEditor::getOBB() {
    Ogre::AxisAlignedBox aabb = mEntityHandle->getBoundingBox();
    const Ogre::Vector3* corner = aabb.getAllCorners();
    
    OBB ret;
    
    Ogre::Node* node = mHandle;
    Ogre::Matrix4 transform = node->_getFullTransform();
    
    for(uint8_t i = 0; i < 8; ++i) {
        (&ret.bbl)[i] = transform * corner[i] ; 
    }
    return ret;
}

CBuildingEditor* CBuildingEditor::linkLeft(CBuildingEditor* building) {
    // unlink old building.
    CBuildingEditor* old = 0;  
    if(mLeft) {
        old = mLeft;  
        old->mRight = 0;
        old->mRightBuildingName->set("");
    }
    mLeft = building; 
    mLeftBuildingName->set(building->getName());
    building->mRightBuildingName->set(mName->get());
    // free the new building from old links
    if(building->mRight != 0) {
        building->mRight->mLeft = 0;
        building->mRight->mLeftBuildingName->set("");
    }
    building->mRight = this;
    return old;
}

CBuildingEditor* CBuildingEditor::linkRight(CBuildingEditor* building) {
    // unlink old building.
    CBuildingEditor* old = 0; 
    if(mRight) {
        old = mRight;  
        old->mLeft = 0;
        old->mLeftBuildingName->set("");
    }
    mRight = building; 
    mRightBuildingName->set(building->getName());
    building->mLeftBuildingName->set(mName->get());
    // free the new building from old links
    if(building->mLeft != 0) {
       building->mLeft->mRight = 0;
       building->mLeft->mRightBuildingName->set("");
    }
    building->mLeft = this;
    return old;
}

bool CBuildingEditor::addBuilding(CBuildingEditor* building_to_add, Ogre::Vector3 &position) {
    float old_building_width = getWidth();
    
    if(old_building_width - 2 < building_to_add->getWidth()) {
        return false;
    }
    
    //create another building
    OgitorsPropertyValueMap params;
    OgitorsPropertyValue pvalue;
    params["init"] = EMPTY_PROPERTY_VALUE;
    pvalue.propType = PROP_STRING;
    pvalue.val = mProperties.getProperty("meshfile")->getValue();
    params["meshfile"] = pvalue;
    pvalue.propType = PROP_VECTOR3;
    pvalue.val = mProperties.getProperty("position")->getValue();
    params["position"] = pvalue;
    pvalue.propType = PROP_QUATERNION;
    pvalue.val = mProperties.getProperty("orientation")->getValue();
    params["orientation"] = pvalue;
    
    
    CBaseEditor *parent = OgitorsRoot::getSingletonPtr()->GetSceneManagerEditor();
    CBuildingEditor* another = static_cast<CBuildingEditor*>(OgitorsRoot::getSingletonPtr()->CreateEditorObject(0, "Building Object", params, true, true));
    
    {
        CBuildingEditor* old_right = mRight;   
        linkRight(building_to_add);
        another->linkLeft(building_to_add);
        if(old_right) {
            another->linkRight(old_right);
        }
    }
    
    Ogre::Vector3 left_extremity = (getOBB().fbl + getOBB().bbl) / 2;
    Ogre::Vector3 right_extremity = (getOBB().fbr + getOBB().bbr) / 2;
    float sideways_space = old_building_width - building_to_add->getWidth();
    float left_side_width = sideways_space / 2;
    
    setWidth(left_side_width);
    another->mProperties.setValue("scale", mScale->get());
    
    Ogre::Vector3 center = (left_extremity + right_extremity) / 2;
    
    //position 
    Ogre::Vector3 this_left_extremity = (getOBB().fbl + getOBB().bbl) / 2;
    float x = left_extremity.x - this_left_extremity.x;
    float z = left_extremity.z - this_left_extremity.z;
    Ogre::Vector3 this_pos = mPosition->get();
    this_pos.x += x;
    this_pos.z += z;
    mProperties.setValue("position", this_pos);
    
    Ogre::Vector3 another_right_extremity = (another->getOBB().fbr + another->getOBB().bbr) / 2;
    x = right_extremity.x - another_right_extremity.x;
    z = right_extremity.z - another_right_extremity.z;
    Ogre::Vector3 another_pos = another->mPosition->get();
    another_pos.x += x;
    another_pos.z += z;
    another_pos.y = this_pos.y;
    another->mProperties.setValue("position", another_pos);
    
    building_to_add->mProperties.setValue("orientation", mOrientation->get());
    
    position = center;
    return true;
}

void CBuildingEditor::setWidth(float width) {
    Ogre::Vector3 scale = mScale->get();
    if(!scale_const) {
        scale_const = scale.x / getWidth();
    }
    scale.x = width * scale_const;
    mProperties.setValue("scale", scale);
}

TiXmlElement* CBuildingEditor::exportDotScene(TiXmlElement *pParent)
{
    TiXmlElement* pBuilding = CEntityEditor::exportDotScene(pParent)->
                                    InsertEndChild(TiXmlElement("building"))->ToElement();
    return CEntityEditor::exportDotScene(pParent);
}

void CBuildingEditor::updateGeometry() {
    
     if(mFixed->get()) {
        float half_width = getWidth() / 2;
        if(mLockSide == LEFT) {
            Ogre::Vector3 start_right_extremity = getRightExtremity();
            Ogre::Vector3 final_right_extremity = mRight->getLeftExtremity();
            mProperties.setValue("position", mPosition->get() + final_right_extremity - start_right_extremity);
            if(mLeft) {
                mLeft->updateGeometry();
            }
        } else {
            Ogre::Vector3 start_left_extremity = getLeftExtremity();
            Ogre::Vector3 final_left_extremity = mLeft->getRightExtremity();
            mProperties.setValue("position", mPosition->get() + final_left_extremity - start_left_extremity);
            if(mRight) {
                mRight->updateGeometry();
            }
        }
        return;
    }   
    
    if(mLockSide != LEFT) {
        if(mLeft) {
            mLeftExtremity = mLeft->getRightExtremity();
        } else {
            mLeftExtremity = getLeftExtremity();
        }
    } else {
        if(mRight) {
            mRightExtremity = mRight->getLeftExtremity();
        } else {
            mRightExtremity = getRightExtremity();
        }
    }
    
//         Dimension 
    float width = mLeftExtremity.distance(mRightExtremity);
    if(width > 1) {
    setWidth(width);
    
//         Orientation
    Ogre::Radian angle = getAnglePlaneXZ(mLeftExtremity, mRightExtremity);
    Ogre::Matrix3 transform_matrix;
    transform_matrix.FromEulerAnglesXYZ(Ogre::Radian(0), -angle, Ogre::Radian(0));
    Ogre::Quaternion orientation(transform_matrix);
    mProperties.setValue("orientation", orientation);
    
//         Position
    Ogre::Vector3 center = (mLeftExtremity + mRightExtremity) / 2;
    mProperties.setValue("position", center);

    
    }
}
void     CBuildingEditor::setSelectedImpl(bool bSelected) {
    CBaseEditor::setSelectedImpl(bSelected);
    if(bSelected) {
        lockAdiacentBuilding(); 
    }
}
 
void CBuildingEditor::lockAdiacentBuilding() {
    if(mLeft) {
        mLeft->mLockSide = LEFT;
        mLeft->mLeftExtremity = mLeft->getLeftExtremity();
    }
    if(mRight) {
        mRight->mLockSide = RIGHT;
        mRight->mRightExtremity = mRight->getRightExtremity();
    }
}
 
Ogre::Radian CBuildingEditor::getAnglePlaneXZ(const Ogre::Vector3& first, const Ogre::Vector3& second) {
    Ogre::Radian ret(0);
    
    if(second.x == first.x) {
        if(second.z > first.z) {
            return Ogre::Radian(Ogre::Math::PI / 2);
        } else {
            return Ogre::Radian(Ogre::Math::PI * 3 / 2);
        }
    }
    
    float m = (second.z - first.z) / (second.x - first.x);
    
    ret = Ogre::Math::ATan(m);
    if(ret > Ogre::Radian(0)) {
        if(second.x > first.x) {
            return ret;
        } else {
            return ret + Ogre::Radian(Ogre::Math::PI);
        }
    } else {
        if(second.x > first.x) {
            return ret;
        } else {
            return ret + Ogre::Radian(Ogre::Math::PI);
        }
    }
    
    return ret;
}

bool CBuildingEditor::update(float timePassed) {
    return true;
}

CBuildingEditorFactory::CBuildingEditorFactory(OgitorsView *view) : CEntityEditorFactory(view)
{
    mTypeName = "Building Object"; 
    AddPropertyDefinition("leftBuildingName","Left Building Name","The name of the building linked to the left side",PROP_STRING, false, false, false);
    AddPropertyDefinition("rightBuildingName","Right Building Name","The name of the building linked to the right side",PROP_STRING, false, false, false);
    AddPropertyDefinition("fixed", "Fixed", "Wheter this building may automatically be resized and rotated", PROP_BOOL, true, true, false);
}

CBaseEditorFactory *CBuildingEditorFactory::duplicate(OgitorsView *view)
{
    CBaseEditorFactory *ret = OGRE_NEW CBuildingEditorFactory(view);
    ret->mTypeID = mTypeID;

    return ret;
}

CBaseEditor *CBuildingEditorFactory::CreateObject(CBaseEditor **parent, OgitorsPropertyValueMap &params)
{
    CBuildingEditor *building = OGRE_NEW CBuildingEditor(this);
    
    mBuildings.push_back(building);

    OgitorsPropertyValueMap::iterator ni;
    

    if ((ni = params.find("init")) != params.end())
    {      
        Ogre::String entName = Ogre::any_cast<Ogre::String>(params["meshfile"].val);
        int pos = entName.find(".");
        if(pos != -1)
            entName.erase(pos, entName.length() - pos);
        
        params.erase(ni); 
        
        OgitorsPropertyValue value = EMPTY_PROPERTY_VALUE;
        value.val = Ogre::Any(CreateUniqueID(Ogre::String("Building") + Ogre::StringConverter::toString(mInstanceCount)));
        params["name"] = value;
    }

    building->createProperties(params);
    building->mParentEditor->init(*parent);
    
    //TODO: 
//     building->mRenderingDistance->connectTo(OgitorsRoot::getSingletonPtr()->GetSceneManagerEditor()->getProperties()->getProperty("renderingdistance"));
    
    mInstanceCount++;
    return building;
}

void CBuildingEditorFactory::DestroyObject(CBaseEditor *object) {
    mBuildings.remove(static_cast<CBuildingEditor*>(object));
    //call parent instead of origin?
    CBaseEditorFactory::DestroyObject(object);
}

bool CBuildingEditorFactory::CanInstantiate()
{
    return true;
}

CBuildingEditor* CBuildingEditorFactory::findNearestBuilding(const Ogre::Vector3& position, float radius) {
    CBuildingEditor* ret = 0;
    float distance;
    float nearest_distance = radius + 1;
    for(std::list<CBuildingEditor*>::iterator i = mBuildings.begin(); i != mBuildings.end(); ++i) {
        distance = position.distance((*i)->getWorldAABB().getCenter()); 
        if(radius >= distance) {
            if(distance < nearest_distance) {
                nearest_distance = distance;
                ret = (*i);
            }
        }
    }
    return ret;
}

CBuildingEditor* CBuildingEditorFactory::findNearestBuilding(CBuildingEditor* building, float radius) {
    CBuildingEditor* ret = 0;
    Ogre::Vector3 position = building->getWorldAABB().getCenter();
    float distance;
    float nearest_distance = radius + 1;
    for(std::list<CBuildingEditor*>::iterator i = mBuildings.begin(); i != mBuildings.end(); ++i) {
        distance = position.distance((*i)->getWorldAABB().getCenter()); 
        if(radius >= distance) {
            if(distance < nearest_distance && building != *i) {
                nearest_distance = distance;
                ret = (*i);
            }
        }
    }
    return ret;
}

CBuildingEditor* CBuildingEditorFactory::findBuilding(const Ogre::String& name) {
    for(std::list<CBuildingEditor*>::iterator i = mBuildings.begin(); i != mBuildings.end(); ++i) {
        if(name == (*i)->getName()) {
            return *i;
        }
    }
    return 0;
}

}