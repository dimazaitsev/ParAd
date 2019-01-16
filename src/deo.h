// ParAd: Parallel Adriana 
// deo1.h                              
// Petri Net Decomposition into clans in SPM format
// version for decomposition-based solution of equations + parallel-sequential

int DecomposeIntoFSN( char * NetFileName, int debug );

/* INPUT: NetFileName - PN in SPM format */
/* OUTPUT: NetFileName.z - contact places info: <place> <input_FSN> <outputFSN> */
/*         NetFileName.p - graph of decomposition */
/*         NetFileName.z#i - FSN in SPM format */
/* FUNCTION: decomposes PN into its minimal FSN */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

