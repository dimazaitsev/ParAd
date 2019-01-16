// ParAd: Parallel Adriana 
// mui.h           
// Composes joint matrix of basis invariants

void ComposeJointMatrix( char * NetFileName );

/* INPUT: FileName.z - contact places of decomposition */
/*        FileName.i#i  - basis invariants of FSN */
/* OUTPUT: FileName.g - joint matrix of FSN basis invariants */
/* FUNCTION: add unit solutions to solutions of composition system */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

