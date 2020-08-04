package require Tk
package require zint
wm title . "ZINT Demo"
wm minsize . 480 320
if {[info command sdltk] eq "sdltk"} {
    wm attributes . -fullscreen 1
    set padx 30
    set pady 20
} else {
    wm geometry . 480x320
    set padx 5
    set pady 5
}
image create photo ::zintimg
pack [::ttk::combobox .c -values [lsort [zint symbologies]] -state readonly] \
    -side top -fill x -padx $padx -pady $pady
.c set Datamatrix
pack [::ttk::entry .e] -side top -fill x -padx $padx -pady $pady
.e insert end 12345
bind .e <Return> Generate
pack [::ttk::entry .o] -side top -fill x -padx $padx -pady $pady
.o insert end "-bold 1"
bind .o <Return> Generate
pack [::ttk::button .b -text Generate -command Generate] -fill x -side top \
    -padx $padx -pady $pady
proc Generate {} {
    ::zintimg blank
    ::zintimg configure -width 1 -height 1
    ::zintimg blank
    ::zintimg configure -width 0 -height 0
    if {[catch {zint encode [.e get] ::zintimg -barcode [.c get] {*}[.o get]} e]} {
        tk_messageBox -message $e -title "Zint error"
    } else {
	set w [image width ::zintimg]
	set h [image height ::zintimg]
	set lw [winfo width .l]
	set lh [winfo height .l]
	set sx [expr {int(1.0 * $lw / $w)}]
	set sy [expr {int(1.0 * $lh / $h)}]
	if {$sy < $sx} {
	    set sx $sy
	}
	if {$sx <= 0} {
	    set sx [expr {1.1 * $lw / $w}]
	    set sy [expr {1.1 * $lh / $h}]
	    if {$sy < $sx} {
		set sx $sy
	    }
	}
	::zintimg blank
	::zintimg configure -width 1 -height 1
	::zintimg blank
	::zintimg configure -width 0 -height 0
	catch {
	    zint encode [.e get] ::zintimg -barcode [.c get] -scale $sx {*}[.o get]
	}
    }
}
pack [label .l -image ::zintimg -bg white] -side top -fill both -expand 1 \
    -padx $padx -pady $pady
bind .e <Configure> {
    after cancel Generate
    after idle Generate
}
bind .c <<ComboboxSelected>> {
    after cancel Generate
    after idle Generate
}
bind all <Break> exit
bind all <Control-q> exit
bind all <Alt-q> exit
