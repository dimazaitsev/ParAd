// ParAd: Parallel Adriana 
// cor1.h                            
// Collapse of a weighted graph

struct edge {
  int v1;
  int v2;
  int w;
};


int CollapseOfGraph( char ty, char * GraphFileName, char * TraceFileName );

int ReadGraph( char * GraphFileName, struct edge **ae, int *nn );
int ChooseEdge(char ty, struct edge *e, int *n, struct edge *pe, int *pn);
int CollapseEdge(struct edge *ce, struct edge *e, int *n, struct edge *pe, int *pn);

/* INPUT: ty - type of collapse: 'f' - first max; 'm' - random max; 'r' - random; 'i' - last min */
/*        GraphFileName - weighted graph given by edges: <node1> <node2> <weight> */
/* OUTPUT: TraceFileName - sequence (trace) of collapse */
/* FUNCTION: collapse weighted graph acoording to a given rule of edge choice */

// @ 2018 Dmitry Zaitsev, Stanimire Tomov, and Jack Dongarra: daze@acm.org

