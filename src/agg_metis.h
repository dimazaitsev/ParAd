// ParAd: Parallel Adriana 
// agg_metis.h                              
// Aggregate clans via METIS

int AggregateClansMETIS( char * NetFileName, int nz, int np, int k, int debug );

/* INPUT: NetFileName - PN in SPM format */
//        nn - number of partitions
/*         NetFileName.z - contact places info: <place> <input_FSN> <outputFSN> */
/*         NetFileName.p - graph of decomposition */
/*         NetFileName.z#i - FSN in SPM format */
/* FUNCTION: aggregates clans into np partitions, if np==0, into optimal number of partitions */
/* OUTPUT: files of aggregated clans: */
/*         NetFileName.z - contact places info: <place> <input_FSN> <outputFSN> */
/*         NetFileName.p - graph of decomposition */
/*         NetFileName.z#i - FSN in SPM format */

// @ 2019 Dmitry Zaitsev: daze@acm.org

