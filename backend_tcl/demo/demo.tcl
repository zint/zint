package require Tk
lappend auto_path [file dirname [info script]]/../lib
package require zint
image create photo ::zintimg
pack [label .l -image ::zintimg] -side left
pack [ttk::separator .s -orient vertical] -side left
pack [::ttk::combobox .c -values [lsort [zint symbologies]] -state readonly]\
        -side top -fill x
.c set Datamatrix
pack [::ttk::entry .e] -side top -fill x
.e insert end 12345
bind .e <Return> Generate
pack [::ttk::button .b -text Generate -command Generate] -fill x -side top
proc Generate {} {
    ::zintimg blank
    if {[catch {zint encode [.e get] ::zintimg -barcode [.c get]} e]} {
        tk_messageBox -message $e -title "Zint error"
    }
}
Generate
