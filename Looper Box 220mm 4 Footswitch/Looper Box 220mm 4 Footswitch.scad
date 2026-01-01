// Enclosure BODY — 220 x 100 x 65, 3 mm walls, thicker top (4 mm), open bottom
// Top: 4 front FS (Ø12) with 4 LEDs aligned (6.7 mm), 2 back FS (Tap + Aux)
//       Tap LED above Tap FS, Aux LED above Aux FS
// Rear (two even rows, NO USB):
//   Top (7): DC(Ø8), MIDI TRS 6.35, IN, OUT, SEND1, SEND2, SEND3
//   Bot (7): SEND4, RETURN1, RETURN2, RETURN3, RETURN4, TRS SEND/RETURN 5, EXPRESSION TRS
// Notes: straight rows; bottom-edge chamfers to reduce PLA warping; blended bosses (no pilot/nubs)

// ---------- size ----------
W=220; D=100; H=65; WALL=3;
TOP_WALL=4;
$fn=80;

// ---------- hole sizes ----------
D_FOOT = 12.0;   // footswitches
D_LED  = 5.7;    // 3 mm LED bezel panel hole (bezel OD)
D_JACK = 10.0;   // 6.35 mm Lumberg compact (mono/TRS)
DC_D   = 8.0;    // DC barrel jack

// ---------- top layout ----------
FS_FRONT_Y=18;
FS_BACK_Y =68;

// Front footswitches (4 evenly spaced)
front_margin = 15;
front_span   = W - 2*front_margin;
function fxf(i) = front_margin + front_span*(i/3);  // i=0..3
fs_front_x = [ for (i=[0:3]) fxf(i) ];

// Back footswitches: Tap aligned with front FS #4 (rightmost), Aux offset from Tap
aux_right_offset = 35;
fs_back_x_tap = fs_front_x[3];
right_limit   = W - front_margin;
desired_right = fs_back_x_tap + aux_right_offset;
desired_left  = fs_back_x_tap - aux_right_offset;
fs_back_x_aux = (desired_right <= right_limit) ? desired_right : max(front_margin, desired_left);

// LEDs aligned with front FS (nudged up for more room above switches)
LED_Y_ROW = 40;  // was 36
led_xy = [ for (xv = fs_front_x) [xv, LED_Y_ROW] ];

// Tap & Aux LEDs above the back FS
TAP_LED_X = fs_back_x_tap;
AUX_LED_X = fs_back_x_aux;
TAP_LED_Y = FS_BACK_Y + 14;  // raised a touch
AUX_LED_Y = FS_BACK_Y + 14;

// Controls
OLED_W=24; OLED_H=13; OLED_X=45; OLED_Y=70;
ENC_X=110; ENC_Y=70; ENC_D=7;
BTN1_X=96; BTN2_X=82; BTN_Y=70; BTN_D=6;

// ---------- rear layout (TWO even rows, 7 + 7) ----------
REAR_Z_TOP = H/2 + 10;
REAR_Z_BOT = H/2 - 10;

// Keep more side clearance; also ensure clearance from rear-wall bosses
rear_margin = 20;                 // was 18 (gives extra room vs side bosses)
rear_span   = W - 2*rear_margin;

function rxt(i) = rear_margin + rear_span*(i/6);  // 7 columns -> i=0..6
function rxb(i) = rear_margin + rear_span*(i/6);  // 7 columns -> i=0..6

// TOP row order: DC, MIDI, IN, OUT, S1, S2, S3
rear_top = [
  [rxt(0), REAR_Z_TOP, DC_D  ],
  [rxt(1), REAR_Z_TOP, D_JACK],
  [rxt(2), REAR_Z_TOP, D_JACK],
  [rxt(3), REAR_Z_TOP, D_JACK],
  [rxt(4), REAR_Z_TOP, D_JACK],
  [rxt(5), REAR_Z_TOP, D_JACK],
  [rxt(6), REAR_Z_TOP, D_JACK]
];

// BOTTOM row order: S4, R1, R2, R3, R4, TRS S/R 5, EXP TRS
rear_bot = [
  [rxb(0), REAR_Z_BOT, D_JACK],
  [rxb(1), REAR_Z_BOT, D_JACK],
  [rxb(2), REAR_Z_BOT, D_JACK],
  [rxb(3), REAR_Z_BOT, D_JACK],
  [rxb(4), REAR_Z_BOT, D_JACK],
  [rxb(5), REAR_Z_BOT, D_JACK],
  [rxb(6), REAR_Z_BOT, D_JACK]
];

// ---------- lip / bosses ----------
LIP_H=2.0; LIP_W=2.0;
BOSS_H=10.0;
innerW=W-2*WALL; innerD=D-2*WALL;

