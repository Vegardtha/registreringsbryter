// =============================================================================
// Registreringsbryter – kabinett
// Parametrisk modell. Se CAD-SPEK.md for bakgrunn.
//
// Velg hva som vises/eksporteres med `del` (eller Customizer-panelet i OpenSCAD).
//
// Printinnstillinger: PLA, 0,2 mm lag, 3 perimetre, 15 % fyll, ingen støtte.
//   Ca. 210 °C dyse, 60 °C plate, full kjølevifte.
//   kasse : bunnen ned
//   lokk  : toppen OPP (føringsringene står opp). Skru gjerne på «ironing» for pen topp.
//   hette : toppen ned (4 stk.)
// Print `hette_test` + én `hette` først for å sjekke passform og klikk.
// =============================================================================

/* [Hva som vises] */
del = "samlet"; // ["samlet", "kasse", "lokk", "hette", "print", "hette_test", "led_test"]

/* [Boks] */
bredde        = 140;  // ytre bredde (venstre–høyre)
dybde         = 80;   // ytre dybde (front–bak)
hoyde_front   = 32;   // ytre høyde foran
hoyde_bak     = 48;   // ytre høyde bak
vegg          = 2.4;
bunn_tykkelse = 2.4;
lokk_tykkelse = 2.4;  // maks 3,0 der knappene sitter
hjorneradius  = 4;

/* [Skruer (4 × M3)] */
skrue_hull_stolpe = 2.6;  // 2.6 = selvgjengende M3, 4.0 = innpresset gjengeinnsats
skrue_hull_lokk   = 3.4;
stolpe_d          = 7;

/* [Trykknapp – MÅL SELV] */
knapp_hull        = 10.2; // gjengen er ca. 10 mm (testet: 10,2 passer)
blaa_hette_d      = 8.5;  // diameter på den blå knapphetten (mål selv)
blaa_hette_hoyde  = 9.0;  // toppen av blå hette over panelet, med mutter på (mål selv)
vandring          = 2.0;  // hvor langt knappen går før den klikker (mål selv)
mutter_topp       = 2.5;  // toppen av mutter (+ evt. skive) over panelet (mål selv)
mutter_d          = 15;   // mutteren over hjørnene (mål selv)
knapp_hus_lengde  = 20;   // hus + loddebein under panelet (mål selv)
knapp_hus_d       = 13;   // (mål selv)

/* [Stor hette] */
hette_d     = 24;   // øk denne og knapp_avstand like mye for større knapper
hette_topp  = 2.4;
hette_vegg  = 1.6;
hette_fas   = 0.6;
hette_grop  = 0;    // fingergrop i mm. 0 = flat (best når hetten printes med toppen ned)
hylse_vegg  = 1.6;
hylse_dybde = 3.5;  // hvor langt hylsa tres ned over den blå hetten
hylse_klem  = 0.1;  // presspasning; øk hvis hetten sitter løst, senk hvis den ikke går på

/* [Føringsring] */
foring_hoyde   = 6.5;
foring_vegg    = 1.6;
foring_klaring = 0.3;

/* [LED 10 mm – MÅL SELV] */
led_kropp         = 10.0; // diameter på LED-kroppen
led_hull          = 10.4; // hullet før ribbene
led_ribbe_d       = 9.9;  // avstand mellom ribbetuppene; ribbene klemmer og sentrerer LED-en. 0 = ingen ribber
led_ribber        = 6;
led_krage         = 11.0; // kragen på det bredeste (mål selv)
led_krage_flat    = 10.0; // kragen målt fra den flate siden og over (mål selv). Sett lik led_krage hvis kragen er rund
led_flat_retning  = 90;   // retning på den flate siden (minus/katode): 90 = bakover
led_krage_klaring = 0.4;
led_krage_dybde   = 1.0;  // forsenkning på undersiden som kragen ligger i
led_krage_tykk    = 1.5;  // (mål selv)
led_hoyde         = 13.8; // fra underkant krage til topp kuppel (mål selv)

/* [Panel-oppsett (mål langs panelet fra frontkanten)] */
knapp_avstand    = 32;
etikett_pos      = 12;
knapp_pos        = 33;
led_pos          = 56;
etikett_b        = 26;
etikett_h        = 9;
etikett_dybde    = 0.4;
nummer_dybde     = 0.6;
nummer_storrelse = 4;

