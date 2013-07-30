/*
 *  sb_resources.cpp
 *  SR
 *
 *  Created by Андрей Куницын on 06.02.11.
 *  Copyright 2011 andryblack. All rights reserved.
 *
 */

#include "sb_resources.h"
#include <ghl_vfs.h>
#include <ghl_render.h>
#include <ghl_image_decoder.h>
#include <ghl_data_stream.h>
#include <ghl_texture.h>
#include <ghl_image.h>
#include <ghl_shader.h>
#include <ghl_data.h>

#include "sb_atlaser.h"

#include "sb_log.h"



namespace Sandbox {

    static const char* MODULE = "Sandbox:Resources";
	
	
	Resources::Resources(GHL::VFS* vfs) :
		m_vfs(vfs),m_render(0),m_image(0) {
			
        m_memory_limit = 20 * 1024 * 1024;
        m_memory_used = 0;
	}
	
    void Resources::Init(GHL::Render* render,GHL::ImageDecoder* image) {
        m_render = render;
        m_image = image;
    }
	Resources::~Resources() {
		
	} 
	
	void Resources::SetBasePath(const char* path) {
		m_base_path = path;
	}

	GHL::DataStream* Resources::OpenFile(const char* filename) {
		std::string fn = m_base_path + filename;
		GHL::DataStream* ds = m_vfs->OpenFile(fn.c_str());
		if (!ds) {
			LogError(MODULE) << "Failed open file " << filename;
			LogError(MODULE) << "full path : " << fn;
		}
		return ds;
	}
	
    GHL::Texture* Resources::InternalCreateTexture( int w,int h, bool alpha,bool fill) {
        if (!m_render) {
            LogError(MODULE) << "render not initialized";
            return 0;
        }
		GHL::TextureFormat tfmt;
		int bpp = 4;
		if (alpha) {
			tfmt = GHL::TEXTURE_FORMAT_RGBA;
			bpp = 4;
		} else {
			tfmt = GHL::TEXTURE_FORMAT_RGB;
			bpp = 3;
        }
        GHL::Image* img = 0;
        if ( fill ) {
            img = GHL_CreateImage(w, h, alpha ? GHL::IMAGE_FORMAT_RGBA : GHL::IMAGE_FORMAT_RGB);
            img->Fill(0);
        }
		GHL::Texture* texture = m_render->CreateTexture(w,h,tfmt,img);
		if (img) img->Release();
		return texture;
	}
    
    bool Resources::LoadImageSubdivs(const char* filename, std::vector<Image>& output) {
        GHL::Image* img = LoadImage(filename);
        if (!img) return false;
        GHL::UInt32 w = img->GetWidth();
        GHL::UInt32 h = img->GetHeight();
        GHL::UInt32 y = 0;
        while ( y < h ) {
            GHL::UInt32 oh = h - y;
            GHL::UInt32 ph = oh <= 16 ? oh : prev_pot( oh );
            GHL::UInt32 iph = ph > oh ? oh : ph;
            GHL::UInt32 x = 0;
            while ( x < w ) {
                GHL::UInt32 ow = w - x;
                GHL::UInt32 pw = ow <= 16 ? ow : prev_pot( ow );
                GHL::UInt32 ipw = pw > ow ? ow : pw;
                GHL::Texture* texture = InternalCreateTexture(pw, ph, ImageHaveAlpha(img), false);
                if (!texture || !ConvertImage(img,texture)) {
                    LogError(MODULE) << "failed load subdiv image " << filename;
                    img->Release();
                    return false;
                }
                GHL::Image* subimg = img->SubImage(x, y, ipw, iph);
                if (!subimg) {
                    LogError(MODULE) << "failed load subdiv image " << filename;
                    img->Release();
                    return false;
                }
                texture->SetData(0,0,subimg);
                subimg->Release();
                output.push_back(Image(sb::make_shared<Texture>(texture),0,0,float(ipw),float(iph)));
                output.back().SetHotspot(Vector2f(-float(x),-float(y)));
                x+=ipw;
            }
            y+=iph;
        }
        return true;
    }
	GHL::Image* Resources::LoadImage(const char* filename,const char** ext) {
        if (!m_image) {
            LogError(MODULE) << "image decoder not initialized";
            return 0;
        }
        if (!m_vfs) {
            LogError(MODULE) << "VFS not initialized";
            return 0;
        }
		std::string fn = m_base_path + filename;
		GHL::DataStream* ds = 0;
		if (fn.find_last_of('.')>fn.find_last_of("/")) {
			ds = m_vfs->OpenFile(fn.c_str());
		}
		if (!ds) {
			std::string file = fn+".png";
			ds = m_vfs->OpenFile( file.c_str() );
			if ( !ds ) {
				file = fn+".tga";
				ds = m_vfs->OpenFile( file.c_str() );
				if ( !ds ) {
					LogError(MODULE) <<"error opening file " << fn;
					return 0;
				} else {
                    if (ext) *ext = "tga";
                }
			} else {
                if (ext) *ext = "png";
            }
		}
		GHL::Image* img = m_image->Decode(ds);
		if (!img) {
			ds->Release();
			LogError(MODULE) <<"error decoding file " << filename;
			return 0;
		}
		ds->Release();
		return img;
	}
    
