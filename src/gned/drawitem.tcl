#==========================================================================
#  DRAWITEM.TCL -
#            part of the GNED, the Tcl/Tk graphical topology editor of
#                            OMNeT++
#
#   By Andras Varga
#
#==========================================================================

#----------------------------------------------------------------#
#  Copyright (C) 1992,99 Andras Varga
#  Technical University of Budapest, Dept. of Telecommunications,
#  Stoczek u.2, H-1111 Budapest, Hungary.
#
#  This file is distributed WITHOUT ANY WARRANTY. See the file
#  `license' for details on this and other legal matters.
#----------------------------------------------------------------#

#
# On the Tk canvas, canvas item tags are used to identify what is what.
# Following tags are used (persistent):
#   key-*        - key in the ned() array
#   module       - main rectange (in fact, 2 rects) of enclosing module
#   background   - item cannot be dragged/right-clicked: background,label etc
#   submod       - rectangle or icon of submodule
#   conn         - connection arrow
# Other tags are also used, but only for temporary purposes.
#

# default module icon. Used if specified icon is not found
image create bitmap defaulticon -data {
   #define box2_width 32
   #define box2_height 32
   static unsigned char box2_bits[] = {
    0xf8, 0xff, 0xff, 0x1f, 0xfc, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0x7f,
    0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0x00, 0xf0, 0x0f, 0x00, 0x00, 0xf0,
    0xcf, 0xff, 0xff, 0xf3, 0xcf, 0xff, 0xff, 0xf3, 0xcf, 0x00, 0x00, 0xf3,
    0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3,
    0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3,
    0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3,
    0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3,
    0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3, 0xcf, 0x00, 0x00, 0xf3,
    0xcf, 0xff, 0xff, 0xf3, 0xcf, 0xff, 0xff, 0xf3, 0x0f, 0x00, 0x00, 0xf0,
    0x0f, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xff, 0xff, 0x7f,
    0xfc, 0xff, 0xff, 0x3f, 0xf8, 0xff, 0xff, 0x1f};
}


# drawItemRecursive --
#
# draw one item, including recursively
# (currently only compound modules need recursive treatment)
#
proc drawItemRecursive {key {canv_id {}}} {
    global ned

    if {$ned($key,type)!="module"} {

        # not a compound module
        #
        drawItem $key $canv_id
        return

    } else {
        # now handle compound module
        #

        # first the submodules...
        set sectionkey [getChildrenWithType $key submods]
        if {$sectionkey!=""} {
            foreach smkey [getChildren $sectionkey] {
                drawItem $smkey $canv_id
            }
        }

        # ... then the enclosing module (its size and position may
        #     depend on submodule placement)
        drawItem $key $canv_id

        # ...then the connections, normal and loop
        set sectionkey [getChildrenWithType $key conns]
        if {$sectionkey!=""} {
            foreach connkey [getChildren $sectionkey] {
                if {$ned($connkey,type)=="forloop"} {
                    foreach nconnkey [getChildrenWithType $connkey conn] {
                        drawItem $nconnkey $canv_id
                    }
                } else {
                    drawItem $connkey $canv_id
                }
            }
        }
    }
}

# drawItems --
#
#  draw items passed in the keys list. Used when pasting the clipboard contents.
#
proc drawItems {keys {canv_id {}}} {
    global ned

    # first module and submodules...
    foreach key $keys {
       if {$ned($key,type)=="module" || $ned($key,type)=="submod"} {
          drawItem $key $canv_id
       }
    }

    # ...then the connections
    foreach key $keys {
       if {$ned($key,type)=="forloop"} {
           foreach nconnkey [getChildrenWithType $key conn] {
               drawItem $nconnkey $canv_id
           }
       } elseif {$ned($key,type)=="conn"} {
           drawItem $connkey $canv_id
       }
    }
}

# drawItem --
#
#   Draw item given by key on the canvas.
#   NOT recursive.
#
proc drawItem {key {canv_id {}}} {
    global ned gned canvas

    if {$canv_id==""} {set canv_id $gned(canvas_id)}
    set c $canvas($canv_id,canvas)

    if {$ned($key,type)=="module"} {
        draw_module $c $key
    } elseif {$ned($key,type)=="submod"} {
        draw_submod $c $key
    } elseif {$ned($key,type)=="conn"} {
        draw_connection $c $key
    } else {
        error "drawItem: item $key is no module, submod or conn"
    }

    if $ned($key,aux-isselected) {
        selectItem $key
    }
}

