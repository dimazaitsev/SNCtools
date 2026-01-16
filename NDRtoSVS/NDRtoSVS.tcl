namespace eval NDRtoSVS {

    proc controls {} {
	radiobox NDRtoSVS::fmt "output format" ".lsn/.hsn .h .h-with-pin .h-SVS" "-l -c -i -v" -v
    }

    proc command {} {
    
	return "NDRtoSVS $NDRtoSVS::fmt"
    }

}


