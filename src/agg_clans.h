// ParAd: Parallel Adriana 
// agg_clans.h                              
// Aggregate clans

int AggregateClans( char * NetFileName, int nz, int np, double ap, int debug );

/* INPUT: NetFileName - PN in SPM format */
//        ap - parameter of aggregation, >=0, 0 - no aggregation
//         NetFileName.d - digest of decomposition
/*         NetFileName.z - contact places info: <place> <input_FSN> <outputFSN> */
/*         NetFileName.p - graph of decomposition */
/*         NetFileName.z#i - FSN in SPM format */
/* FUNCTION: aggregates clans to not exceed threshold of ap*max_c, max_c - maximal clan size */
/* OUTPUT: files of aggregated clans: */
/*         NetFileName.z - contact places info: <place> <input_FSN> <outputFSN> */
/*         NetFileName.p - graph of decomposition */
/*         NetFileName.z#i - FSN in SPM format */

// @ 2019 Dmitry Zaitsev: daze@acm.org

