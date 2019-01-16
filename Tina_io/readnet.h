// ParAd: Parallel Adriana 
// readnet.h         
// Reads .net or .ndr file and write it into sparse matrix and names' tables

int ReadNDRNET( char * NetFileName, char * SPMFileName, char * TxtFileName, int verbose );

/* INPUT: NetFileName - PN in .ndr/.net format */
/* OUTPUT: NetFileName.spm - nat in SPM format */
/*         NetFileName.nmp - table of places names */
/*         NetFileName.nmt - table of transitions names */
/* FUNCTION: transforms PN into SPM format and store tables of names */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

