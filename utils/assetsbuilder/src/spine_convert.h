//
//  SconverSpine.h
//  AssetsBuilder
//
//  Created by AndryBlack on 24/06/15.
//
//

#ifndef __AssetsBuilder__spine_convert__
#define __AssetsBuilder__spine_convert__

#include "skeleton_convert.h"

#include <stdio.h>
#include <sb_file_provider.h>
#include <spine/spine.h>
//#include <skelet/sb_skelet_data.h>
#include <pugixml.hpp>
#include <sbstd/sb_map.h>

class Application;
class SpineConvert : public SkeletonConvert {
    SB_META_OBJECT
protected:
    spAtlas*   m_atlas;
    spSkeletonData* m_skeleton;
    spAnimationStateData* m_state;
    sb::string  m_atlas_name;
    void ExportAtlas(Application* app);
    void ExportAnimation();
    bool    m_premultiply_images;
public:
    SpineConvert();
    ~SpineConvert();
    bool Load(const char* atlas, const char* skelet,
              Sandbox::FileProvider* file_provider);
    void Export(const char* file, Application* app);
    struct sp_event_data {
        SpineConvert* this_;
        animation* a;
        size_t frame;
    };
    static void sp_event_listener(spAnimationState* state, int trackIndex, spEventType type, spEvent* event,
                                         int loopCount);
    
};

#endif /* defined(__AssetsBuilder__spine_convert__) */
