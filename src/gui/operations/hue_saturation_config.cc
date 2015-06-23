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

//#include "../../operations/hue_saturation.hh"

#include "../../base/color.hh"
#include "hue_saturation_config.hh"


class HueEqualizerArea: public PF::CurveArea
{
public:
  void draw_background(const Cairo::RefPtr<Cairo::Context>& cr)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width() - get_border_size()*2;
    const int height = allocation.get_height() - get_border_size()*2;
    const int x0 = get_border_size();
    const int y0 = get_border_size();

    cr->set_source_rgb( 0.5, 0.5, 0.5 );
    for( int x = 0; x < width; x++ ) {
      float h = static_cast<float>(x)*360/width, s = 0.99, l = 0.5;
      //std::cout<<"Hue: "<<h<<std::endl;
      float R, G, B;
      PF::hsl2rgb( h, s, l, R, G, B );
      cr->set_source_rgb( R, G, B );
      cr->move_to( double(0.5+x0+x), double(y0+height-height/1) );
      cr->rel_line_to (double(0), double(height/1) );
      cr->stroke ();
    }

    // Draw grid
    cr->set_source_rgb( 0.9, 0.9, 0.9 );
    std::vector<double> ds (2);
    ds[0] = 4;
    ds[1] = 4;
    cr->set_dash (ds, 0);
    //cr->move_to( double(0.5+x0+width/4), double(y0) );
    //cr->rel_line_to (double(0), double(height) );
    //cr->move_to( double(0.5+x0+width/2), double(y0) );
    //cr->rel_line_to (double(0), double(height) );
    //cr->move_to( double(0.5+x0+width*3/4), double(y0) );
    //cr->rel_line_to (double(0), double(height) );
    cr->move_to( double(x0), double(0.5+y0+height/4) );
    cr->rel_line_to (double(width), double(0) );
    cr->move_to( double(x0), double(0.5+y0+height/2) );
    cr->rel_line_to (double(width), double(0) );
    cr->move_to( double(x0), double(0.5+y0+height*3/4) );
    cr->rel_line_to (double(width), double(0) );
    cr->stroke ();
    cr->unset_dash ();
  }
};


