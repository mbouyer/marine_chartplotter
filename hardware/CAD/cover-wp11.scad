$fn=100;
longueur = 150.5;
largeur = 110.5;
hauteur = 42;
rayon = 20;
base_ep = 1.5;
murs_ep = 1.5;

longueur_e = longueur+murs_ep*2;
largeur_e = largeur+murs_ep*2;
rayon_e = rayon + murs_ep;

longueur_c = longueur / 2 - rayon;
largeur_c = largeur / 2 - rayon;

conge = murs_ep * 2;

module quart_tore() {
	rotate_extrude(angle = 90, convexity=10) translate([rayon, 0, 0])
	    difference() {
		translate([-conge/2, conge/2, 0]) square([conge, conge], center=true);
		translate([-conge, conge, 0]) circle(r=conge, $fn=500);
	    };
}

module quart_rond(l) {
	difference() {
		translate([-conge/2, -conge/2, 0]) cube([conge, conge, l], center=true);
		translate([-conge, -conge, 0]) cylinder(r=conge, h=l, center=true, $fn=500);
	}
}

module quart_tube() {
	intersection() {
		difference() {
			cylinder(r = rayon_e, h = hauteur, center=true, $fn = 500);
			cylinder(r = rayon - 0.001, h = hauteur, center=true, $fn = 500);
		};
		translate([rayon_e/2, rayon_e/2, 0]) cube([rayon_e, rayon_e, hauteur], center=true);
	};
}

module base () {
	union() {
		difference() {
			cube([longueur_e, largeur_e, base_ep], center = true);
			translate([longueur/2, largeur/2, 0]) cube([rayon*2, rayon*2, base_ep], center = true);
			translate([-longueur/2, largeur/2, 0]) cube([rayon*2, rayon*2, base_ep], center = true);
			translate([-longueur/2, -largeur/2, 0]) cube([rayon*2, rayon*2, base_ep], center = true);
			translate([longueur/2, -largeur/2, 0]) cube([rayon*2, rayon*2, base_ep], center = true);
		} ;
		translate([longueur_c, largeur_c, 0]) cylinder(r = (rayon+murs_ep), h = base_ep, center = true, $fn = 500);
		translate([-longueur_c, largeur_c, 0]) cylinder(r = (rayon+murs_ep), h = base_ep, center = true, $fn = 500);
		translate([-longueur_c, -largeur_c, 0]) cylinder(r = (rayon+murs_ep), h = base_ep, center = true, $fn = 500);
		translate([longueur_c, -largeur_c, 0]) cylinder(r = (rayon+murs_ep), h = base_ep, center = true, $fn = 500);

	};
}

module murs() {
	translate([0,0,base_ep/2 + hauteur/2]) union() {
		translate([(longueur+murs_ep)/2, 0, 0]) cube([murs_ep + 0.0001, largeur-rayon*2, hauteur], center = true);
		translate([-(longueur+murs_ep)/2, 0, 0]) cube([murs_ep + 0.0001, largeur-rayon*2, hauteur], center = true);
		translate([0, (largeur+murs_ep)/2, 0]) cube([longueur-rayon*2, murs_ep + 0.0001, hauteur], center = true);
		translate([0, -(largeur+murs_ep)/2, 0]) cube([longueur-rayon*2, murs_ep + 0.0001, hauteur], center = true);
		translate([longueur_c, largeur_c, 0]) quart_tube();
		translate([-longueur_c, largeur_c, 0]) rotate([0,0,90]) quart_tube();
		translate([-longueur_c, -largeur_c, 0]) rotate([0,0,180]) quart_tube();
		translate([longueur_c, -largeur_c, 0]) rotate([0,0,-90]) quart_tube();
	};
}

module conges() {
	translate([0,0,base_ep/2]) union() {
		translate([longueur/2, 0, 0]) rotate([90,90,0]) quart_rond(largeur-rayon*2);
		translate([-longueur/2, 0, 0]) rotate([-90,90,0]) quart_rond(largeur-rayon*2);
		translate([0, largeur/2, 0]) rotate([0,90,0]) quart_rond(longueur-rayon*2);
		translate([0, -largeur/2, 0]) rotate([90,90,-90]) quart_rond(longueur-rayon*2);
		translate([longueur_c, largeur_c, 0]) quart_tore();
		translate([-longueur_c, largeur_c, 0]) rotate([0,0,90]) quart_tore();
		translate([-longueur_c, -largeur_c, 0]) rotate([0,0,180]) quart_tore();
		translate([longueur_c, -largeur_c, 0]) rotate([0,0,-90]) quart_tore();
	};
}

module all() {
	union() {
		base();
		murs();
		conges();

	};
}

all();

// difference() {
// 	all ();
// 	cube([longueur+10, largeur+10, (hauteur-5) * 2], center=true);
// }
// projection(cut=true) translate([0,0, -(base_ep+conge+0.1)]) all();
// projection(cut=true) rotate([90, 0, 0]) all();