// Boss anchor positions (blended blocks)
// Pull the rear bosses 5 mm farther from the back wall for jack/nut clearance
bx1=WALL+10; bx2=W-WALL-10; bxm=W/2;
by1=WALL+10; 
by2=D-WALL-15;   // was D-WALL-10 -> more clearance to rear jacks
boss_xy=[[bx1,by1],[bx2,by1],[bx1,by2],[bx2,by2],[bxm,by1],[bxm,by2]];

// ---------- modules ----------
module solid_with_lip(){
  // outer minus interior cavity (leaves TOP_WALL thickness)
  difference(){
    cube([W,D,H]);
    translate([WALL,WALL,0]) cube([innerW,innerD,H-TOP_WALL]);
  }
  // inner lip ring
  translate([WALL,WALL,WALL])
    difference(){
      cube([innerW,innerD,LIP_H]);
      translate([LIP_W,LIP_W,-0.05])
        cube([innerW-2*LIP_W, innerD-2*LIP_W, LIP_H+0.1]);
    }
}

module all_cutters(){
  // TOP: front FS
  for(xv=fs_front_x)
    translate([xv,FS_FRONT_Y,H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=D_FOOT);

  // TOP: back FS (Tap + Aux)
  translate([fs_back_x_tap,FS_BACK_Y,H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=D_FOOT);
  translate([fs_back_x_aux,FS_BACK_Y,H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=D_FOOT);

  // TOP: 4 LEDs aligned with front FS (nudged up)
  for(p=led_xy)
    translate([p[0],p[1],H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=D_LED);

  // TOP: Tap & Aux LEDs above their switches
  translate([TAP_LED_X,TAP_LED_Y,H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=D_LED);
  translate([AUX_LED_X,AUX_LED_Y,H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=D_LED);

  // TOP: OLED + controls
  translate([OLED_X-OLED_W/2,OLED_Y-OLED_H/2,H-TOP_WALL]) cube([OLED_W,OLED_H,TOP_WALL+1]);
  translate([ENC_X,ENC_Y,H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=ENC_D);
  translate([BTN1_X,BTN_Y,H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=BTN_D);
  translate([BTN2_X,BTN_Y,H-TOP_WALL]) cylinder(h=TOP_WALL+1,d=BTN_D);

  // REAR: two rows (short bores through rear wall)
  for(r=rear_top) translate([r[0], D+5, r[1]]) rotate([90,0,0]) cylinder(h=WALL+6, d=r[2]);
  for(r=rear_bot) translate([r[0], D+5, r[1]]) rotate([90,0,0]) cylinder(h=WALL+6, d=r[2]);
}

// Blended boss (rectangular block tied into wall) — NO pilot holes (flat tops)
module wall_boss(xc, yc){
  boss_size = 14;                   // footprint width (square)
  boss_h    = LIP_H + BOSS_H;
  translate([xc - boss_size/2, yc - boss_size/2, WALL])
    cube([boss_size, boss_size, boss_h]);
}

// Ribs (beefier)
RIB_W=10;
module rib_front(xc,yc){ translate([xc-RIB_W/2, WALL, WALL]) cube([RIB_W, yc-WALL+7, LIP_H+BOSS_H]); }
module rib_back (xc,yc){ translate([xc-RIB_W/2, yc-7, WALL]) cube([RIB_W, D-WALL-(yc-7), LIP_H+BOSS_H]); }
module rib_left (xc,yc){ translate([WALL, yc-RIB_W/2, WALL]) cube([xc-WALL+7, RIB_W, LIP_H+BOSS_H]); }
module rib_right(xc,yc){ translate([xc-7, yc-RIB_W/2, WALL]) cube([W-WALL-(xc-7), RIB_W, LIP_H+BOSS_H]); }

// Bottom-edge chamfers (2 mm 45°)
CHAMFER=2;
module subtract_bottom_chamfers(){
  module edge_wedge_x(y0){
    translate([0,y0,0]) rotate([0,90,0])
      linear_extrude(height=W) polygon(points=[[0,0],[CHAMFER,0],[0,CHAMFER]]);
  }
  module edge_wedge_y(x0){
    translate([x0,0,0]) rotate([-90,0,0])
      linear_extrude(height=D) polygon(points=[[0,0],[CHAMFER,0],[0,CHAMFER]]);
  }
  edge_wedge_x(0);
  edge_wedge_x(D-CHAMFER);
  edge_wedge_y(0);
  edge_wedge_y(W-CHAMFER);
}

// ---------- CSG ----------
difference(){
  solid_with_lip();
  all_cutters();
  subtract_bottom_chamfers();
}

// ---------- bosses & ribs ----------
for(p=boss_xy) wall_boss(p[0],p[1]);
for(p=boss_xy){
  xc=p[0]; yc=p[1];
  if(yc==by1) rib_front(xc,yc);
  if(yc==by2) rib_back (xc,yc);
  if(xc==bx1) rib_left (xc,yc);
  if(xc==bx2) rib_right(xc,yc);
}