PF::HueSaturationConfigDialog::HueSaturationConfigDialog( PF::Layer* layer ):
  OperationConfigDialog( layer, "B/C/S/H Adjustment" ),
  brightnessSlider( this, "brightness", "Brightness", 0, -100, 100, 5, 10, 100),
  brightness2Slider( this, "brightness_eq", "Brightness (curve)", 0, -100, 100, 5, 10, 100),
  contrastSlider( this, "contrast", "Contrast", 0, -100, 100, 5, 10, 100),
  contrast2Slider( this, "contrast_eq", "Contrast(curve)", 0, -100, 100, 5, 10, 100),
  saturationSlider( this, "saturation", "Saturation", 0, -100, 100, 5, 10, 100),
  saturation2Slider( this, "saturation_eq", "Saturation (curve)", 0, -100, 100, 5, 10, 100),
  hueSlider( this, "hue", "Hue", 0, -180, 180, 0.1, 10, 1),
  hue2Slider( this, "hue_eq", "Hue (curve)", 0, -180, 180, 0.1, 10, 1),
  hueHeq( this, "hue_H_equalizer", new HueEqualizerArea(), 0, 360, 0, 100, 400, 150 ),
  hueSeq( this, "hue_S_equalizer", new PF::CurveArea(), 0, 100, 0, 100, 400, 150 ),
  hueLeq( this, "hue_L_equalizer", new PF::CurveArea(), 0, 100, 0, 100, 400, 150 ),
  hueHeq_enable( this, "hue_H_equalizer_enabled", "Enable", true ),
  hueSeq_enable( this, "hue_S_equalizer_enabled", "Enable", true  ),
  hueLeq_enable( this, "hue_L_equalizer_enabled", "Enable", true  ),
  saturationHeq( this, "saturation_H_equalizer", new HueEqualizerArea(), 0, 360, 0, 100, 400, 150 ),
  saturationSeq( this, "saturation_S_equalizer", new PF::CurveArea(), 0, 100, 0, 100, 400, 150 ),
  saturationLeq( this, "saturation_L_equalizer", new PF::CurveArea(), 0, 100, 0, 100, 400, 150 ),
  contrastHeq( this, "contrast_H_equalizer", new HueEqualizerArea(), 0, 360, 0, 100, 400, 150 ),
  contrastSeq( this, "contrast_S_equalizer", new PF::CurveArea(), 0, 100, 0, 100, 400, 150 ),
  contrastLeq( this, "contrast_L_equalizer", new PF::CurveArea(), 0, 100, 0, 100, 400, 150 )
{
  controlsBox.pack_start( brightnessSlider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( brightness2Slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( sep1, Gtk::PACK_SHRINK );
  controlsBox.pack_start( contrastSlider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( contrast2Slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( sep2, Gtk::PACK_SHRINK );
  controlsBox.pack_start( saturationSlider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( saturation2Slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( sep3, Gtk::PACK_SHRINK );
  controlsBox.pack_start( hueSlider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( hue2Slider, Gtk::PACK_SHRINK );
  //controlsBox.pack_start( sep4, Gtk::PACK_SHRINK );

  curves_nb[0].append_page( hueHeq_box, "Hue curve" );
  curves_nb[0].append_page( hueSeq_box, "Saturation curve" );
  curves_nb[0].append_page( hueLeq_box, "Luminosity curve" );

  hueHeq_box.pack_start( hueHeq, Gtk::PACK_SHRINK );
  hueHeq_box.pack_start( hueHeq_enable_box, Gtk::PACK_SHRINK );
  hueSeq_box.pack_start( hueSeq, Gtk::PACK_SHRINK );
  hueSeq_box.pack_start( hueSeq_enable_box, Gtk::PACK_SHRINK );
  hueLeq_box.pack_start( hueLeq, Gtk::PACK_SHRINK );
  hueLeq_box.pack_start( hueLeq_enable_box, Gtk::PACK_SHRINK );

  hueHeq_enable_box.pack_end( hueHeq_enable, Gtk::PACK_SHRINK );
  hueSeq_enable_box.pack_end( hueSeq_enable, Gtk::PACK_SHRINK );
  hueLeq_enable_box.pack_end( hueLeq_enable, Gtk::PACK_SHRINK );
  //hueHeq_enable_box.pack_end( hueHeq_enable_padding, Gtk::PACK_EXPAND_WIDGET );

  expander_paddings[0][0].set_size_request(20,-1);

  expanders[0][0].set_label( "HSL curves" );
  expanders[0][0].add( expander_hboxes[0][0] );
  expander_hboxes[0][0].pack_start( expander_paddings[0][0], Gtk::PACK_SHRINK );
  expander_hboxes[0][0].pack_start( expander_vboxes[0], Gtk::PACK_SHRINK, 0 );
  expander_vboxes[0].pack_start( brightness2Slider, Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( contrast2Slider, Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( saturation2Slider, Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( hue2Slider, Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( curves_nb[0], Gtk::PACK_SHRINK );

  controlsBox.pack_start( expanders[0][0], Gtk::PACK_SHRINK );
  /*
  controlsBox.pack_start( adjustments_nb );

  adjustments_nb.append_page( adjustment_box[0], "Hue" );
  adjustments_nb.append_page( adjustment_box[1], "Saturation" );
  adjustments_nb.append_page( adjustment_box[2], "Contrast" );

  // Hue
  adjustment_box[0].pack_start( hueSlider, Gtk::PACK_SHRINK );
  adjustment_box[0].pack_start( hue2Slider, Gtk::PACK_SHRINK );
  adjustment_box[0].pack_start( curves_nb[0], Gtk::PACK_SHRINK );

  curves_nb[0].append_page( hueHeq, "Hue curve" );
  curves_nb[0].append_page( hueSeq, "Saturation curve" );
  curves_nb[0].append_page( hueLeq, "Luminosity curve" );

  // Saturation
  adjustment_box[1].pack_start( saturationSlider, Gtk::PACK_SHRINK );
  adjustment_box[1].pack_start( saturation2Slider, Gtk::PACK_SHRINK );
  adjustment_box[1].pack_start( curves_nb[1], Gtk::PACK_SHRINK );

  curves_nb[1].append_page( saturationHeq, "Hue curve" );
  curves_nb[1].append_page( saturationSeq, "Saturation curve" );
  curves_nb[1].append_page( saturationLeq, "Luminosity curve" );

  // Contrast
  adjustment_box[2].pack_start( contrastSlider, Gtk::PACK_SHRINK );
  adjustment_box[2].pack_start( contrast2Slider, Gtk::PACK_SHRINK );
  adjustment_box[2].pack_start( curves_nb[2], Gtk::PACK_SHRINK );

  curves_nb[2].append_page( contrastHeq, "Hue curve" );
  curves_nb[2].append_page( contrastSeq, "Saturation curve" );
  curves_nb[2].append_page( contrastLeq, "Luminosity curve" );
*/
  /*
  controlsBox.pack_start( expanders[0][0] );
  controlsBox.pack_start( expanders[1][0] );
  controlsBox.pack_start( expanders[2][0] );

  expanders[0][0].set_label( "hue adjustment" );
  expanders[1][0].set_label( "saturation adjustment" );
  expanders[2][0].set_label( "contrast adjustment" );

  //for( int i = 0; i < 3; i++ )
  //  for( int j = 0; j < 4; j++ )
  //    expanders[i][j].set_resize_toplevel( true );

  for( int i = 0; i < 3; i++ )
    for( int j = 0; j < 4; j++ )
      expander_paddings[i][j].set_size_request(20,-1);

  expanders[0][0].add( expander_hboxes[0][0] );
  expander_hboxes[0][0].pack_start( expander_paddings[0][0], Gtk::PACK_SHRINK );
  expander_hboxes[0][0].pack_start( expander_vboxes[0], Gtk::PACK_SHRINK, 0 );
  expander_vboxes[0].pack_start( hueSlider, Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( hue2Slider, Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( expanders[0][1], Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( expanders[0][2], Gtk::PACK_SHRINK );
  expander_vboxes[0].pack_start( expanders[0][3], Gtk::PACK_SHRINK );
  expanders[0][1].set_label( "hue curve" );
  expanders[0][1].add( expander_hboxes[0][1] );
  expander_hboxes[0][1].pack_start( expander_paddings[0][1], Gtk::PACK_SHRINK );
  expander_hboxes[0][1].pack_start( hueHeq, Gtk::PACK_SHRINK, 0 );
  expanders[0][2].set_label( "saturation curve" );
  expanders[0][2].add( hueSeq );
  expanders[0][3].set_label( "luminance curve" );
  expanders[0][3].add( hueLeq );
  */
  
  add_widget( controlsBox );
}