/* [ESP32-C3 SuperMini] */
esp_l          = 22.5;
esp_b          = 18.0;
esp_pcb        = 1.2;  // (mål selv)
esp_avstand    = 1.5;  // luft under kortet (for loddepunkter)
esp_x          = 70;   // senter av kortet i bredden
usb_hull_b     = 13.0;
usb_hull_h     = 7.5;
usb_over_pcb   = 1.6;  // senter av USB-C over kortets overside

/* [Valg] */
med_stripboard = true;
med_vektlomme  = true;

/* [Hidden] */
$fn = 64;
eps = 0.01;
farger = ["#e5484d", "#30a46c", "#f5a524", "#3e63dd"];

// -----------------------------------------------------------------------------
// Avledede mål
// -----------------------------------------------------------------------------
vinkel       = atan((hoyde_bak - hoyde_front) / dybde);
k            = tan(vinkel);
lokk_v       = lokk_tykkelse / cos(vinkel);   // loddrett tykkelse på lokket
panel_lengde = dybde / cos(vinkel);
knapp_x      = [for (i = [0:3]) bredde / 2 + (i - 1.5) * knapp_avstand];
stolper      = [[vegg + 3, vegg + 3], [bredde - vegg - 3, vegg + 3],
                [vegg + 3, dybde - vegg - 3], [bredde - vegg - 3, dybde - vegg - 3]];
fotter       = [[8, 8], [bredde - 8, 8], [8, dybde - 8], [bredde - 8, dybde - 8]];
esp_y0       = dybde - vegg - esp_l;             // fremkant av kortet
usb_z        = bunn_tykkelse + esp_avstand + esp_pcb + usb_over_pcb;

hette_topp_under = blaa_hette_hoyde;             // hetten hviler på den blå hetten
hette_toppflate  = hette_topp_under + hette_topp;
skjort_bunn      = vandring + 1;
hylse_indre      = blaa_hette_d - hylse_klem;

vektlomme_pos = [bredde / 2 - 20, vegg + 3];     // 40 × 30, foran på midten
vektlomme_mal = [40, 30, 6];
stripboard_pos = [esp_x + esp_b / 2 + 10, dybde - vegg - 28];  // 35 × 25

// -----------------------------------------------------------------------------
// Kontroller (skrives i konsollen; stopper rendring hvis noe er feil)
// -----------------------------------------------------------------------------
function under_lokk_z(u) = hoyde_front + u * sin(vinkel) - lokk_tykkelse * cos(vinkel);
function over_vektlomme(x, y) = med_vektlomme
    && x > vektlomme_pos[0] - knapp_hus_d / 2 && x < vektlomme_pos[0] + vektlomme_mal[0] + knapp_hus_d / 2
    && y > vektlomme_pos[1] - knapp_hus_d / 2 && y < vektlomme_pos[1] + vektlomme_mal[1] + knapp_hus_d / 2;

knapp_y        = knapp_pos * cos(vinkel);
fri_hoyde      = [for (x = knapp_x) under_lokk_z(knapp_pos)
                    - (over_vektlomme(x, knapp_y) ? bunn_tykkelse + vektlomme_mal[2] : bunn_tykkelse)];
min_fri_hoyde  = min(fri_hoyde);
skjort_klaring = skjort_bunn;
hylse_klaring  = hette_topp_under - hylse_dybde - mutter_topp;
overlapp       = foring_hoyde - skjort_bunn;
mutter_margin  = (hette_d - 2 * hette_vegg - mutter_d) / 2;

echo(str("Helning: ", vinkel, "°   panel-lengde: ", panel_lengde));
echo(str("Fri høyde under knappene: ", fri_hoyde, " (krav ≥ ", knapp_hus_lengde + 2, ")"));
echo(str("Skjørt–panel i hvile: ", skjort_klaring, " (krav ≥ vandring + 1 = ", vandring + 1, ")"));
echo(str("Hylse–mutter i hvile: ", hylse_klaring, " (krav ≥ vandring + 0,5 = ", vandring + 0.5, ")"));
echo(str("Overlapp skjørt/føringsring: ", overlapp, " (krav ≥ 3)"));
echo(str("Klaring skjørt–mutter: ", mutter_margin, " per side"));

assert(lokk_tykkelse <= 3.0, "Lokket er for tykt til at mutteren får tak");
assert(min_fri_hoyde >= knapp_hus_lengde + 2, "For lite fri høyde under en knapp");
assert(hylse_klaring >= vandring + 0.5, "Hylsa treffer mutteren før knappen klikker");
assert(overlapp >= 3, "For lite overlapp mellom hette og føringsring");
assert(mutter_margin >= 1, "Skjørtet går ikke klar av mutteren");
assert(knapp_avstand >= hette_d + 2 * (foring_klaring + foring_vegg) + 2, "Føringsringene overlapper");
assert(led_pos - knapp_pos >= (hette_d / 2 + foring_klaring + foring_vegg) + (led_krage + led_krage_klaring) / 2 + 1,
       "LED-en kolliderer med føringsringen");