# redrawItem --
#
#   Redraw item given by key on the canvas
#
proc redrawItem {key {canv_id {}}} {
    global ned gned canvas

    if {$canv_id==""} {set canv_id $gned(canvas_id)}
    set c $canvas($canv_id,canvas)

    # delete from canvas
    foreach i [array names ned "$key,*-cid"] {
       $c delete $ned($i)
       set ned($i) ""
    }

    # redraw
    drawItem $key $canv_id
}

# selectItem --
#
#   Display item as selected
#
proc selectItem {key} {
    global ned canvas

    set ned($key,aux-isselected) 1

    set canv_id [canvasIdFromItemKey $key]
    set c $canvas($canv_id,canvas)

    if {$ned($key,type)=="module"} {
        $c itemconfig $ned($key,rect2-cid) -outline red
        $c itemconfig $ned($key,label-cid) -fill red
    } elseif {$ned($key,type)=="submod"} {
        $c itemconfig $ned($key,rect-cid) -outline red
        $c itemconfig $ned($key,label-cid) -fill red
    } elseif {$ned($key,type)=="conn"} {
        $c itemconfig $ned($key,arrow-cid) -fill red
    } else {
        error "item $key is neither module, submod or conn"
    }
}

# deselectItem --
#
#   Display item as deselected
#
proc deselectItem {key} {
    global ned canvas

    set ned($key,aux-isselected) 0

    set canv_id [canvasIdFromItemKey $key]
    set c $canvas($canv_id,canvas)

    if {$ned($key,type)=="module"} {
        set outline   [_getPar {} "$key,disp-outlinecolor" #000000]
        $c itemconfig $ned($key,rect2-cid) -outline $outline
        $c itemconfig $ned($key,label-cid) -fill #000000
    } elseif {$ned($key,type)=="submod"} {
        set outline   [_getPar {} "$key,disp-outlinecolor" #000000]
        if {$ned($key,icon-cid)!=""} {
           $c itemconfig $ned($key,rect-cid) -outline ""
        } else {
           $c itemconfig $ned($key,rect-cid) -outline $outline
        }
        $c itemconfig $ned($key,label-cid) -fill #000000
    } elseif {$ned($key,type)=="conn"} {
        set fill      [_getPar {} "$key,disp-fillcolor" #000000]
        $c itemconfig $ned($key,arrow-cid) -fill $fill
    } else {
        error "item $key is neither module, submod or conn"
    }
}

# deselectAllItems --
#
#   Display all items as deselected
#
proc deselectAllItems {} {
    global ned

    foreach i [array names ned "*,aux-isselected"] {
       if {$ned($i)} {
          regsub -- ",aux-isselected" $i "" key
          deselectItem $key
       }
    }
}

# selectedItems --
#
#   Returns keys of all selected items
#
proc selectedItems {} {
    global ned
    set keys {}

    foreach i [array names ned "*,aux-isselected"] {
       if {$ned($i)} {
          regsub -- ",aux-isselected" $i "" key
          lappend keys $key
       }
    }
    return $keys
}

#-------------------------------------------------
#
# individual draw procedures for all item types
#
#-------------------------------------------------

# _getPar: internal to draw_XXX
proc _getPar {arg ned_index default} {
    global ned

    if {$arg!=""} {
        return $arg
    } elseif {[info exist ned($ned_index)] && $ned($ned_index)!=""} {
        return $ned($ned_index)
    } else {
        return $default
    }
}

# draw_module: internal to drawItem
proc draw_module {c key} {
    global ned

    # top-left corner
    set x1 $ned($key,disp-xpos)
    set y1 $ned($key,disp-ypos)
    if {$x1==""} {set x1 10}
    if {$y1==""} {set y1 10}

    # size
    set sx $ned($key,disp-xsize)
    set sy $ned($key,disp-ysize)

    set bb [$c bbox submod]
    if {$sx==""} {
        if {$bb==""} {
            set sx 300
        } else {
            set sx [expr [lindex $bb 2]+[lindex $bb 0]-2*$x1]
        }
    }
    if {$sy==""} {
        if {$bb==""} {
            set sy 200
        } else {
            set sy [expr [lindex $bb 3]+[lindex $bb 1]-2*$y1]
        }
    }

    # right-bottom corner
    set x2 [expr $x1+$sx]
    set y2 [expr $y1+$sy]

    # now: draw it!
    set fill    [_getPar {} "$key,disp-fillcolor" #c0c0c0]
    set outline [_getPar {} "$key,disp-outlinecolor" #000000]
    set thickness [_getPar {} "$key,disp-linethickness" 2]

    set bg  [$c create rect $x1 $y1 $x2 $y2 -outline ""  -fill $fill]
    set r1  [$c create rect [expr $x1+6] [expr $y1+6] [expr $x2-6] [expr $y2-6] -width 12 -outline $fill -fill ""]
    set r2  [$c create rect $x1 $y1 $x2 $y2 -width $thickness -outline $outline -fill ""]
    set lbl [$c create text [expr $x1+3] [expr $y1+3]  -anchor nw]

    $c lower $lbl
    $c lower $r2
    $c lower $r1
    $c lower $bg

    # tags, cids etc.
    set ned($key,background-cid) $bg
    set ned($key,rect-cid)       $r1
    set ned($key,rect2-cid)      $r2
    set ned($key,label-cid)      $lbl

    $c addtag "key-$key" withtag $bg
    $c addtag "key-$key" withtag $r1
    $c addtag "key-$key" withtag $r2
    $c addtag "key-$key" withtag $lbl

    $c addtag "background" withtag $bg
    $c addtag "module"     withtag $r1
    $c addtag "module"     withtag $r2
    $c addtag "module"     withtag $lbl

    $c itemconfigure $lbl -text $ned($key,name)
}

# draw_submod: internal to drawItem
proc draw_submod {c key} {
    global ned

    set icon  [_getPar {} "$key,disp-icon" ""]

    set x  [_getPar {} "$key,disp-xpos" [expr 100+100*rand()]]
    set y  [_getPar {} "$key,disp-ypos" [expr 100+100*rand()]]

    if {$icon==""} {
        set sx [_getPar {} "$key,disp-xsize" 40]
        set sy [_getPar {} "$key,disp-ysize" 24]

        set x1 [expr $x-$sx/2]
        set y1 [expr $y-$sy/2]
        set x2 [expr $x+$sx/2]
        set y2 [expr $y+$sy/2]

        set fill      [_getPar {} "$key,disp-fillcolor" #8080ff]
        set outline   [_getPar {} "$key,disp-outlinecolor" #000000]
        set thickness [_getPar {} "$key,disp-linethickness" 2]

        set r   [$c create rect $x1 $y1 $x2 $y2 -width $thickness -outline $outline -fill $fill]
        set lbl [$c create text $x $y2 -anchor n]
        set ic ""

    } else {

        # check if icon exists
        if [catch {image type $icon}] {
            tk_messageBox -type ok -title Warning -icon error \
                 -message "Warning: icon '$icon' for submodule '$ned($key,name)' not found."
            set icon "defaulticon"
        }

        set sx [expr [image width $icon]+4]
        set sy [expr [image height $icon]+4]

        set x1 [expr $x-$sx/2]
        set y1 [expr $y-$sy/2]
        set x2 [expr $x+$sx/2]
        set y2 [expr $y+$sy/2]

        set r   [$c create rect $x1 $y1 $x2 $y2 -outline "" -fill ""]
        set lbl [$c create text $x $y2 -anchor n]
        set ic  [$c create image $x $y -image $icon]
    }

    set ned($key,rect-cid)   $r
    set ned($key,label-cid)  $lbl
    set ned($key,icon-cid)   $ic

    $c addtag "submod" withtag $r
    $c addtag "submod" withtag $lbl
    $c addtag "submod" withtag $ic

    $c addtag "key-$key" withtag $r
    $c addtag "key-$key" withtag $lbl
    $c addtag "key-$key" withtag $ic

    if {$ned($key,vectorsize)!=""} {
        set txt "$ned($key,name)\[$ned($key,vectorsize)\]"
    } else {
        set txt $ned($key,name)
    }
    $c itemconfigure $lbl -text $txt
}

# draw_connection: internal to drawItem
proc draw_connection {c key} {
    global ned

    set mode  [_getPar {} "$key,disp-drawmode" "auto"]

    set src_x [_getPar {} "$key,disp-src-anchor-x" 50]
    set src_y [_getPar {} "$key,disp-src-anchor-y" 50]
    set dest_x [_getPar {} "$key,disp-dest-anchor-x" 50]
    set dest_y [_getPar {} "$key,disp-dest-anchor-y" 50]

    set fill      [_getPar {} "$key,disp-fillcolor" #000000]
    set thickness [_getPar {} "$key,disp-linethickness" 1]

    set s_coords [_getCoords $c $ned($ned($key,src-ownerkey),rect-cid)]
    set d_coords [_getCoords $c $ned($ned($key,dest-ownerkey),rect-cid)]

    set a_coords [eval opp_arrowcoords $s_coords $d_coords  0 1 0 1 \
                                       $mode $src_x $src_y $dest_x $dest_y]
    set cid [eval $c create line $a_coords -arrow last -width $thickness -fill $fill]

    set ned($key,arrow-cid) $cid

    $c addtag "key-$key" withtag $cid
    $c addtag "conn" withtag $cid
}