    bool Resources::GetImageInfo(sb::string& file,GHL::UInt32& w,GHL::UInt32& h) {
        if (!m_image) {
            LogError(MODULE) << "image decoder not initialized";
            return false;
        }
        if (!m_vfs) {
            LogError(MODULE) << "VFS not initialized";
            return false;
        }
        
        std::string fn = m_base_path + file;
		GHL::DataStream* ds = 0;
		if (file.find_last_of('.')!=file.npos) {
			ds = m_vfs->OpenFile(fn.c_str());
		}
		if (!ds) {
			std::string ifile = fn+".png";
			ds = m_vfs->OpenFile( ifile.c_str() );
			if ( !ds ) {
				ifile = fn+".tga";
				ds = m_vfs->OpenFile( ifile.c_str() );
				if ( !ds ) {
					LogError(MODULE) <<"error opening file " << fn;
					return 0;
				} else {
                    file += ".tga";
                }
			} else {
                file += ".png";
            }
		}
        GHL::ImageInfo info;
        if (!m_image->GetFileInfo(ds, &info)) {
            LogError(MODULE) <<"error getting image info for file " << fn;
			return false;
        }
        w = info.width;
        h = info.height;
		return true;
    }
    
	TexturePtr Resources::CreateTexture( GHL::UInt32 w, GHL::UInt32 h,bool alpha, const GHL::Image* data) {
        GHL::UInt32 tw = next_pot( w );
        GHL::UInt32 th = next_pot( h );
        bool setData = ( tw == w ) && ( th == h );
		GHL::Texture* texture = m_render->CreateTexture(tw,
                                                        th,alpha ? GHL::TEXTURE_FORMAT_RGBA:GHL::TEXTURE_FORMAT_RGB,
                                                        setData ? data : 0);
		if (data && !setData) texture->SetData(0,0,data);
        return sb::make_shared<Texture>(texture,w,h);
	}
	
	
	TexturePtr Resources::GetTexture(const char* filename) {
        if (!filename)
            return TexturePtr();
        
        sb::string fn = filename;
        
        
#ifdef SB_RESOURCES_CACHE
        sb::weak_ptr<Texture> al = m_textures[fn];
		if (TexturePtr tex = al.lock()) {
			return tex;
		}
#endif
    
        GHL::UInt32 img_w = 0;
        GHL::UInt32 img_h = 0;
        if (!GetImageInfo(fn,img_w,img_h)) {
            return TexturePtr();
        }
        
        TexturePtr ptr = sb::make_shared<Texture>(fn,need_premultiply,img_w,img_h);
		
#ifdef SB_RESOURCES_CACHE
		m_textures[fn]=ptr;
#endif
        m_managed_textures.push_back(ptr);
        return ptr;
	}
    
