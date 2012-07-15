/*
 *  sb_container.cpp
 *  SR
 *
 *  Created by Андрей Куницын on 11.02.11.
 *  Copyright 2011 andryblack. All rights reserved.
 *
 */

#include "sb_container.h"
#include <algorithm>

SB_META_DECLARE_OBJECT(Sandbox::Container, Sandbox::SceneObject)

namespace Sandbox {

	Container::Container() {
		
	}
	
	Container::~Container() {
		for (std::vector<SceneObjectPtr>::iterator i = m_objects.begin();i!=m_objects.end();i++) {
			(*i)->SetParent(0);
		}
	}
	void Container::Reserve(size_t size) {
		m_objects.reserve(size);
	}
	void Container::AddObject(const SceneObjectPtr& o) {
		sb_assert(o && "null object");
		if (Container* c=o->GetParent()) {
			c->RemoveObject(o);
		}
		o->SetParent(this);
		m_objects.push_back(o);
	}
	void Container::RemoveObject(const SceneObjectPtr& obj) {
		sb_assert( obj && "null object");
		std::vector<SceneObjectPtr>::iterator i = std::find(m_objects.begin(),m_objects.end(),obj);
		if (i!=m_objects.end()) {
			(*i)->SetParent(0);
			m_objects.erase(i);
		}
	}
	void Container::Clear() {
		for (size_t i=0;i<m_objects.size();i++) {
			m_objects[i]->SetParent(0);
		}
		m_objects.clear();
	}
	
	void Container::Draw(Graphics& g) const {
		DrawChilds(g);
	}
	void Container::DrawChilds(Graphics& g) const {
		for (std::vector<SceneObjectPtr>::const_iterator i = m_objects.begin();i!=m_objects.end();++i) {
			(*i)->DoDraw(g);
		}
	}
    bool Container::HandleTouch(const Sandbox::TouchInfo &touch) {
        return HandleChilds(touch,true);
    }
    bool Container::HandleChilds( const TouchInfo& touch, bool firstResponse ) {
        /// copy, for allow scene modifications
        std::vector<SceneObjectPtr> childs = m_objects;
        for (std::vector<SceneObjectPtr>::reverse_iterator i = childs.rbegin();i!=childs.rend();++i) {
			if ( (*i)->DoHandleTouch(touch) && firstResponse) {
                return true;
            }
		}
        return false;
    }
    
    void Container::Update( float dt ) {
        UpdateChilds(dt);
    }
    
    void Container::UpdateChilds( float dt ) {
        /// copy, for allow scene modifications
        std::vector<SceneObjectPtr> childs = m_objects;
        for (std::vector<SceneObjectPtr>::reverse_iterator i = childs.rbegin();i!=childs.rend();++i) {
			(*i)->DoUpdate(dt);
		}
    }
}
