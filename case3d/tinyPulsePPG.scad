$fa = 1.0;
$fs = 1.0;
Length = 80;
Width  = 50;
Height = 20;
Thick = 2;
Rnd    = 6;
BoardHeight = 8;

use <MCAD/boxes.scad>

module pillar(h,r) {
    difference() {
        union() {
            color("blue") cylinder(h=h,r=r);
            color("red")sphere(r=r);
        }
        translate([0,0,h/2]) cylinder(h=(h/2+1),r=1.3);
    }
}

module bottomBox(size,r){
    difference(){
        translate([size[0]/2, size[1]/2,(size[2]+r)/2]) 
            roundedBox([size[0],size[1],size[2]+r],r,false);
        translate([0,0,size[2]-0.1])
            cube([size[0],size[1],r+1]);
    }
}

module bottomCase(size,r,thick,bh){
    difference(){
        bottomBox(size,r);
        translate([thick,thick,thick]) 
            bottomBox([size[0]-thick*2, size[1]-thick*2, size[2]],r-thick);
    }
    translate([size[0]-r+1,size[1]-r+1,r-1])
        pillar(h=size[2]-r+0.88,r=3);
    translate([r-1,r-1,r-1])
        pillar(h=size[2]-r+0.88,r=3);
    translate([12,7,0.2])
        color("red") cylinder(h=size[2]-bh,r=2);
    translate([12,43,0.5])
        color("red") cylinder(h=size[2]-bh,r=2);
    translate([68,7,0.5])
        color("red") cylinder(h=size[2]-bh,r=2);
    translate([68,43,0.5])
        color("red") cylinder(h=size[2]-bh,r=2);
 }

bottomCase([Length,Width,Height],Rnd,Thick,BoardHeight);

module screwhole(thick) {
    color("red") {
        translate([0,0,-0.2]) cylinder(h=1.8,r1=3.0,r2=1.5);
        cylinder(h=thick*2,r=1.5);
    }
}

    
module topPlate(len,wid,r,thick){
    difference(){
        union() {
            translate([len/2,wid/2,thick/2])
                roundedBox([len,wid,thick],r,true);
        }
        translate([32,10,-thick])
            cube([12, 27, thick*3]);
       translate([26,13,-0.2])
            cylinder(h=thick*2,r=2); 
        translate([26,19,-0.2])
            cylinder(h=thick*2,r=1.5); 
        translate([61,24,-0.2])
            cylinder(h=thick*2,r=9); 
        translate([r-1,wid-r+1,0])
             screwhole(thick);
        translate([len-r+1,r-1,0])
             screwhole(thick);
    }
   translate([thick,2*r,thick-0.1])
       color("red") cube([thick,wid-4*r,thick]);
   translate([len-2*thick,2*r,thick-0.1])
       color("red") cube([thick,wid-4*r,thick]);
   translate([2*r,thick,thick-0.1])
       color("green") cube([30-2*r,thick,thick]);
   translate([2*r+36,thick,thick-0.1])
       color("green") cube([30-2*r,thick,thick]);
   translate([2*r,wid-2*thick,thick-0.1])
       color("green") cube([len-4*r,thick,thick]);


}

//topPlate(Length,Width,Rnd,Thick);