    GHL::Texture* Resources::LoadTexture( const sb::string& filename ) {
        const char* ext = "";
		GHL::Image* img = LoadImage(filename.c_str(),&ext);
		if (!img) {
			return 0;
		}
		GHL::TextureFormat tfmt;
        int bpp = 4;
		if (img->GetFormat()==GHL::IMAGE_FORMAT_RGB) {
			tfmt = GHL::TEXTURE_FORMAT_RGB;
            bpp = 3;
		} else if (img->GetFormat()==GHL::IMAGE_FORMAT_RGBA) {
			tfmt = GHL::TEXTURE_FORMAT_RGBA;
            bpp = 4;
		} else {
			img->Release();
			LogError(MODULE) <<"unsupported format file " << filename;
			return 0;
		}
		GHL::UInt32 tw = next_pot( img->GetWidth() );
        GHL::UInt32 th = next_pot( img->GetHeight() );
        
        size_t mem = tw * th * bpp;
        size_t need_release = 0;
        if ((m_memory_used+mem)>m_memory_limit) {
            need_release = m_memory_used + mem - m_memory_limit;
        }
        if (need_release) {
            FreeMemory(need_release, false);
        }
        
        bool setData = ( tw == img->GetWidth() ) && ( th == img->GetHeight() );
		GHL::Image* fillData = setData ? 0 : GHL_CreateImage(tw,th,img->GetFormat());
        if (fillData) fillData->Fill(0);
        GHL::Texture* texture = m_render->CreateTexture(tw,th,tfmt,setData ? img : fillData);
		tfmt = texture->GetFormat();
		GHL::ImageFormat ifmt;
		if (tfmt==GHL::TEXTURE_FORMAT_RGB) {
			ifmt = GHL::IMAGE_FORMAT_RGB;
			bpp = 3;
		}
		else if (tfmt==GHL::TEXTURE_FORMAT_RGBA) {
			ifmt = GHL::IMAGE_FORMAT_RGBA;
			bpp = 4;
		}
		
        m_memory_used += texture->GetWidth() * texture->GetHeight() * bpp;
        
		if (!setData) {
            fillData->Release();
            LogWarning(MODULE) << "image " << filename << "." << ext<< " NPOT " <<
            img->GetWidth() << "x" << img->GetHeight() << " -> " <<
            tw << "x" << th;
            img->Convert(ifmt);
            texture->SetData(0,0,img);
        } else {
			//LogInfo(MODULE) << "Loaded image : " << filename << "." << ext << " " << img->GetWidth() << "x" << img->GetHeight() ;
        }
        texture->DiscardInternal();
        img->Release();
        
        return texture;
    }
    
	bool Resources::ImageHaveAlpha(const GHL::Image* img) const {
		return img->GetFormat()==GHL::IMAGE_FORMAT_RGBA;
	}
	
	bool Resources::ConvertImage(GHL::Image* img,GHL::Texture* tex) const {
		GHL::TextureFormat tfmt = tex->GetFormat();
		GHL::ImageFormat ifmt;
		if (tfmt==GHL::TEXTURE_FORMAT_RGB) {
			ifmt = GHL::IMAGE_FORMAT_RGB;
		}
		else if (tfmt==GHL::TEXTURE_FORMAT_RGBA) {
			ifmt = GHL::IMAGE_FORMAT_RGBA;
		} else {
			LogError(MODULE) <<  "unexpected texture format";
			return false;
		}

		img->Convert(ifmt);
		return true;
	}
	
	ImagePtr Resources::GetImage(const char* filename) {
        TexturePtr texture = GetTexture( filename );
        if (!texture) {
            return ImagePtr();
        }
		GHL::UInt32 imgW = texture->GetOriginalWidth();
		GHL::UInt32 imgH = texture->GetOriginalHeight();
        return sb::make_shared<Image>(texture,0,0,float(imgW),float(imgH));
	}
	
