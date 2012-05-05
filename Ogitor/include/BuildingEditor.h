/*/////////////////////////////////////////////////////////////////////////////////
/// An
///    ___   ____ ___ _____ ___  ____
///   / _ \ / ___|_ _|_   _/ _ \|  _ \
///  | | | | |  _ | |  | || | | | |_) |
///  | |_| | |_| || |  | || |_| |  _ <
///   \___/ \____|___| |_| \___/|_| \_\
///                              File
///
/// Copyright (c) 2008-2011 Ismail TARIM <ismail@royalspor.com> and the Ogitor Team
//
/// The MIT License
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to deal
/// in the Software without restriction, including without limitation the rights
/// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
/// copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
///
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
/// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
/// THE SOFTWARE.
////////////////////////////////////////////////////////////////////////////////*/

#pragma once

#include <OgreVector3.h>

namespace Ogitors
{
    
    enum LockSide {
        LEFT,
        RIGHT
    };
    
    struct OBB {
        OBB();
        
        OBB(Ogre::Vector3 bbl, Ogre::Vector3 btl, Ogre::Vector3 btr, Ogre::Vector3 bbr, Ogre::Vector3 ftr, Ogre::Vector3 ftl, Ogre::Vector3 fbl, Ogre::Vector3 fbr);
        
        Ogre::Vector3 bbl;
        Ogre::Vector3 btl;
        Ogre::Vector3 btr;
        Ogre::Vector3 bbr;
        Ogre::Vector3 ftr;
        Ogre::Vector3 ftl;
        Ogre::Vector3 fbl;
        Ogre::Vector3 fbr;
    };
    
    class OgitorExport CBuildingEditor : public CEntityEditor
    {
        friend class CBuildingEditorFactory;
        
    public:

        /** @copydoc CBaseEditor::load(bool) */
        virtual bool load(bool async = true);
        /** @copydoc CBaseEditor::unLoad() */
        virtual bool unLoad();
        /** @copydoc CBaseEditor::createProperties(Ogre::NameValuePairList &) */
        virtual void createProperties(OgitorsPropertyValueMap &params);
        /** @copydoc CBaseEditor::getHandle() */
        inline virtual void     *getHandle() { return CEntityEditor::getHandle(); };
        /** @copydoc CBaseEditor::getAABB() */
        virtual Ogre::AxisAlignedBox getAABB();
        /** @copydoc CBaseEditor::getWorldAABB() */
        virtual Ogre::AxisAlignedBox getWorldAABB();
        /** @copydoc CBaseEditor::getObjectContextMenu() */
        virtual bool getObjectContextMenu(UTFStringVector &menuitems);
        /** @copydoc CBaseEditor::onObjectContextMenu() */
        virtual void onObjectContextMenu(int menuresult);
        /** @copydoc CBaseEditor::exportDotScene() */       
        virtual TiXmlElement* exportDotScene(TiXmlElement *pParent); 
        /** @copydoc CBaseEditor::update(float) */
        virtual bool update(float timePassed);
        /** @copydoc CBaseEditor::setDerivedOrientation() */
        virtual void setDerivedOrientation(Ogre::Quaternion val);
        /** @copydoc CBaseEditor::setDerivedScale() */
        virtual void setDerivedScale(Ogre::Vector3 val);
        /** @copydoc CBaseEditor::setSelectedImpl() */
        virtual void setSelectedImpl(bool bSelected);
        /** @copydoc CBaseEditor::setDerivedPosition() */
        virtual void setDerivedPosition(Ogre::Vector3 val);
        /**
         * Search building close to @param val, 
         * if there is a building, add this to it.
         * @param val the position where the search will be made. 
         * this parameter will point where the building position should 
         * be setted.
         */
        virtual void             setDerivedPositionAndLinkBuilding(Ogre::Vector3 val);
        
        inline bool isLinked() {return (mLeft || mRight); };
        void unlinkBuilding();
        void unlinkAndJoinBuilding();
        CBuildingEditor* unlinkLeft();
        CBuildingEditor* unlinkRight();
        
        void updateGeometry();
        

        bool addBuilding(CBuildingEditor* building_to_add, Ogre::Vector3& position);
//         static CBaseEditorFactory *getFactory();

        CBuildingEditor* linkLeft(CBuildingEditor* building);
        CBuildingEditor* linkRight(CBuildingEditor* building);
        
        OBB getOBB();
        
        float getWidth() { return getWorldAABB().getSize().x; };
        void setWidth(float width);
        
        Ogre::Vector3 getLeftExtremity() {return (getOBB().bbl + getOBB().fbl) / 2; };
        Ogre::Vector3 getRightExtremity() {return (getOBB().bbr + getOBB().fbr) / 2; };
        
        CBuildingEditor* getLeft() { return mLeft; };
        CBuildingEditor* getRight() { return mRight; };
        
    protected:
        CBuildingEditor* mLeft;
        CBuildingEditor* mRight;
        float scale_const;

        OgitorsProperty<Ogre::String> *mMeshFile;          /** Mesh file name */
        
        /**
        * Constructor
        * @param factory Handle to terrain editor factory
        */
        CBuildingEditor(CBaseEditorFactory *factory);
        
        /**
        * Destructor
        */
         virtual     ~CBuildingEditor();
         
        Ogre::Vector3 mLeftExtremity;
        Ogre::Vector3 mRightExtremity;
        LockSide mLockSide;
        
        void lockAdiacentBuilding();
        
        OgitorsProperty<Ogre::String> *mLeftBuildingName;   
        OgitorsProperty<Ogre::String> *mRightBuildingName;   
        OgitorsProperty<bool> *mFixed;   
        
        static Ogre::Radian getAnglePlaneXZ(const Ogre::Vector3& first, const Ogre::Vector3& second);
        static const float SEARCH_RADIUS;
       
    };

    class OgitorExport CBuildingEditorFactory: public CEntityEditorFactory
    {
    public:
        /** @copydoc CBaseEditorFactory::CBaseEditorFactory() */
        CBuildingEditorFactory(OgitorsView *view = 0);
//         virtual ~CBuildingEditorFactory() {};
        /** @copydoc CBaseEditorFactory::duplicate(OgitorsView *view) */
        virtual CBaseEditorFactory* duplicate(OgitorsView *view);
        /** @copydoc CBaseEditorFactory::CanInstantiate() */
        virtual bool CanInstantiate();
        /** @copydoc CBaseEditorFactory::CreateObject(CBaseEditor **, OgitorsPropertyValueMap &) */
        virtual CBaseEditor *CreateObject(CBaseEditor **parent, OgitorsPropertyValueMap &params);
        virtual void DestroyObject(CBaseEditor *object);
        
        CBuildingEditor* findNearestBuilding(const Ogre::Vector3& position, float radius);
        CBuildingEditor* findNearestBuilding(CBuildingEditor* building, float radius);
        CBuildingEditor* findBuilding(const Ogre::String& name);
        
    private:
        static std::list<CBuildingEditor*> mBuildings;
        
    };
}

