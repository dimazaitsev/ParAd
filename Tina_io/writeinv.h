// ParAd: Parallel Adriana 
// writeinv.h          
// Write invariants (given in SPM format) into text file according to table of names

int WriteInv( char * TableFileName, char * InvFileName, char * OutFileName, int verbose );

/* INPUT:  TableFileName - table of nodes names */
/*         InvFileName - basis invariants in SPM format */
/* OUTPUT: OutFileName - basis invariants in text form */
/* FUNCTION: transforms invariants from SPM format to text form according to table of names */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