	ShaderPtr Resources::GetShader(const char* vfn,const char* ffn) {
		std::string vfilename = vfn;
		std::string ffilename = ffn;
#ifdef SB_RESOURCES_CACHE
		std::string name = vfilename+"+"+ffilename;
        sb::weak_ptr<Shader> al = m_shaders[name];
		if (ShaderPtr sh = al.lock()) {
			return sh;
		}
#endif
		GHL::VertexShader* vs = 0;
		GHL::FragmentShader* fs = 0;
#ifdef SB_RESOURCES_CACHE
		{
			std::map<std::string,GHL::VertexShader*>::iterator it = m_vshaders.find(vfilename);
			if (it!=m_vshaders.end()) vs = it->second;
		}
		{
			std::map<std::string,GHL::FragmentShader*>::iterator it = m_fshaders.find(ffilename);
			if (it!=m_fshaders.end()) fs = it->second;
		}
#endif
		if (!vs) {
			std::string filename = m_base_path+vfilename;
			GHL::DataStream* ds = m_vfs->OpenFile(filename.c_str());
			if (!ds) {
				LogError(MODULE) << "error opening file " << filename;
				return ShaderPtr();
			}
            GHL::Data* dsd = GHL_ReadAllData( ds );
            ds->Release();
            if (!dsd) {
                LogError(MODULE) << "error loading shader " << vfilename;
            }
			vs = m_render->CreateVertexShader(dsd);
			dsd->Release();
			if (!vs) {
				LogError(MODULE) << "error loading shader " << vfilename;
				//return ShaderPtr();
			}
#ifdef SB_RESOURCES_CACHE
			m_vshaders[vfilename]=vs;
#endif
		}
		if (!fs) {
			std::string filename = m_base_path+ffilename;
			GHL::DataStream* ds = m_vfs->OpenFile(filename.c_str());
			if (!ds) {
				LogError(MODULE) << "error opening file " << filename;
				return ShaderPtr();
			}
            GHL::Data* dsd = GHL_ReadAllData( ds );
            ds->Release();
            if (!dsd) {
                LogError(MODULE) << "error loading shader " << vfilename;
            }
			fs = m_render->CreateFragmentShader(dsd);
			dsd->Release();
			if (!fs) {
				LogError(MODULE) << "error loading shader " << ffilename;
				//return ShaderPtr();
			}
#ifdef SB_RESOURCES_CACHE
			m_fshaders[vfilename]=fs;
#endif
		}
		GHL::ShaderProgram* sp = m_render->CreateShaderProgram(vs,fs);
		if (!sp) {
			LogError(MODULE) << "error creating shader program from " << vfilename << " , " << ffilename ;
			//return ShaderPtr();
		}
		ShaderPtr res = sb::make_shared<Shader>(sp);
#ifdef SB_RESOURCES_CACHE
		m_shaders[name]=res;
#endif
		return res;
	}
	
	
    sb::shared_ptr<Atlaser> Resources::CreateAtlaser(int w,int h) {
        return sb::make_shared<Atlaser>(this,w,h);
    }
    
    RenderTargetPtr Resources::CreateRenderTarget(int w, int h, bool alpha, bool depth) {
        sb_assert(w>0);
        sb_assert(h>0);
        GHL::UInt32 nw = next_pot(w);
        GHL::UInt32 nh = next_pot(h);
        sb_assert(m_render);
        return sb::make_shared<RenderTarget>(m_render->CreateRenderTarget(nw, nh, alpha ? GHL::TEXTURE_FORMAT_RGBA : GHL::TEXTURE_FORMAT_RGB, depth));
    }
    
    size_t    Resources::FreeMemory(size_t need_release,bool full) {
        size_t memory_used = 0;
        for (sb::list<sb::weak_ptr<Texture> >::iterator it = m_managed_textures.begin();it!=m_managed_textures.end();) {
            TexturePtr t = it->lock();
            sb::list<sb::weak_ptr<Texture> >::iterator next = it;
            ++next;
            if (t) {
                memory_used += t->GetMemoryUsage();
                if (need_release) {
                    size_t lt = t->GetLiveTicks();
                    if ( lt && lt < m_live_ticks ) {
                        size_t released = t->Release();
                        memory_used-=released;
                        if (released>need_release) {
                            need_release = 0;
                        } else {
                            need_release -= released;
                        }
                        if (released) {
                            m_managed_textures.push_back(t);
                            next = m_managed_textures.erase(it);
                        }
                    }
                } else {
                    if (!full) break;
                }
            } else {
                next = m_managed_textures.erase(it);
            }
            it = next;
        }
        return memory_used;
    }

	void    Resources::ProcessMemoryMgmt() {
        ++m_live_ticks;
        size_t need_release = 0;
        if (m_memory_used>m_memory_limit) {
            need_release = m_memory_used - m_memory_limit;
        }
        m_memory_used = FreeMemory(need_release,true);
    }
}
