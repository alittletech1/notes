// 0=bottom, 1=top, 
// note: must print bottom with supports!!!
render_component = 1;


// 8.5 inch = 205 mm
$fn = 50;
outside_thickness = 3;
length=130+35; // you lose on the speker
width=165;
height=75;
volume_diameter=6.9; // actual
screw_enclosure=6;
screw_diameter=1.6;

mink_size=1; // roundedness
lip_gap=5; //make top thickness bigger
screw_hole_offset=7;
speaker_diameter = 63; // actual
speaker_depth = 30.2; //actual
led_diameter = 5; // actual
// key pad
// actual size circuit board: 70 x 50
kp_overlap=3; // thickness of the outer square to screw into
kp_height_of_key = 9.4;  //6.4 height of yellow keypad cover - ciruit board base
kp_hole_width=45.3;  // actual
kp_hole_height=63.3; // actual
kp_width=52.6; // actual
kp_height=72.6; // actual
kp_top_thickness = 0.6; //actual  how thick is pad between key and button

epsilon = 100;  // arbitrary number for cutting through gaps
  


if (render_component == 0) {
  difference() { 
    union() {
      screws();
      box();
      speaker_holder();
    }   
    top();
    speaker_hole();      
  }
}

else if (render_component == 1) {
    difference() { 
    
      union() { 
      top();
      keypad_base();
  
      }
      screws_top(); 
      lcd_display(66.5, 36.8); //actual
      on_switch(11.8,5.8); //actual
      volume();
      led_on();
      keypad_key_holes();
      keypad_hole();
      
    }
    
 
  
}


module keypad_base() { 
                  // 3 is zero raised
  
  thickness = 3 + kp_height_of_key;  // there should be 8mm obove circuit board for the keypad cover
  translate([(length/3-(kp_width+kp_overlap))/2,-(width/3-(kp_height+kp_overlap))/2-(kp_height+kp_overlap),height/2-thickness])    
    minkowski() { 
      cube([kp_width+kp_overlap, kp_height+kp_overlap, thickness], center=false);   
      sphere(mink_size);
    }     
  
    
}

module keypad_hole() { 
  //space_for_screw_on_circuit = 3;    
thickness = 3 + kp_height_of_key;
  // circuit board sits on this gap and can be screwed in.
  kp_overlap = 0;
  translate([(length/3-(kp_width+kp_overlap))/2,-(width/3-(kp_height+kp_overlap))/2-(kp_height+kp_overlap),(height/2+outside_thickness/2)-.8-epsilon])   
    cube([kp_width, kp_height, epsilon], center=false);

}


module keypad_key_holes() { 

  //start of the cut (edge, not center)
  kp_startx = 4.3; //actual
  kp_starty = 16; //actual
  cut_size = 8.7;  //actual
  offset_x = 15;
  offset_y = 20;
  circle_d = 7.7;
  echo(kp_overlap);
  x=0;
  y=0;
  translate([(length/3-(kp_width))/2,
    -(width/3-(kp_height))/2,
    (height/2-kp_top_thickness-.2)])
  
  {
    for (yn = [0:10]) { 
        for (xn = [0:2]) { 
            if (yn < 2 || (yn == 2 && xn == 0)) {
            translate([ kp_overlap + kp_startx + xn*offset_x,
                        -kp_overlap - kp_starty - yn*offset_y, 0])
              cube([cut_size, cut_size, kp_top_thickness+epsilon]); 

           // translate([ kp_overlap + kp_startx + xn*offset_x +circle_d/2-.5, // no idea where this .5 comes from?
             //           -kp_overlap - kp_starty - yn*offset_y + circle_d/2-.5, 0])
             //cylinder(h=kp_top_thickness+epsilon, d=circle_d, center=false);
//              echo(kp_overlap + kp_startx + xn*offset_x);
//              echo("Y: " ,  kp_overlap + kp_starty + yn*offset_y);
        }
        
    } 
       
  }}
  
  // debug
/*    translate([(length/2-(kp_width+kp_overlap))/2,
    -(width/2-(kp_height+kp_overlap))/2-10,
    (height/2-kp_top_thickness-.2)])
  cube([100,100,kp_top_thickness+epsilon]);
  */
}

module lcd_display(x, y) { 
  translate([0,(width-lip_gap)/4,0])
  cube([x, y, height+outside_thickness+10], center=true);
  
}

module on_switch(x, y) { 
  buffer_for_speaker = 12;
  translate([-(length-lip_gap)/4+buffer_for_speaker,-40,0])
  cube([x, y, height+outside_thickness+10], center=true);
     
}

module volume() { 
  buffer_for_speaker = 12;
  translate([-(length-lip_gap)/4 + buffer_for_speaker,-20,0])
  cylinder(height+epsilon, d=volume_diameter, center=true);   
}

module led_on() { 
  buffer_for_speaker = 12;
  translate([-(length-lip_gap)/4-15+ buffer_for_speaker,-40,0])
  cylinder(height+epsilon, d=led_diameter, center=true);   
}

module speaker_hole() { 
    rotate([0,90,0])
    translate([0,0,length/2 - speaker_depth/2])
        cylinder(speaker_depth+10,speaker_diameter/2,speaker_diameter/2,true);
    
    // air hole for speaker to breathe
    translate([length/2-10, 0, 0])
    cube([screw_enclosure, screw_enclosure, height], center=true);
    rotate([90,0,0])
    translate([length/2-10, 0, 0])
    cube([screw_enclosure, screw_enclosure, height], center=true);
    
}
module speaker_holder() {
    thickness = 3;
    rotate([0,90,0])
    translate([0,0,length/2 - speaker_depth/2])
        cylinder(speaker_depth,speaker_diameter/2+ thickness,speaker_diameter/2+ thickness,true);
}
module box() {
    difference() {
    minkowski() {
        cube([length,width,height], center=true);
        sphere(mink_size);
    }
    minkowski() {
      cube([length-outside_thickness-6,width-outside_thickness-6,height-outside_thickness], center=true);
     sphere(mink_size);
    }

    }
}

module top() { 
    // cut off the top bevel so its flat.
    translate([0,0,height/2])
      cube([length-lip_gap,width-lip_gap,outside_thickness], center=true);

}

module screw(x, y) { 
    difference() { 
      translate([x, y, 0])
        minkowski() { 
          cube([screw_enclosure, screw_enclosure+10, height], center=true);
            sphere(mink_size);
        } 
        
        translate([x, y, 0])
          cylinder(height, screw_diameter, screw_diameter, center = true);
    }

}

module screws(h=height) { 
  x = (length-outside_thickness-screw_enclosure-screw_hole_offset)/2;
  y = (width-outside_thickness-screw_enclosure-screw_hole_offset)/2;
    // screw1
    screw(x, y);
    screw(-x, y);
    screw(x, -y);
    screw(-x, -y);
    screw(0, y);
    screw(0, -y);
    screw(-x, 0);
}

module screw_top(x, y) { 
  translate([x, y, (height)/2+outside_thickness*1/4])
    cylinder(outside_thickness*1/4, screw_diameter*3.5, screw_diameter*2);
  translate([x, y, (height-outside_thickness)/2])        
    cylinder(outside_thickness*3/4, screw_diameter, screw_diameter);
}    
module screws_top() { 
  x = (length-outside_thickness-screw_enclosure-screw_hole_offset)/2;
  y = (width-outside_thickness-screw_enclosure-screw_hole_offset)/2;

  screw_top(x,y);  
  screw_top(-x, y);
  screw_top(x, -y);
  screw_top(-x, -y);
  screw_top(0, y);
  screw_top(0, -y);
  screw_top(x, 0);
}
