/* 
 */

/*

    Copyright (C) 2014 Ferrero Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.


 */

/*

    These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/

 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "../base/processor_imp.hh"
#include "guided_filter.hh"
#include "local_contrast_v2.hh"


class LogLumiPar: public PF::OpParBase
{
  PF::ICCProfile* profile;
  float anchor;
public:
  LogLumiPar():
    PF::OpParBase(), profile(NULL), anchor(0.5) {

  }

  PF::ICCProfile* get_profile() { return profile; }
  float get_anchor() { return anchor; }
  void set_anchor(float a) { anchor = a; }


  VipsImage* build(std::vector<VipsImage*>& in, int first,
      VipsImage* imap, VipsImage* omap,
      unsigned int& level)
  {
    if( in.empty() ) {printf("LogLumiPar::build(): in.empty()\n"); return NULL;}

    VipsImage* image = in[0];
    if( !image ) {printf("LogLumiPar::build(): image==NULL\n"); return NULL;}

    profile = PF::get_icc_profile( image );
    if( !profile ) {printf("LogLumiPar::build(): profile==NULL\n"); return NULL;}

    grayscale_image( get_xsize(), get_ysize() );

    VipsImage* out = OpParBase::build( in, first, NULL, NULL, level );
    //printf("LogLumiPar::build(): out=%p\n", out);

    return out;
  }
};


template < OP_TEMPLATE_DEF >
class LogLumiProc
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
  }
};


template < class BLENDER, int CHMIN, int CHMAX, bool has_imap, bool has_omap, bool PREVIEW >
class LogLumiProc< float, BLENDER, PF::PF_COLORSPACE_GRAYSCALE, CHMIN, CHMAX, has_imap, has_omap, PREVIEW >
{
public:
  void render(VipsRegion** ireg, int n, int in_first,
      VipsRegion* imap, VipsRegion* omap,
      VipsRegion* oreg, PF::OpParBase* par)
  {
    static const double inv_log_base = 1.0 / log(10.0);
    LogLumiPar* opar = dynamic_cast<LogLumiPar*>(par);
    if( !opar ) return;
    VipsRect *r = &oreg->valid;
    int line_size = r->width * oreg->im->Bands; //layer->in_all[0]->Bands;
    int width = r->width;
    int height = r->height;
    PF::ICCProfile* profile = opar->get_profile();
    if( !profile ) return;

    const float bias = 1.0f/profile->perceptual2linear(opar->get_anchor());

    float L, pL;
    float* pin;
    float* pout;
    int x, y, R;

    for( y = 0; y < height; y++ ) {
      pin = (float*)VIPS_REGION_ADDR( ireg[0], r->left, r->top + y );
      pout = (float*)VIPS_REGION_ADDR( oreg, r->left, r->top + y );

      for( x = 0; x < width; x++, pin+=3, pout++ ) {
        L = profile->get_lightness(pin[0], pin[1], pin[2]);
        L *= bias;
        pL = (L>1.0e-16) ? xlog10( L ) : -16;

        pout[0] = pL;
      }
    }
  }
};



PF::LocalContrastV2Par::LocalContrastV2Par():
  OpParBase(), 
  amount("amount",this,1),
  radius("radius",this,64),
  threshold("threshold",this,0.1),
  white_level("white_level",this,0.8)
{
  loglumi = new PF::Processor<LogLumiPar,LogLumiProc>();
  int ts = 1;
  for(int gi = 0; gi < 10; gi++) {
    guided[gi] = new_guided_filter();
    threshold_scale[gi] = ts;
    ts *= 2;
  }
  set_type("local_contrast_v2" );

  set_default_name( _("local contrast") );
}


static void fill_rv(int* rv, float* tv, float radius, int level)
{
  int gi = 0;
  for(gi = 0; gi < 10; gi++) rv[gi] = 0;

  int r = 1;
  float ts = 1;
  for( unsigned int l = 1; l <= level; l++ )
    r *= 2;

  /*
  for(gi = 0; gi < 1; gi++) {
    rv[gi] = r;
    tv[gi] = ts;
  }
  r *= 4;
  ts *= 1.5;
  */
  for(gi = 0; gi < 9; gi++) {
    if( r >= radius ) break;
    rv[gi] = r;
    tv[gi] = ts;
    ts *= 1.5;
    if( (r*4) > radius ) break;
    r *= 4;
    //break;
  }
  rv[gi] = radius;
  tv[gi] = ts;
}



void PF::LocalContrastV2Par::propagate_settings()
{
  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    guidedpar->propagate_settings();
  }
}



void PF::LocalContrastV2Par::compute_padding( VipsImage* full_res, unsigned int id, unsigned int level )
{
  int tot_padding = 0;
  int rv[10] = { 0 };
  float tv[10] = { 0 };
  rv[0] = radius.get(); tv[0] = 1;
  fill_rv(rv, tv, radius.get(), level);

  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    std::cout<<"ShadowsHighlightsV2Par::compute_padding: rv["<<gi<<"]="<<rv[gi]<<std::endl;
    if(rv[gi] == 0) break;
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold( threshold.get() / threshold_scale[gi] );
    int subsampling = 1;
    while( subsampling <= 16 ) {
      if( (subsampling*8) >= rv[gi] ) break;
      subsampling *= 2;
    }
    guidedpar->set_subsampling(subsampling);
    guidedpar->propagate_settings();
    guidedpar->compute_padding(full_res, id, level);
    tot_padding += guidedpar->get_padding(id);
  }

  set_padding( tot_padding, id );
}



