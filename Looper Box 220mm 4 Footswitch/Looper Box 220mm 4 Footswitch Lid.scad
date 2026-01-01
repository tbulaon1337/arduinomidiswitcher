// Matching LID for 220 x 100 x 65 enclosure (body: WALL=3, TOP_WALL=4, LIP_H=2, LIP_W=2)
// - Flat plate with centering tongue against inner lip
// - M3 clearance holes + counterbores aligned to blended boss blocks
// - Body's rear bosses are at by2 = D - WALL - 15 (moved forward) — mirrored here

// ---------- must match BODY ----------
W = 220;
D = 100;
WALL = 3;
LIP_H = 2;
LIP_W = 2;
$fn = 80;

// ---------- lid parameters ----------
LID_T    = 3.0;    // lid thickness
TONGUE_H = 1.7;    // slightly less than lip (easy fit)
CL       = 0.30;   // clearance around tongue

// ---------- screw holes (M3 through, heads recessed) ----------
M3_CLR_D = 3.2;    // through-hole in lid
CB_D     = 6.5;    // counterbore diameter
CB_T     = 1.8;    // counterbore depth

// ---------- derived (match body cavity & boss XY) ----------
innerW = W - 2*WALL;
innerD = D - 2*WALL;

// Boss coordinates (MUST match body)
// bx1=WALL+10; bx2=W-WALL-10; bxm=W/2;
// by1=WALL+10; by2=D-WALL-15;   // note: -15 per your latest body
bx1 = WALL + 10;
bx2 = W - WALL - 10;
bxm = W/2;
by1 = WALL + 10;
by2 = D - WALL - 15;

boss_xy = [
  [bx1,by1],[bx2,by1],   // front corners
  [bx1,by2],[bx2,by2],   // rear corners (moved forward)
  [bxm,by1],[bxm,by2]    // front & rear mid
];

// ---------- model ----------
module lid_plate(){
  difference(){
    // Plate + centering tongue
    union(){
      // main plate
      cube([W, D, LID_T]);

      // centering tongue (fits inside, clears lip by CL)
      translate([WALL + LIP_W + CL, WALL + LIP_W + CL, LID_T])
        cube([ innerW - 2*LIP_W - 2*CL,
               innerD - 2*LIP_W - 2*CL,
               TONGUE_H ]);
    }

    // through-holes + counterbores at boss centers
    for (p = boss_xy){
      // through (M3 clearance)
      translate([p[0], p[1], -0.1])
        cylinder(h = LID_T + TONGUE_H + 1.0, d = M3_CLR_D);

      // counterbore from outside face (bottom of lid)
      translate([p[0], p[1], 0])
        cylinder(h = CB_T, d = CB_D);
    }
  }
}

// render lid
lid_plate();