// -----------------------------------------------------------------------------
// Hjelpere
// -----------------------------------------------------------------------------
module avrundet(b, d, r) {
    translate([r, r]) offset(r = r) square([b - 2 * r, d - 2 * r]);
}
module avrundet_sentrert(b, d, r) {
    translate([-b / 2, -d / 2]) avrundet(b, d, r);
}
module fotavtrykk() { avrundet(bredde, dybde, hjorneradius); }
module innside() {
    translate([vegg, vegg]) avrundet(bredde - 2 * vegg, dybde - 2 * vegg, max(0.5, hjorneradius - vegg));
}

// Skråskjæring: z' = z + k·y. Gir rom som følger skråningen.
module skraa() { multmatrix([[1, 0, 0, 0], [0, 1, 0, 0], [0, k, 1, 0], [0, 0, 0, 1]]) children(); }

// Alt under planet z = z0 + k·y
module under_plan(z0) {
    skraa() translate([-50, -50, -200]) cube([bredde + 100, dybde + 100, 200 + z0]);
}
// Alt mellom planene z = z0 + k·y og z = z1 + k·y
module mellom_plan(z0, z1) {
    skraa() translate([-50, -50, z0]) cube([bredde + 100, dybde + 100, z1 - z0]);
}

// Lokkets eget koordinatsystem: x = bredde, y = langs panelet fra frontkanten,
// z = ut av toppflaten (0 = toppflaten).
module paa_lokk() { translate([0, 0, hoyde_front]) rotate([vinkel, 0, 0]) children(); }

// -----------------------------------------------------------------------------
// Kassa
// -----------------------------------------------------------------------------
module esp_holder() {
    h = esp_avstand + esp_pcb + 1.5;
    y_start = esp_y0 - 0.3;
    lengde = dybde - vegg - y_start + eps;
    // sideskinner
    translate([esp_x - esp_b / 2 - 0.3 - 2, y_start, bunn_tykkelse - eps]) cube([2, lengde, h]);
    translate([esp_x + esp_b / 2 + 0.3, y_start, bunn_tykkelse - eps]) cube([2, lengde, h]);
    // endestopp foran
    translate([esp_x - esp_b / 2 - 0.3 - 2, y_start - 2, bunn_tykkelse - eps]) cube([esp_b + 0.6 + 4, 2, h]);
    // tverrlister kortet hviler på (foran og bak)
    for (y = [esp_y0, dybde - vegg - 1.2])
        translate([esp_x - esp_b / 2, y, bunn_tykkelse - eps]) cube([esp_b, 1.2, esp_avstand + eps]);
}

module stripboard_tapper() {
    for (p = [[3, 3], [35 - 3, 25 - 3]])
        translate([stripboard_pos[0] + p[0], stripboard_pos[1] + p[1], bunn_tykkelse - eps])
            difference() {
                cylinder(d = 5, h = 4 + eps);
                cylinder(d = 1.8, h = 10);
            }
}

module vektlomme() {
    translate([vektlomme_pos[0], vektlomme_pos[1], bunn_tykkelse - eps])
        difference() {
            cube([vektlomme_mal[0], vektlomme_mal[1], vektlomme_mal[2] + eps]);
            translate([1.6, 1.6, -1]) cube([vektlomme_mal[0] - 3.2, vektlomme_mal[1] - 3.2, vektlomme_mal[2] + 2]);
        }
}

module usb_hull() {
    translate([esp_x, dybde - vegg - 1, usb_z]) rotate([-90, 0, 0])
        linear_extrude(vegg + 2) avrundet_sentrert(usb_hull_b, usb_hull_h, 2);
}

module kasse() {
    difference() {
        intersection() {
            union() {
                difference() {
                    linear_extrude(hoyde_bak + 1) fotavtrykk();
                    translate([0, 0, bunn_tykkelse]) linear_extrude(hoyde_bak + 2) innside();
                }
                for (p = stolper) translate([p[0], p[1], 0]) cylinder(d = stolpe_d, h = hoyde_bak);
                esp_holder();
                if (med_stripboard) stripboard_tapper();
                if (med_vektlomme) vektlomme();
            }
            under_plan(hoyde_front - lokk_v);
        }
        for (p = stolper) translate([p[0], p[1], bunn_tykkelse + 3]) cylinder(d = skrue_hull_stolpe, h = hoyde_bak);
        usb_hull();
        for (p = fotter) translate([p[0], p[1], -eps]) cylinder(d = 10.5, h = 1 + eps);
    }
}

