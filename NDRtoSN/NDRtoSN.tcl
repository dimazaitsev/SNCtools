namespace eval NDRtoSN {

    proc controls {} {
	radiobox NDRtoSN::format "output format" ".lsn/.hsn .h .h-with-pin" "-l -c -i" -c
    }

    proc command {} {
    
	return "NDRtoSN $NDRtoSN::format"
    }

}


