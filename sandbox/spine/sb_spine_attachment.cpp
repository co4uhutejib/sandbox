#include "sb_spine_attachment.h"
#include <spine/extension.h>

namespace Sandbox {
    
    static void SpineImageAttachment_dispose(spAttachment* a) {
        delete static_cast<SpineImageAttachment*>(SUB_CAST(spRegionAttachment, a));
    }
    
    SpineImageAttachment::SpineImageAttachment(const char* name) {
        this->scaleX = 1;
        this->scaleY = 1;
        this->r = 1;
        this->g = 1;
        this->b = 1;
        this->a = 1;
        this->rotation = 0;
        this->width = 0;
        this->height = 0;
        _spAttachment_init(SUPER(this), name, SP_ATTACHMENT_REGION, SpineImageAttachment_dispose);
    }
    
    
    static spAttachment* _ImageAttachmentLoader_createAttachment (spAttachmentLoader* loader, spSkin* skin, spAttachmentType type,
                                                                  const char* name, const char* path) {
        SpineAttachmentLoader* self = static_cast<SpineAttachmentLoader*>(loader);
        switch (type) {
            case SP_ATTACHMENT_REGION: {
                SpineImageAttachment* attachment;
                spAtlasRegion* region = spAtlas_findRegion(self->atlas, path);
                if (!region) {
                    _spAttachmentLoader_setError(loader, "Region not found: ", path);
                    return 0;
                }
                attachment = new SpineImageAttachment(name);
                attachment->rendererObject = region;
                attachment->regionOffsetX = region->offsetX;
                attachment->regionOffsetY = region->offsetY;
                attachment->regionWidth = region->width;
                attachment->regionHeight = region->height;
                attachment->regionOriginalWidth = region->originalWidth;
                attachment->regionOriginalHeight = region->originalHeight;
                
                
                
                return SUPER(attachment);
            }
            case SP_ATTACHMENT_BOUNDING_BOX:
                return SUPER(spBoundingBoxAttachment_create(name));
            default:
                _spAttachmentLoader_setUnknownTypeError(loader, type);
                return 0;
        }
        
        UNUSED(skin);
    }
    
    static void _ImageAttachmentLoader_configureAttachment(spAttachmentLoader* self, spAttachment* a) {
        if (a->type == SP_ATTACHMENT_REGION) {
            SpineImageAttachment* attachment = static_cast<SpineImageAttachment*>(SUB_CAST(spRegionAttachment, a));
            spAtlasRegion* region = static_cast<spAtlasRegion*>(attachment->rendererObject);
            TexturePtr tex(static_cast<Texture*>(region->page->rendererObject));
            
            attachment->image.reset(new Image(tex,float(region->x),
                                              float(region->y),
                                              float(region->rotate ? region->height : region->width),
                                              float(region->rotate ? region->width : region->height)));
            if (region->rotate) {
                attachment->image->SetRotated(true);
                attachment->image->SetSize(float(region->width),float(region->height));
            }
            float oy = float(region->originalHeight-region->height-region->offsetY);
            attachment->image->SetHotspot(Vector2f(float(region->originalWidth / 2 - region->offsetX),
                                                   float(region->originalHeight / 2 - oy)));
            
            float regionScaleX = attachment->width / attachment->regionOriginalWidth * attachment->scaleX;
            float regionScaleY = attachment->height / attachment->regionOriginalHeight * attachment->scaleY;
            
            float radians = attachment->rotation * float(M_PI) / 180.0f;
            attachment->tr.translate(attachment->x,attachment->y);
            attachment->tr.rotate(radians);
            attachment->tr.scale(regionScaleX,-regionScaleY);
        }
    }
    
    SpineAttachmentLoader::SpineAttachmentLoader(spAtlas* atlas) : atlas(atlas) {
        _spAttachmentLoader_init(this, _spAttachmentLoader_deinit, _ImageAttachmentLoader_createAttachment,
                                 _ImageAttachmentLoader_configureAttachment, 0);
    }
    
    SpineAttachmentLoader::~SpineAttachmentLoader() {
        _spAttachmentLoader_deinit(this);
    }

}
