
#ifndef _EXTERNAL_LIBRARIES_H_
#include "extlib.h"
#endif//_EXTERNAL_LIBRARIES_H_

#include <regex.h>

/*

    entire_regular_expression   := ( basic_regular_expression | extended_regular_expression ) +

    escape_sequence             := escape_character single_character

    escape_character: <backslash> that is neither in a bracket expression or itself escaped

    1. Matching is based on bit pattern used for encoding the character -> streaming pure bytes.
    2. Search starts at the beggining of a string.
    3. Match is the longest of the leftmost matches.
    4. Null string is longer that no match at all.
    5. Length of the match is measured in bytes.
    6. Invalid REs generate an error condition.
    7. '\0' is not permitted in REs or string to be matched, and are treated as the end of the string.
    8. Must support at least 256 byte REs.

Mamy trzy konteksty:

    1. Bracket Expression
    2. Basic Regular Expression
    3. Extended Regular Expression

-------------------


    bracket_expression	:=  "["	"^"? "]"? ( ordinary_character | collating_element | collating_symbol | equivalence_class | character_class | range_expression )+ "]"

    
9.3.1:	bre_single_element_match    	:= bre_orignary_character | bre_special_character | "]" | "." | bracket_expression

9.4.1:	ere_single_element_match	:= ere_orignary_character | ere_special_character | "]"	| "}" | "." | bracket_expression 
				 
    
  
*/