VipsImage* PF::LocalContrastV2Par::build(std::vector<VipsImage*>& in, int first,
				     VipsImage* imap, VipsImage* omap, unsigned int& level)
{
  if( (in.size()<1) || (in[0]==NULL) )
    return NULL;


  std::vector<VipsImage*> in2;
  in_profile = PF::get_icc_profile( in[0] );

  LogLumiPar* logpar = dynamic_cast<LogLumiPar*>( loglumi->get_par() );
  VipsImage* logimg = NULL;
  if(logpar) {
    logpar->set_anchor( 0.5 );
    logpar->set_image_hints( in[0] );
    logpar->set_format( get_format() );
    in2.clear(); in2.push_back( in[0] );
    logimg = logpar->build( in2, 0, NULL, NULL, level );
  } else {
    logimg = in[0];
    PF_REF(logimg, "ShadowsHighlightsV2Par::build: in[0] ref");
  }

  std::cout<<"ShadowsHighlightsV2Par::build(): logimg="<<logimg<<std::endl;

  if( !logimg ) return NULL;




  int rv[10] = { 0 };
  float tv[10] = { 0 };
  rv[0] = radius.get(); tv[0] = 1;
  fill_rv(rv, tv, radius.get(), level);

  VipsImage* timg = logimg;
  VipsImage* smoothed = logimg;
  std::cout<<"ShadowsHighlightsV2Par::build(): radius="<<radius.get()<<std::endl;

  for(int gi = 0; gi < 10; gi++) {
    PF::GuidedFilterPar* guidedpar = dynamic_cast<PF::GuidedFilterPar*>( guided[gi]->get_par() );
    if( !guidedpar ) break;
    if( rv[gi] == 0 ) break;
    guidedpar->set_image_hints( timg );
    guidedpar->set_format( get_format() );
    //std::cout<<"ShadowsHighlightsV2Par::build(): gi="<<gi<<"  radius="<<rv[gi]<<std::endl;
    guidedpar->set_radius( rv[gi] );
    guidedpar->set_threshold(threshold.get() / tv[gi]);
    int subsampling = 1;
    while( subsampling <= 16 ) {
      if( (subsampling*8) >= rv[gi] ) break;
      subsampling *= 2;
    }
    guidedpar->set_subsampling(subsampling);
    guidedpar->set_convert_to_perceptual( false );
    guidedpar->propagate_settings();
    guidedpar->compute_padding(timg, 0, level);

    std::cout<<"ShadowsHighlightsV2Par::build(): gi="<<gi<<"  radius="
        <<rv[gi]<<"  padding="<<guidedpar->get_padding(0)
        <<"  logimg="<<logimg<<"  smoothed="<<smoothed<<std::endl;
    if( guidedpar->get_padding(0) > 64 ) {
      int ts = 128;
      int tr = guidedpar->get_padding(0) / ts + 1;
      int nt = (timg->Xsize/ts) * tr + 1;
      VipsAccess acc = VIPS_ACCESS_RANDOM;
      int threaded = 1, persistent = 0;
      VipsImage* cached = NULL;
      if( phf_tilecache(timg, &cached,
          "tile_width", ts,
          "tile_height", ts,
          "max_tiles", nt,
          "access", acc, "threaded", threaded,
          "persistent", persistent, NULL) ) {
        std::cout<<"ShadowsHighlightsV2Par::build(): vips_tilecache() failed."<<std::endl;
        return NULL;
      }
      PF_UNREF( timg, "ShadowsHighlightsV2Par::build(): cropped unref" );
      timg = cached;
    }

    in2.clear();
    in2.push_back( timg );
    if( gi==0 && smoothed != logimg ) {
      std::cout<<"ShadowsHighlightsV2Par::build(): adding smoothed guide image"<<std::endl;
      in2.push_back( smoothed );
    }
    smoothed = guidedpar->build( in2, first, NULL, NULL, level );
    if( !smoothed ) {
      std::cout<<"ShadowsHighlightsV2Par::build(): NULL local contrast enhanced image"<<std::endl;
      return NULL;
    }
    PF_UNREF(timg, "ShadowsHighlightsV2Par::build(): timg unref");
    timg = smoothed;
  }


  if( !smoothed ) {
    std::cout<<"ShadowsHighlightsV2Par::build(): null Lab image"<<std::endl;
    PF_REF( in[0], "ShadowsHighlightsV2Par::build(): null Lab image" );
    return in[0];
  }


  in2.clear();
  in2.push_back(smoothed);
  in2.push_back(logimg);
  in2.push_back(in[0]);
  VipsImage* out = OpParBase::build( in2, 0, imap, omap, level );
  PF_UNREF( smoothed, "ShadowsHighlightsV2Par::build() smoothed unref" );
  set_image_hints( in[0] );

  return out;
}


PF::ProcessorBase* PF::new_local_contrast_v2()
{ return new PF::Processor<PF::LocalContrastV2Par,PF::LocalContrastV2Proc>(); }
