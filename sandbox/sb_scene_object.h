/*
 *  sb_scene_object.h
 *
 *  Created by Андрей Куницын on 08.02.11.
 *  Copyright 2011 andryblack. All rights reserved.
 *
 */

#ifndef SB_SCENE_OBJECT_H
#define SB_SCENE_OBJECT_H

#include "meta/sb_meta.h"
#include "sb_vector2.h"
#include "sb_transform2d.h"

namespace Sandbox {
	
	class Graphics;
	class Container;
    class Scene;

	class SceneObject : public meta::object {
	    SB_META_OBJECT
    public:
    	SceneObject();
		virtual ~SceneObject();
		/// self drawing implementation
		virtual void Draw(Graphics& g) const = 0;
        /// self update object
        virtual void Update( float dt ) {(void)dt;}
        /// visible
		void SetVisible(bool v) { m_visible = v;}
		bool GetVisible() const { return m_visible;}
        
        Vector2f GlobalToLocal(const Vector2f& v) const;
        void MoveToTop();
        Transform2d GetTransform() const;
        Vector2f LocalToGlobal(const Vector2f& v) const;
	protected:
		friend class Container;
		Container* GetParent() const { return m_parent;}
        Scene*  GetScene() const;
        virtual void GlobalToLocalImpl(Vector2f& v) const;
        virtual void GetTransformImpl(Transform2d& tr) const;
	private:
		void SetParent(Container* parent);
		Container* m_parent;
		bool	m_visible;
	};
	typedef sb::intrusive_ptr<SceneObject> SceneObjectPtr;
    
    class SceneObjectWithPosition : public SceneObject {
        SB_META_OBJECT
    public:
        void SetPos(const Vector2f& pos) { m_pos=pos;}
		const Vector2f& GetPos() const { return m_pos;}
        void Move(const Vector2f& d) { m_pos += d; }
        void MoveX(float x) { m_pos.x += x; }
        void MoveY(float y) { m_pos.y += y; }
    private:
        Vector2f    m_pos;
        virtual void GlobalToLocalImpl(Vector2f& v) const;
        virtual void GetTransformImpl(Transform2d& tr) const;
    };
}

#endif /*SB_SCENE_OBJECT_H*/
