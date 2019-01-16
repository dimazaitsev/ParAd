// ParAd: Parallel Adriana 
// fic.h                            
// Filters contact places for fusion of clans

int FilterContactPlaces( char * NetFileName, int l );

int psFilterContactPlaces( char * NetFileName, int v1, int v2 );

/* INPUT: NetFileName.y - set of contact places */
/*        NetFileName.t - trace of collapse */
/*        l - number of collapse step */
/* OUTPUT: NetFileName.z - set of contact places for step of collapse */
/*         NetFileName.y - new set of contact places after fusion */
/* FUNCTION: create set of contact places for a given step of collapse and filter contact places */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