// -----------------------------------------------------------------------------
// Lokket
// -----------------------------------------------------------------------------
module foringsring() {
    difference() {
        cylinder(d = hette_d + 2 * (foring_klaring + foring_vegg), h = foring_hoyde);
        translate([0, 0, -1]) cylinder(d = hette_d + 2 * foring_klaring, h = foring_hoyde + 2);
    }
}

// Hull for LED-en: rundt hull med smale ribber som klemmes litt sammen når LED-en
// trykkes inn. Det gir stram og sentrert passform selv om printeren bommer litt.
module led_hull_form(hull_d, ribbe_d, h) {
    difference() {
        cylinder(d = hull_d, h = h);
        if (ribbe_d > 0 && ribbe_d < hull_d)
            for (i = [0:led_ribber - 1]) rotate(i * 360 / led_ribber)
                translate([ribbe_d / 2, -0.4, -1]) cube([(hull_d - ribbe_d) / 2 + 0.5, 0.8, h + 2]);
    }
}

// Forsenkning for LED-kragen, med den flate siden, så LED-en ikke kan vris og minus alltid peker samme vei.
module krage_form() {
    r = (led_krage + led_krage_klaring) / 2;
    f = max(led_krage_flat - led_krage / 2, led_kropp / 2) + led_krage_klaring / 2;
    rotate(led_flat_retning) intersection() {
        circle(r = r);
        translate([-r - 1, -r - 1]) square([r + 1 + f, 2 * r + 2]);
    }
}

module lokk() {
    difference() {
        union() {
            intersection() {
                linear_extrude(hoyde_bak + 10) fotavtrykk();
                mellom_plan(hoyde_front - lokk_v, hoyde_front);
            }
            paa_lokk() for (x = knapp_x) translate([x, knapp_pos, -eps]) foringsring();
        }
        paa_lokk() for (i = [0:3]) {
            x = knapp_x[i];
            // knapp
            translate([x, knapp_pos, -lokk_tykkelse - 1]) cylinder(d = knapp_hull, h = lokk_tykkelse + 2);
            // LED + forsenkning for kragen på undersiden
            translate([x, led_pos, -lokk_tykkelse - 1]) led_hull_form(led_hull, led_ribbe_d, lokk_tykkelse + 2);
            translate([x, led_pos, -lokk_tykkelse - 1]) linear_extrude(1 + led_krage_dybde) krage_form();
            // etikettfelt
            translate([x - etikett_b / 2, etikett_pos - etikett_h / 2, -etikett_dybde]) cube([etikett_b, etikett_h, 1]);
            // nummer
            translate([x + etikett_b / 2 - 1.2, etikett_pos - etikett_h / 2 + 1.2, -etikett_dybde - nummer_dybde])
                linear_extrude(nummer_dybde + eps)
                    text(str(i + 1), size = nummer_storrelse, halign = "right", valign = "bottom",
                         font = "Liberation Sans:style=Bold");
        }
        for (p = stolper) translate([p[0], p[1], 0]) cylinder(d = skrue_hull_lokk, h = hoyde_bak + 10);
    }
}

// Lokket lagt flatt med toppen opp, klart for printing
module lokk_flat() {
    translate([0, 0, lokk_tykkelse]) rotate([-vinkel, 0, 0]) translate([0, 0, -hoyde_front]) lokk();
}

// -----------------------------------------------------------------------------
// Hetten (i hvilestilling: z = 0 er panelets toppflate)
// -----------------------------------------------------------------------------
module hette() {
    difference() {
        union() {
            difference() {
                union() {
                    translate([0, 0, skjort_bunn])
                        cylinder(d = hette_d, h = hette_toppflate - skjort_bunn - hette_fas);
                    translate([0, 0, hette_toppflate - hette_fas])
                        cylinder(d1 = hette_d, d2 = hette_d - 2 * hette_fas, h = hette_fas);
                }
                translate([0, 0, skjort_bunn - eps])
                    cylinder(d = hette_d - 2 * hette_vegg, h = hette_topp_under - skjort_bunn + eps);
            }
            translate([0, 0, hette_topp_under - hylse_dybde])
                cylinder(d = hylse_indre + 2 * hylse_vegg, h = hylse_dybde + eps);
        }
        translate([0, 0, hette_topp_under - hylse_dybde - eps]) cylinder(d = hylse_indre, h = hylse_dybde + eps);
        if (hette_grop > 0) {
            r = (hette_d - 2 * hette_vegg) / 2;
            R = (r * r + hette_grop * hette_grop) / (2 * hette_grop);
            translate([0, 0, hette_toppflate + R - hette_grop]) sphere(r = R, $fn = 160);
        }
    }
}

