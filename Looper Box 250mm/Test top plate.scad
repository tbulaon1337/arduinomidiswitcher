// =============================
// Looper Box – Top Plate Test
// =============================
// Dimensions match 250 × 100 × 65 mm enclosure
// Prints only the top panel (no walls or lip)
// Thickness = 4 mm (TOP_WALL)

W = 250;
D = 100;
TOP_WALL = 4;
$fn = 80;

// ---------- hole diameters ----------
D_FOOT = 12.0;    // footswitches
D_LED  = 6.7;     // 3 mm LED bezel
D_JACK = 10.0;    // rear jacks (not cut here)
DC_D   = 8.0;     // DC barrel jack (not cut here)
ENC_D  = 7.2;     // Alpha A-6329 encoder
BTN_D  = 12.2;    // PAS6B2M1CESG2-5 push button

// ---------- top layout ----------
FS_FRONT_Y = 18;
FS_BACK_Y  = 68;

front_margin = 15;
front_span = W - 2*front_margin;
function fxf(i,n) = front_margin + front_span*(i/(n-1));
fs_front_x = [ for (i=[0:4]) fxf(i,5) ];   // 5 footswitches

// back row (Tap + Aux)
aux_right_offset = 52.5;
fs_back_x_tap = fs_front_x[4];
right_limit = W - front_margin;
desired_right = fs_back_x_tap + aux_right_offset;
desired_left  = fs_back_x_tap - aux_right_offset;
fs_back_x_aux = (desired_right <= right_limit) ? desired_right : max(front_margin, desired_left);

// LEDs aligned with front FS
LED_Y_ROW = 40;
led_xy = [ for (xv = fs_front_x) [xv, LED_Y_ROW] ];

TAP_LED_X = fs_back_x_tap; TAP_LED_Y = FS_BACK_Y + 14;
AUX_LED_X = fs_back_x_aux; AUX_LED_Y = FS_BACK_Y + 14;

// Controls
OLED_W = 24; OLED_H = 13; OLED_X = 52; OLED_Y = 70;
ENC_X = 135; ENC_Y = 70;
BTN1_X = 110; BTN2_X = 85; BTN_Y = 70;

// ---------- modules ----------
module top_plate(){
  difference(){
    // flat plate
    cube([W, D, TOP_WALL]);

    // Footswitch holes
    for (xv = fs_front_x)
      translate([xv, FS_FRONT_Y, 0])
        cylinder(h = TOP_WALL + 1, d = D_FOOT);

    // Back row FS (Tap + Aux)
    translate([fs_back_x_tap, FS_BACK_Y, 0])
      cylinder(h = TOP_WALL + 1, d = D_FOOT);
    translate([fs_back_x_aux, FS_BACK_Y, 0])
      cylinder(h = TOP_WALL + 1, d = D_FOOT);

    // LEDs
    for (p = led_xy)
      translate([p[0], p[1], 0])
        cylinder(h = TOP_WALL + 1, d = D_LED);
    translate([TAP_LED_X, TAP_LED_Y, 0])
      cylinder(h = TOP_WALL + 1, d = D_LED);
    translate([AUX_LED_X, AUX_LED_Y, 0])
      cylinder(h = TOP_WALL + 1, d = D_LED);

    // OLED window
    translate([OLED_X - OLED_W/2, OLED_Y - OLED_H/2, 0])
      cube([OLED_W, OLED_H, TOP_WALL + 1]);

    // Encoder & Buttons
    translate([ENC_X, ENC_Y, 0])
      cylinder(h = TOP_WALL + 1, d = ENC_D);
    translate([BTN1_X, BTN_Y, 0])
      cylinder(h = TOP_WALL + 1, d = BTN_D);
    translate([BTN2_X, BTN_Y, 0])
      cylinder(h = TOP_WALL + 1, d = BTN_D);
  }
}

// ---------- render ----------
top_plate();
