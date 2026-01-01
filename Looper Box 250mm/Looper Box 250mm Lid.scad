// Matching LID for 250 x 100 x 65 enclosure
// Body refs: WALL=3, TOP_WALL=4 (not used here), LIP_H=2, LIP_W=2
// Bosses: bx1=WALL+10, bx2=W-WALL-10, bxm=W/2; by1=WALL+10, by2=D-WALL-15

// ---------- must match BODY ----------
W = 250;
D = 100;
WALL = 3;
LIP_H = 2;
LIP_W = 2;
$fn = 80;

// ---------- lid parameters ----------
LID_T     = 3.0;   // plate thickness
TONGUE_H  = 1.7;   // a bit under lip height for easy fit
CL        = 0.30;  // clearance around tongue vs inner lip/cavity

// ---------- screw holes (M3) ----------
M3_CLR_D  = 3.2;   // through-hole
CB_D      = 6.5;   // counterbore diameter
CB_T      = 1.8;   // counterbore depth

// ---------- derived from body ----------
innerW = W - 2*WALL;
innerD = D - 2*WALL;

// Boss coordinates (MUST match body)
bx1 = WALL + 10;
bx2 = W - WALL - 10;
bxm = W/2;

by1 = WALL + 10;
by2 = D - WALL - 15;   // rear bosses moved forward in body

boss_xy = [
  [bx1,by1],[bx2,by1],   // front corners
  [bx1,by2],[bx2,by2],   // rear corners
  [bxm,by1],[bxm,by2]    // mid front & mid rear
];

// ---------- model ----------
module lid_plate(){
  difference(){
    // plate + centering tongue
    union(){
      // main plate (z: 0..LID_T)
      cube([W, D, LID_T]);

      // centering tongue (fits inside body against inner lip)
      translate([WALL + LIP_W + CL, WALL + LIP_W + CL, LID_T])
        cube([
          innerW - 2*LIP_W - 2*CL,
          innerD - 2*LIP_W - 2*CL,
          TONGUE_H
        ]);
    }

    // through-holes + counterbores at boss centers
    for (p = boss_xy){
      // M3 clearance through
      translate([p[0], p[1], -0.1])
        cylinder(h = LID_T + TONGUE_H + 1.0, d = M3_CLR_D);

      // counterbore from the outside face (bottom of the lid)
      translate([p[0], p[1], 0])
        cylinder(h = CB_T, d = CB_D);
    }
  }
}

// render lid
lid_plate();