module hette_print() { translate([0, 0, hette_toppflate]) rotate([180, 0, 0]) hette(); }

// -----------------------------------------------------------------------------
// Testbiter
// -----------------------------------------------------------------------------
module hette_test() {
    difference() {
        union() {
            translate([-20, -20, 0]) cube([40, 40, lokk_tykkelse]);
            translate([0, 0, lokk_tykkelse - eps]) foringsring();
        }
        translate([0, 0, -1]) cylinder(d = knapp_hull, h = lokk_tykkelse + 2);
    }
}

// Fire varianter: [hull, ribbetupper, etikett]. Velg den som sitter best og sett
// led_hull / led_ribbe_d til de verdiene. Kragen ligger i forsenkningen på undersiden.
led_test_varianter = [[10.0, 0, "10.0"], [10.1, 0, "10.1"], [10.4, 10.0, "R10.0"], [10.4, 9.8, "R9.8"]];

module led_test() {
    n = len(led_test_varianter);
    difference() {
        cube([6 + n * 16, 22, lokk_tykkelse]);
        for (i = [0:n - 1]) {
            v = led_test_varianter[i];
            translate([11 + i * 16, 12, -1]) led_hull_form(v[0], v[1], lokk_tykkelse + 2);
            translate([11 + i * 16, 12, -eps]) linear_extrude(led_krage_dybde + eps) krage_form();
            translate([11 + i * 16, 1.0, lokk_tykkelse - 0.5])
                linear_extrude(1) text(v[2], size = 2.6, halign = "center", font = "Liberation Sans:style=Bold");
        }
    }
}

// -----------------------------------------------------------------------------
// Samlet visning med dummy-komponenter
// -----------------------------------------------------------------------------
module dummy_led(farge) {
    // kragen ligger i forsenkningen på undersiden, kuppelen stikker opp gjennom panelet
    z0 = -lokk_tykkelse + led_krage_dybde - led_krage_tykk;
    color(farge, 0.7) {
        translate([0, 0, z0]) cylinder(d = led_krage, h = led_krage_tykk);
        translate([0, 0, z0]) cylinder(d = 10, h = led_hoyde - 5);
        translate([0, 0, z0 + led_hoyde - 5]) sphere(d = 10);
    }
    color("silver") for (s = [-1, 1]) translate([s * 1.27 - 0.25, -0.25, z0 - 20]) cube([0.5, 0.5, 20]);
}

module dummy_knapp() {
    color("dimgray") translate([0, 0, -lokk_tykkelse - knapp_hus_lengde]) cylinder(d = knapp_hus_d, h = knapp_hus_lengde);
    color("silver") cylinder(d = mutter_d, h = mutter_topp, $fn = 6);
    color("royalblue") cylinder(d = blaa_hette_d, h = blaa_hette_hoyde);
}

module samlet() {
    color("gainsboro") kasse();
    color("whitesmoke") lokk();
    paa_lokk() for (i = [0:3]) {
        translate([knapp_x[i], knapp_pos, 0]) {
            dummy_knapp();
            color(farger[i]) hette();
        }
        translate([knapp_x[i], led_pos, 0]) dummy_led(farger[i]);
    }
    // ESP32-C3 SuperMini
    color("darkgreen") translate([esp_x - esp_b / 2, esp_y0, bunn_tykkelse + esp_avstand]) cube([esp_b, esp_l, esp_pcb]);
    color("silver") translate([esp_x - 4.5, dybde - vegg - 7.5 + 1, bunn_tykkelse + esp_avstand + esp_pcb]) cube([9, 7.5, 3.2]);
}

module print_plate() {
    kasse();
    translate([0, dybde + 15, 0]) lokk_flat();
    for (i = [0:3]) translate([bredde + 15 + hette_d / 2, hette_d / 2 + i * (hette_d + 6), 0]) hette_print();
}

// -----------------------------------------------------------------------------
if (del == "samlet")          samlet();
else if (del == "kasse")      kasse();
else if (del == "lokk")       lokk_flat();
else if (del == "hette")      hette_print();
else if (del == "print")      print_plate();
else if (del == "hette_test") hette_test();
else if (del == "led_test")   led_test();
