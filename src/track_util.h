/*
 * track_util.h
 *
 *  Created on: 17.02.2013
 *      Author: 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cmath>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#ifndef errno
#include <errno.h>
#endif

#ifndef TRACK_UTIL_H_
#define TRACK_UTIL_H_

extern const int progType;	//type of the program
const int SG=1;
const int PRJ=2;
const int CNF=4;

const  int DEBUG_PRINT=1;
const  int DEBUG_LOG=2;

#define PRM_EXT "prm"
#define BPROF_EXT "bprof"
#define AC_EXT ".acf"
#define BGR_EXT "bgraph"
#define DERIV '_'

#define BINVAL short

const  BINVAL NA=0x8080;
const  BINVAL MAX_SHORT=32000;
const  double PI=3.14159265358979323846;
const int LRAND_MAX=(RAND_MAX > 0XFFFFF)?RAND_MAX : 0x3fffffff;
extern const char* version;
const  int BED_TRACK=1;
const  int BED_GRAPH=2;
const  int WIG_TRACK=3;
const  int BROAD_PEAK=4;
const  int MODEL_TRACK=8;
const  int CAGE_MIN=16;

const  int BP_SCORE=0;
const  int BP_SIGNAL=1;
const  int BP_LOGPVAL=2;
const  int NONE=0;

const  int XML=1;
const  int TAB=2;

const  int GENE=4;
const  int IVS=0x10;
const  int EXON=0x20;
const  int GENE_BEG=0x41;
const  int GENE_END=0x42;
const  int IVS_BEG=0x11;
const  int IVS_END=0x12;
const  int EXON_BEG=0x21;
const  int EXON_END=0x22;

const  int KERN_NORM=1;
const  int KERN_LEFT_EXP =2;
const  int KERN_RIGHT_EXP=3;
const  int KERN_CUSTOM=0x80;

const  int COLLINEAR=1;
const  int COMPLEMENT=2;
const  int IGNORE_STRAND=COLLINEAR|COMPLEMENT;
const  int MAX_SCORE=1;
const  int AV_SCORE=2;

const  int LOG_SCALE =1;
const  int LOG_LOG_SCALE =2;
const  int LIN_SCALE =0;
const  int AUTO_SCALE=2;

const  int WIG_BASE=1;
const  int WIG_CENTER=2;
const  int WIG_SUM=0x10;
const  int WIG_MULT=0x20;
extern int binBufSize;


const int MAX_GENES=100000;

//============================================ Memory operations
void *xmalloc(size_t n, const char * err);
void zfree(void *a, const char* b);
//============================================ MACROS
//#define getMem0(a,n,err) {if(a==0) a=(typeof a)xmalloc((n+100)*sizeof(*a),err);}
#define getMem(a,n,err)  {a=(typeof a)xmalloc((n+100)*sizeof(*a),err);}
#define xfree(a,b) 		 {zfree(a,b); a=0;}
#define zeroMem(a,n) 	 {memset(a,0,n*sizeof(*a));}
#define xmemcpy(dest,pds, src, psrc, n) {memcpy(dest+(pds)*sizeof(*dest), src+(psrc)*sizeof(*src),(n)*sizeof(*src));}
#define max(a,b) a<b ? b : a
#define min(a,b) a>b ? b : a
#define abs(a) (a<0 ? -a : a)
#define sign(a) (a==0 ? 0 : (a<0 ? -1:1))

struct Model;
//========================= Common parameters ================================
extern int  binSize;          // frame size of profile
extern long long GenomeLength;      // TOTAL LENGTH OF THE GENOME
extern char *chromFile;		   // Chromosomes file name
extern char *profile1;		   // first profile file file name
extern char *profile2;		   // second profile file file name
extern char *trackFil;		   // input track
extern char *aliaseFil;		   // aliases
extern char *inputProfiles;    // input formula for track combination
extern int 	bpType;			   // type of the input data in BroadPeak file

extern bool  writeDistr;		// flag: write distributions

extern char trackName[4096];   	// current track name
extern char *profPath;		// path to binary profiles
extern char *cfgFile;		// config file name
extern char *confFile;		// confounder file name
extern char *trackPath;		// path to GBrowse track files
extern char *resPath;		// path to results files (tracks and distributions)
extern char *outFile;		// output filename
extern char *logFileName;	// output filename
extern char *statFileName;	// File name for cummulative statistics
extern char *paramsFileName; // Filename for save parameters of runs
extern unsigned long id;

extern int inputErr;		// flag: if input track has errors
extern int inputErrLine;	// Error line in the input
extern char curFname[4048];	// current input file

extern int inpThreshold;		// Testing of binarized input data, % of max
//============================ Track parameters ===============================
extern int   complFg;		// Flag: IGNORE_STRAND - ignore strand; COLLINEAR - compare collinear strands; COMPLEMENT - compare complement strands
extern int   intervFlag0;	// Flag: for bed tracks take intervals with score 1.
extern bool  NAFlag;		// Flag: 0 -> uncovered_values=0; otherwise uncovered values=NA
extern int   profileLength;	// size of the profile array
//extern float *profile;		// uncompressed profile array
//extern float *profilec;		// uncompressed profile array

extern double scaleFactor;

extern char *pcorProfile;    	// partial correlation profile file name
extern float *outTrackProfile;  // correlation track

//=========================== Correlation parameters ===========================

extern int flankSize;    // size of flank (nucleotides)
extern int LFlankProfSize;        // size of the left flank (profile scale)
extern int RFlankProfSize;        // size of the right flank (profile scale)
extern double noiseLevel; // level of noise
extern int wSize;        // size of widow (nucleotides)
extern int wStep;        // window step   (nucleotides)
extern int wProfSize;    // size of widow (profile scale)
extern int wProfStep;    // window step (profile scale)
extern double kernelSigma;    	// kernel width (nucleotides)
extern double kernelShift;    	// Kernel mean (for Gauss) or Kernel start for exponent
extern double kernelNS;			// Correction for non-specifisity
extern double kernelProfSigma;  // kernel width (profile scale)
extern double kernelProfShift;  // Kernel shift (profile scale)


extern int kernelType;   		// kernel type
extern char* customKern;		// formula for the custom kernel
extern double maxNA0;			// max allowed NA values - if at lest one of profiles contains more than maxNA NA values the window will be ignored
extern double maxZero0;			// max allowed zero values - if both profiles contains more than maxZero zeros the window will be ignored
extern double maxNA;			// max allowed NA values - if at lest one of profiles contains more than maxNA NA values the window will be ignored
extern double maxZero;			// max allowed zero values - if both profiles contains more than maxZero zeros the window will be ignored
extern int nShuffle;			// number of shuffle in percents
extern int nCompare;			// number of observations

extern bool verbose;				// number of suffle
extern int threshold;
extern bool doAutoCorr;
extern int lProfAuto;
extern int corrScale;			// scale for correlations
extern bool silent;				// inhibit stdout
extern bool syntax;				// Strong syntax control

extern bool writeDistCorr;		// write BroadPeak
extern bool outLC;
extern int LCScale;
extern int crossWidth;
extern bool RScriptFg;
extern bool outSpectr;
extern int outRes;
extern bool outChrom;
extern int cage;
extern bool clearProfile; //Force profile recalculation
extern int scoreType;
extern bool writePDF;
extern double lcFDR;		// treshold on FDR when write Local Correlation track

extern int profWithFlanksLength; // size of profWindow array (including random flanks)
//===================================================    results
extern double prod11,prod12,prod22, eprod1,eprod2;	//cummulative scalar products
extern int nprod;
extern double totCorr,BgTotal;
extern int nBkg, nFg;					// size of background and foreground sets of data
extern double *BkgSet, *FgSet;			// background and foreground sets of the correlations

//=================================================== debug and error
extern const char *errStatus;
extern const char *debS;
extern int debugFg;

struct Track;
struct bTrack;
struct Formula;
//================================================== Formula function
struct Identifier{
	char name[80];
	int nodeID; 				//indentifier in the ID list
	char* print(char *b);
	Identifier(){nodeID=0;}
	Identifier(Formula *frm, const char * idd);
};
struct TrackNode:Identifier{
	char* print(char *b);
	bTrack *btr;
	double value;
	int    pos;
	TrackNode(Formula *frm, const char * trackName);
	~TrackNode();
	double getValue(int pos);
};


struct FNode{
	int id;
	Formula* formula;
	int childL;
	int childR;
	int operation;
	double value;

	FNode(Formula* form);
	char *print(char *b);
	double calc();
};

struct Formula{
	FNode *fNodes[256];
	char *formula;
	int mainRoot;
	int nNodes;
	int roots[80];
	int nroots;
	Identifier *identifiers[256];
	TrackNode *tracks[256];
	int nIds, nTracks;
	FNode *arg, *e, *sigma;

	Formula() {init();}
	~Formula() ;
	void init();
	void parse(const char* input);
	int addIdent(const char* ident);
	int addTrack(char* ident);
	Identifier *getIdentificator(const char *);
	TrackNode *getTrack(const char *);
	FNode* getNode(int id) {return fNodes[id];}
	double calc();
	double calc(double x);
	double getValue(const char* ident);
	void setValue(const char* ident, double v);
	void setArg(double x) ;
};


Formula *frmlInit(const char* txt);		// initiate Formula
void   frmlClose(Formula* f);			// free formula
double frmlCalc(Formula* f, double x);	// calculate formula
void   frmlSetValue(Formula* f, const char* txt, double val);	// set value to the variable
double frmlGetValue(Formula* f, const char* txt);				// get the variable value
//====================================================================
const int SG_BUFSIZ=0x100000;
const int SG_BUFEXT=0x10000;		// Max string length
struct BufFile{
	FILE *f;
	char *buffer;
	char *curString;
	BufFile(){f=0; buffer=0; curString=0;}
	BufFile(const char * fname){init(fname);}
	~BufFile();
	void init(const char *fname);
	char *getString();
};
//====================================================================
struct Chromosome{
    char *chrom;		//Chromosome name
    long length;		//Chromosome length
    int  base;			//Start position in binary profile

    double av1, av2, corr, lCorr, count;
    int densCount;
    float *distDens;
    Chromosome(char *chr, long l, int bb);
    void clear();
};

void normChromDist();	// normalize distance distribution

struct GenomePos{
    char* chrom;	// Chromosome name
    long pos;		// Chromosom position
};

struct ScoredRange{
	Chromosome *chr;
    char* chrom;	// Chromosome name
    long beg;		// start Chromosome position
    long end;		// end chromosome position
    float score;	// score
    ScoredRange();
    void printBGraph(FILE *f);
};
//===============================================================
struct MapRange{
//============ example              mapSgm=b:-500..e:+200
	int f,t;
	int cumLength;
	MapRange();
	MapRange(int f, int t);
};

struct IVSet{
	MapRange **ivs;
	int capacity;
	int nIv;
	int totLength;
	int ivNo;
	IVSet();
	void addIv(int f, int t);
	int randPos();
	void fin();
	void clear();
	void write(FILE*f);
	void print(int f, int t);
//	void inspect(const char*msg, int i);
};

//===================================================================
class Exon{
public:
	long from;
	long to;
};

class Exons{
public:
	Exon *list;
	int n;
};

//===============================================================
struct Track;
struct BuffArray;
struct FloatArray;

struct Track{		        // Binary track
	double *profWindow;		// decoded profile with flanks
	char   *name;			// track name
	double  av,				// average score
			sd, 			// score standard deviation
			minP, 			// min score value
			maxP, 			// max score value
			av0,
			sd0,
			nn;
	double avWindow, sdWindow;// mean and stdDev in current window
	float scaleFactor;

	bool hasCompl;
	IVSet *ivs, *ivsC;
	int trackType;
	int deriv;						// order of derivative
	double projCoeff;
	double *autoCorr;				// autocorrelation function

	Track();						// empty constructor
	virtual ~Track();						// empty constructor
	void init();

	bool   openTrack(const char *fname);	// read file
	virtual bool   readTrack(const char *fname)=0;	// read file
	virtual bool   isNA(int pos, bool cmpl)  =0;
	virtual bool   isZero(int pos, bool cmpl)=0;
	virtual double getValue(int pos, int cmpl)=0;
	virtual void 	trackDef(char *s);
	virtual void 	clear();

	double *getProfile(int pos, bool cmpl);	// decode profile starting with given position
	void allocTrack();
	double getProjValue(int pos, bool cmpl);
	double addStatistics();
	void finStatistics();
	int  countNA  (int pos, bool cmpl);		// count NA elements in the window
	int  countZero(int pos, bool cmpl);		// count zero elements in the window

	void ortProject();				// calculate projection coeff
	void makeIntervals();
	void makeIntervals(bool cmpl, IVSet *iv);
	int  getRnd(bool cmpl1);
	void writeWig();
	void writeWig(FILE* f, Chromosome *ch);


};
//============================== Model =======================================
struct bTrack:Track{
	BuffArray *bytes;	// byte_track
	BuffArray *cbytes;	// complement byte_track

	bTrack();
	~bTrack();
	bTrack(const char* fname);		// constructor that reads file
	bool 	check(const char *fname);

	void initBtr();
	virtual bool readTrack(const char *fname);	// read file
	virtual bool isNA(int pos, bool cmpl);
	virtual bool isZero(int pos, bool cmpl);
	virtual double getValue(int pos, int cmpl);
	virtual void clear();

	bool readPrm();
	bool readBin();
	void makeBinTrack(const char *fname);
	void makeBinTrack();
	void writeByteProfile();
	void writeProfilePrm();
	void writeProfilePrm(const char *path);
	void readInputTrack(const char *fname, int cage=0); //cage: if cage>0 : end=beg+cage; cage<0: beg=end+cage.
	int addSgm(ScoredRange *bed, FloatArray *prof);
	int addSgm(char strnd, ScoredRange *bed);
	void initProfile();
	void initProfile(char* name);
	void finProfile();

	double getVal(BINVAL b);
	int    getBVal(int pos, int cmpl);
};
//============================== Model =======================================
struct Model:Track{
	char * definition;
	Formula *form;

	Model();
	~Model();
	void readModel(const char *fnam);
	bool readTrack(const char *fnam);
	bTrack *getTrack(int i) {return form->tracks[i]->btr;}
	char *getTrackName(int i){return form->tracks[i]->name;}
	virtual bool isNA(int pos, bool cmpl);
	virtual bool isZero(int pos, bool cmpl);
	virtual double getValue(int pos, int cmpl);
	virtual void clear();
};
bool isModel(const char *s);
Track * trackFactory(const char* fname);
//=================================================================================
struct BuffArray{
	BINVAL *bval;
	Track *bt;

	FILE *f;
	long offset;
	long bufBeg, bufEnd;
	bool wr;

	~BuffArray();
	BuffArray();
	void init(Track *bt, bool cmpl, bool write);
	BINVAL get(int pos);						// read and remove NA
	void set(int pos, BINVAL v);
	void readBuff(int pos);				// read; if na then as is
	void writeBuff();
	void close();
};

class FloatArray{
private:
	float *val;
	FILE *f;
	char *fname;
	long bufBeg, bufEnd;
	bool wr;
public:
	~FloatArray();
	FloatArray();
	void init(int na);
	float get(int pos);						// read and remove NA
	float getLog(int pos);					// read and remove NA and log(z+1)
	void set(int pos, float v);
	float add(int pos, float v);
	void readBuf(int pos);				// read; if na then as is
	void writeBuf();
};


//================================= Timer ===================================
char *dateTime();
long mtime();
struct Timer{
	long start;
	char bb[80];
	Timer();
	void reset();
	long getTimer();
	char *getTime();
};

//===============================================================
class Fourier{		// Fourier transformation
public:
	int err;
	int length;				// array length
	double *datRe,*datIm, 	// im part of input data (always 0)
			*re, *im; 		// real and im parts of transformation
	float  *spectrum;
	double *autocorr;


	Fourier();				// empty constructor
	Fourier(int n);			// constructor
	~Fourier();
	void freeMem();
	void init(int len);
	void setDat(double *reD);
	void setDat(double *reD, double *imD);
	void calc(double *reD, double *imD, int deriv=0);	//do transform with real and Im data
	void calc(double *dat, int deriv=0);     	// Do transformation with given real data
	void calc(int deriv=0); 			       	// Do transformation
	void calc0(int deriv=0); 			       	// Do transformation
	void calc(double *dRe,double *dIm,double *rRe,double *rIm);
	void norm();						   	// divide transform by length
	void derivat();						   	// get derivative from fft
	void derivat(int deriv);
	float *getSpectrum();					// get spectrum
	double *getAutoCorr();					// get aoutocorrelation
};

//================================ Cross-correlation ==============================
class XYCorrelation:Fourier{
public:
	float *correlation, *corrPlus, *corrMinus, *spectrumX, *spectrumY;  	//correlation function
	int nCorr, nPlus, nMinus;											//counter

	double min,max,av,sd;
	Fourier *fx, *fy;		// source DATA
	void initXY();
	void calcXYCorr(bool cmpl1, bool cmpl2);
	void storeByChrom(int pos,double corr);
	void calcXYCorr(int pos, bool cmpl1, bool cmpl2, double cc);
	void normilize();
	void makeSpectrum();
	void printSpect(char *fname);
};

//=================================================================================
struct Complex{
	double re,im;
	Complex(){re=0; im=0;}
	double Mod();
	Complex scalar(Complex otherC);
};

//=================================================================================
struct statTest{		    // result of a statistical test
	int n1,n2;				// sample sizes
	double u,e,sigma,z,pVal;	// statistics, mean, deviation, z-score and p-value for statistics
};

//=================================================================================
struct Kernel{				// Generic kernel
public:
	char *name;				// kernel name
	int length;				// length (equal to the profile length
	double * kern;			// the kernel values
	double * ckern;			// the complement kernel values
	Fourier ft;				// Fourier transformation for the kernel
	Fourier cft;			// Fourier transformation for the complement kernel
	Fourier fx,fy;			// Fourier transformation for the input data
	bool hasCompl;			// for symmetrical Kernel flag=false; otherwise flag=1;
	virtual ~Kernel(){;}
	void init(int l);
	void fft();
	void fftx(double* , int deriv);		// do transform for given data
	void ffty(double* , int deriv);		// do transform for given data
	double scalar(Fourier *f1, Fourier *f2, Complex *c, bool complem);
	double dist  (Fourier *f1, Fourier *f2, bool complem);
	double dist(bool complem);
	void makeKernel(int l);
	double NSCorrection(double x, double val);
	virtual double kernVal(double x)=0;
};

//=================================================================================
struct  LeftExpKernel:Kernel{
public:
	double sigma, e;
	LeftExpKernel();
	LeftExpKernel(double e,double sgm, int l); //Nucleotides
	double kernVal(double x);
};

//=================================================================================
struct  RightExpKernel:Kernel{
public:
	double sigma, e;
	RightExpKernel();
	RightExpKernel(double e,double sgm, int l); //Nucleotides
	double kernVal(double x);
};

//=================================================================================
struct  CustKernel:Kernel{
public:
	Formula *frml;
	CustKernel();
	CustKernel(double e,double sgm, int l); //Nucleotides
	void initCust(double e,double sgm);
	double kernVal(double x);
};

//=================================================================================
struct  NormKernel:Kernel{
public:
	double sigma, e;
	NormKernel();
	NormKernel(double e,double sgm, int l);	//Nucleotides
	double kernVal(double x);
};

//=========================================================================
struct DinHistogram{		// Dynamic histogram for two variables
	int l;					//number of bins
	int    n[2];			//number of observations
	double min, max;		//min, max observed value
	double hMin, hMax;		// max-min value in the histogram
	double bin;				//bin width
	int    *cnts[2];		//counts
	double *hist[2];		//counts
	double e[2],sd[2];		//mean and std deviation

	DinHistogram(int ll);				//constructor
	~DinHistogram();
	int getIdx(double value);			// Get index for the value
	double getValue(int idx);			// Get value by index
	double getNormValue(int idx);		// Get normalized value (0..1)
	int compress2Left(double value);	// Compress the histogram to left
	int compress2Right(double value);	// Compress the histogram to left
	void addStat(double value, int count, int type);
	void add(double value, int type);				// Add the value
	void add(double value, int count, int type);				// Add the value
	void fin();
	void print(FILE* f);
	void clear();
};
//=================================================================================
//=================================================================================
struct Interval{
	int f,t;
};

//=================================================================================
//================= Alias classes are used for give shorter file names.
//================= It uses the alias table and renames the output files
struct Alias{
	char *oldName;
	char *newName;
	int lnew, lold;
};

class AliasTable{
public:
	Alias *als;
	int nAls;
	AliasTable(){nAls=0; als=0;}
	void readTable(const char* fname);
	char *convert(char*oldName, char *newName);
};

//======================== File list Entry ==========================
struct FileListEntry{
	int id;
	char *fname;
};

//===================================================================
struct PairEntry{			//==== correlation for pair of windows
	int profPos;			//==== Profile position
	float d;				//==== Correlation
};

//===================================================================
struct Matrix{
	int n;
	double * values;
	//=========================================
	//=========================================
	Matrix(){n=0; values=0;}
	Matrix(int mm);
	Matrix(int nn, double *a);
	Matrix(Matrix *mtx);
	void init(int nn);
	void init(int nn, double *a);
	//=========================================
	~Matrix(){free(values);}
	//=========================================
	int getIndex(int i, int j){return i*n+j;}
	//=========================================
	double get(int i, int j){return values[getIndex(i,j)];}
	//=========================================
	void set(int i, int j, double a){values[getIndex(i,j)]=a;}
	//=========================================
	void set(double *a){memcpy(values,a,n*n*sizeof(double));}
	//=========================================
	void transpose();
	void printMtx(FILE *f);
	void printMtx();
};

struct CovarMtx:Matrix{
	double *cov, *meani, *meanj;
	int *count;
	CovarMtx(int n);
	void init(int n);
	CovarMtx(){;cov=0; meani=0; meanj=0; count=0;}
	~CovarMtx();
	double calc(int i, int j);
	void addCov(int itrack, int jtrack, int f, int t);
	void print(FILE *f);
};
struct VectorX{
	double *v;
	int n;
	VectorX();
	VectorX(int nn);
	void init(int nn);

	void random(int nz);
	void get(int pos, double *b);
	void get(int pos);
	int chk(int pos);
	int chk();
	double scalar(VectorX *v);
	double scalar(VectorX &v);
	void print(FILE *f);
	void printH(FILE *f);
};
Matrix * eigenVectors(Matrix *x, double *EValues, int nIter, double precsision);

//=====================================================================

//=====================================================================
extern AliasTable alTable;
extern Chromosome *chrom_list;       // list of chromosomes
extern int n_chrom;
extern Chromosome *curChrom;
extern Track *track1, *track2, *projTrack; // Binary profiles
extern Kernel *kern;			// current kernel
extern FileListEntry files[256];
extern int   nfiles;

extern XYCorrelation XYfgCorrelation;		// array for correlation picture
extern XYCorrelation XYbgcorrelation;		// array for background correlation picture

extern Fourier LCorrelation;			// distance correlation calculation

extern PairEntry *pairs;				// array for pair's correlation (foreground)
extern int nPairs;						// number of foreground observations
//extern Histogram bgHist;				// Background Histogram initiation
//extern Histogram fgHist;				// Foreground Histogram initiation
extern double BgAvCorr;					// average Background correlation
extern double FgAvCorr;					// average Foreground correlation
extern double LCmin, LCmax;
extern FloatArray *fProfile, *cProfile;
extern Timer debTimer;


//=============================== Chromosomes ===========================
int  readChromSizes(char *fname);			// read chromosomes
Chromosome *getChromByPos(int pos);			// get Chromosome by file position
long pos2filePos(char*chrom,long pos);		// transform genome position to byte-profile position
void filePos2Pos(int pos, ScoredRange *gr, int length); // transform byte-profile position to genome position
Chromosome* findChrom(char *ch);			// Find chromosome record by name
Chromosome *checkRange(ScoredRange *gr);    // check if genome range is valid
void clearChromosomes();
void addChromStat(int pos, bool cmpl1,bool cmpl2,double corr);

//=============================== Parsing ===========================
char *getAttr(char *s0, const char *name, char *buf); // find attribute by name in the string and return attribute value
char *skipSpace(char *s);		// skip space characters
char *skipNoSpace(char *s);		// skip non-space characters
char *strtoupper(char*s);		// convert string to upper-case
int  EmptyString(const char*buff);
bool isInt(const char *s);
bool isDouble(const char *s);
bool isUInt(const char *s);
char* getMajorVer(const char *ver, char *buf);
char *trim(char *s);
int  keyCmp(const char *str, const char *key);
int  getFlag(char*s);


//=============================== File names ===========================
void makeDir(const char *path);
char *makeFileName(char *b, const char *path, const char*fname);	// make filename using path and name
char *makeFileName(char *b, const char *path, const char*fname, const char*ext);	// make filename using path, name and extension
char *correctFname(char* s);			// remove fucking MS Widows backslash
char *cfgName(char* p, char* ext);			// Make config file name
char *makePath(char* pt);					// Make path - add '/' to the end of pathname
FILE *xopen(const char*, const char*);		// open file if exists, exit otherwise
FILE *gopen(const char*, const char*);		// open file with parsing ~
bool fileExists(const char *fname);				// check if the file exists
bool fileExists(const char* path, const char *fname);				// check if the file exists
bool fileExists(const char* path, const char *fname, const char *ext); // check if the file exists
void makeDirs();
const char *getExt(const char *fname);					// extract file extension
char *getFnameWithoutExt(char *buf, char *fname);
int   getTrackType(const char *fname);
unsigned long getFileTime(const char *fname);
void flockFile(FILE *f);
void funlockFile(FILE *f);
void addFile(char* fname);			// Add filename to file list

void writeBedGr(const char *fname, FloatArray *array, float lTreshold, float rTreshold);
void writeBedGr(FILE* f, FloatArray *array);
void writeBedGr(FILE* f, FloatArray *array, float lTreshold, float rTreshold);

//============================================== read config file
const char*getKernelType();
const char*getPC();//partial correlation
void  makeId();
const char *getIvFlag();

//============================================== random & statistics
double rGauss();							// standard normal random
double rGauss(double e, double sigma);		// normal random with given mean and sigma
unsigned long randInt(unsigned long n);		// uniform random int
double drand();								// uniform random double [0,1]
double xBetaD(double beta1, double beta2, double x);	// beta density
statTest *MannWhitney( double *set1, int nSet1, double *set2,int nSet2);	//Mann-Whitney test

//============================================ Fourier transformation
int nearPow2(int n, int &i);
int nearPow2(int n);
int nearFactor(int n);
void printFactors(int l);
extern "C" void fftl(int n,double are[],double aim[],double bre[],double bim[]);
extern "C" void fft(double xRe[], double xIm[],double yRe[], double yIm[]);
extern "C" void initFFT(int n);

//============================================ Output ====================
double LocalCorrTrack(int pos1, int pos2, bool cmpl1, bool cmpl2, bool rnd);
void calcAutoCorr();
void finOutLC();
void initOutLC();
void freeLC();
void printStat();
void printFgDistr();
void printBroadPeak();
void printRreport();
void printR();
void printRmd();

void printCorrelations();
void printChomosomes(char *fname);
void printChrDistances(char *fname);
void printParamNames(FILE* f);
void printParams(FILE* f);
void printXMLparams(FILE *f);

//============================================ Arrays
double norm(double *x, int l);			// normalize to z-score


//============================================ Debugging and logging
void errorExit(const char *format, ...);
void clearLog();
void writeLog(const char *format, ...);
void verb(const char *format, ...);
void xverb(const char *format, ...);
void deb(int num);
void deb(const char *format, ...);
void deb(int num, const char *format, ...);
void debt(int num);
void debt(const char *format, ...);
void debt(int num, const char *format, ...);
void debt(); 										//reset timer
void clearDeb();

void helpPage();
//======================== Testing procedures
void test();

//============================================= Calculations =============
void printMiniHelp();
void initSG(int argc, char **argv);
int  Correlator();
//int  Preparator(const char *fname);
void  Preparator();
void Covariator();
void Projector();

#endif /* TRACK_UTIL_H_ */
