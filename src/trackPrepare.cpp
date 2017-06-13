//============================================================================
// Name        : cpp.cpp
// Author      : a
// Version     :
// Copyright   : Your copyright notice
//============================================================================

#include "track_util.h"

//float *profile =0;				// uncompressed profile array
//float *profilec=0;				// uncompressed profile array

FloatArray *fProfile=0, *cProfile=0;

//=====================================================================================
const char *memError="not enaugh memory. Try to increase  parameter \"bin\"";

void bTrack::initProfile(){
	initProfile(0);
//	errStatus="init Profile";
//	if(fProfile==0) fProfile=new FloatArray();
//	fProfile->init(NA);
//	if(bytes==0) bytes=new BuffArray();
//	bytes->init(this,0,1);
//	errStatus=0;
};
void bTrack::initProfile(char *tName){
	errStatus="init Profile";
	if(tName){
		name=strdup(tName);
		if(bytes) bytes->close();
	}
	if(bytes==0) bytes=new BuffArray();
	bytes->init(this,0,1);
	if(fProfile==0) fProfile=new FloatArray();
	fProfile->init(NA);
	errStatus=0;
};
//================================================================= Add segment
void addProfVal(FloatArray *p, int pos, float v){
	p->add(pos,v);
}

int bTrack::addSgm(ScoredRange *bed, FloatArray *prof){
	if(!checkRange(bed)) return 0;
	int p1=pos2filePos(bed->chrom, bed->beg);
	int p2=pos2filePos(bed->chrom, bed->end);
	if(p1<0) p1=0; if(p2>=profileLength) p2=profileLength-1;
	float d;
	if(p1==p2){
		d=bed->score*(bed->end-bed->beg)/binSize;
		addProfVal(prof,p1, d);
	}
	else{
		d=(bed->score)*((p1 + 1 - curChrom->base)*binSize - bed->beg)/binSize;
		addProfVal(prof, p1,d);
		for(int i=p1+1; i<p2; i++){
			addProfVal(prof, i, bed->score);
		}
		d=(bed->score)*(bed->end - (p2-curChrom->base)*binSize)/binSize;
		addProfVal(prof, p2,d);
	}
	return 1;
}

int bTrack::addSgm(char strnd, ScoredRange *bed){
	if(strnd=='-') 	{
		if(cbytes==0) {
			cbytes=new BuffArray();
			cbytes->init(this,1,1);
		}
		if(cProfile==0) {
			cProfile=new FloatArray();
			cProfile->init(NA);
		}
		return addSgm(bed, cProfile);
	}
	else  return addSgm(bed, fProfile);
}

//=====================================================================================
int getTypeByExt(const char *sext){
	char bext[80];
	strtoupper(strcpy(bext,sext));
	if(strcmp(bext,"BED")==0) return BED_TRACK;
	if(strcmp(bext,"WIG")==0) return WIG_TRACK;

	if(strcmp(bext,"BEDGRAPH"	)==0) return BED_GRAPH;
	if(strcmp(bext,"BED_GRAPH"	)==0) return BED_GRAPH;
	if(strcmp(bext,"B_GRAPH"	)==0) return BED_GRAPH;
	if(strcmp(bext,"BGR"		)==0) return BED_GRAPH;

	if(strcmp(bext,"B_PEAK"		)==0) return BROAD_PEAK;
	if(strcmp(bext,"BPEAK"		)==0) return BROAD_PEAK;
	if(strcmp(bext,"BROAD_PEAK"	)==0) return BROAD_PEAK;
	if(strcmp(bext,"BROADPEAK"	)==0) return BROAD_PEAK;

	if(strcmp(bext,"MODEL"	)==0) return MODEL_TRACK;
	if(strcmp(bext,"MOD"	)==0) return MODEL_TRACK;
	if(strcmp(bext,"MDL"	)==0) return MODEL_TRACK;

	return 0;
}
int getTrackType(const char *fname){
	const char *sext=getExt(fname);
	int tt=sext ? getTypeByExt(sext) : 0;
	return tt;
}
//============================================================ read track definition line
void Track::trackDef(char *s){
	char bb[100], *st;
	if(trackType==0){
		trackType=BED_TRACK;
		st=getAttr(s,"type",bb);
		if(st!=0){
			strtoupper(st);
			if(strncmp(st,"WIG",3)==0) 	     trackType=WIG_TRACK;
			if(strncmp(st,"BEDGRAPH" ,8)==0) trackType=BED_GRAPH;
			if(strncmp(st,"BROADPEAK",8)==0) trackType=BROAD_PEAK;
		}
	}
	st=getAttr(s,"name", bb);
	if(st!=0) strcpy(trackName,st);
}

//=====================================================================================
int countFields(char *b){
	int n=0;
	for(char *s=b; s; n++){
		char *ss=strchr(s,' ');
		if(ss==0) ss=strchr(s,'\t');
		if(ss) ss++;
		s=ss;
	}
	return n;
}

int checkWig(char *b){
	if(strncmp(b,(char *)"variableStep", 12)==0) return WIG_TRACK;
	if(strncmp(b,(char *)"fixedStep"   ,  9)==0) return WIG_TRACK;
	if(countFields(b)==4) return BED_GRAPH;
	return WIG_TRACK;
}

char * readChrom(char *s){return strtok(s," \t\n\r");}
char* readChrom(){return strtok(0," \t\n\r");}
long readInt(){
	char *s=strtok(0," \t\n\r");
	if(s==0) return -1;
	if(! isdigit(*s)) return -1;
	return atol(s);
}
double readFloat(){
	char *s=strtok(0," \t\n\r");
	if(s==0) return -1;
	return atof(s);
}



void bTrack::readInputTrack(const char *fname, int cage){
	char *chrom=0;
    int fg=-1;
	int span=1, step=1;
	long start=0, beg=0, end=0;
	float score=0;
    ScoredRange bed;
	char *inputString, abuf[1000], chBuf[256], *sx;
	long i=0;
	char strand=0;
	int nStrand=0;
	hasCompl=1;
	trackType=getTrackType(fname);
	int intervFlag=(trackType == BED_TRACK) ? intervFlag0 : 0;

	FILE *f=xopen(fname, "rt"); setvbuf ( f , NULL , _IOFBF , 65536 );
	strcpy(curFname,fname);
	inputErrLine=inputErr = 0;		// flag: if input track has no errors
	BufFile input;
	input.init(fname);

	for(;(inputString=input.getString())!=0; i++){
		inputErrLine++;
		int dataFg=1;									 	//======== remove end-of-line signes
		if(i%10000000 ==0)
			{verb("%li  %s...\n",i,curChrom->chrom); }
		if(*inputString==0) continue;

		if(strncmp(inputString,"browser"  ,7)==0) continue;					//========= browser line => skip
		if(strncmp(inputString,"track"    ,5)==0) {trackDef(inputString); continue;}
		if(strncmp(inputString,"#bedGraph",9)==0) {trackType=BED_GRAPH; continue;}
		if(*inputString=='#' || *inputString==0) continue;							//======== comment line
		if(trackType==WIG_TRACK) trackType=checkWig(inputString);

		beg=-1; end=-1;

		switch(trackType){
			case BED_TRACK:										//======== BED
				intervFlag=(trackType==BED_TRACK) ? intervFlag0 : 0;
				score=1;										//======== default score=1
				strand=0;										//======== default strand=unknown
				chrom=readChrom(inputString);								//======== find chrom field
				beg=readInt();									//======== find beg field
				end=readInt();									//======== find end field
				sx=strtok(0," \t\n"); if(sx==0) break;			//======== ignore name field
				sx=strtok(0," \t\n"); if(sx==0) break;			//======== take score field
				if(*sx!=0 && (isdigit(*sx) || *sx=='-')) score=atof(sx);
				if(intervFlag) score=100;
				sx=strtok(0," \t\n"); if(sx==0) break;			//======== take strand field
				if(*sx!=0 && *sx!='.') {strand=*sx; nStrand++;}
				if(intervFlag==GENE || intervFlag==NONE) break;
				if(intervFlag==GENE_BEG){
					if(strand=='-') beg=end-1;
					else 			end=beg+1;
					break;
					}
				if(intervFlag==GENE_END) {
					if(strand=='-') end=beg+1;
					else  			beg=end-1;
					break;
				}
				sx=strtok(0,"\t"); if(sx==0) break;			//======== skip tick1
				sx=strtok(0,"\t"); if(sx==0) break;			//======== skip tick2
				sx=strtok(0,"\t"); if(sx==0) break;			//======== skip rgb
				sx=strtok(0,"\t"); if(sx==0) break;			//======== number of exons
//======================= read gene structure
				{
				int nn=atoi(sx);
				if(nn<2) break;
				int lExn[nn];
				int posExn[nn];
				bed.chrom=chrom; bed.score=score;
				for(int i=0; i<nn; i++) {
					sx=strtok(0,",\t");
					if(sx==0){beg=-1; break;}
					lExn[i]=atoi(sx);
				}
				for(int i=0; i<nn; i++) {
					sx=strtok(0,",\t");
					if(sx==0){beg=-1; break;}
					posExn[i]=atoi(sx);
				}
				int genePos=beg;
//===== now 'beg' and 'end' are positions relative to gene beg/
				if((intervFlag&EXON)==EXON){
					for(int i=0; i<nn; i++){
						end=(beg=posExn[i])+lExn[i];
						if(intervFlag==EXON_BEG){
							if(strand=='-') beg=end-1;
							else 			end=beg+1;
						}
						if(intervFlag==EXON_END){
							if(strand=='-') end=beg+1;
							else  			beg=end-1;
						}
					bed.beg=genePos+beg; bed.end=genePos+end;
						addSgm(strand, &bed);
					}
					break;
				}
				if((intervFlag&IVS)==IVS){
					for(int i=0; i<nn-1; i++){
						beg=posExn[i]+lExn[i];
						end=posExn[i+1];
						if(intervFlag==IVS_BEG){
							if(strand=='-') beg=end-1;
							else 			end=beg+1;
						}
						if(intervFlag==IVS_END) {
							if(strand=='-') end=beg+1;
							else  			beg=end-1;
						}
						bed.beg=genePos+beg; bed.end=genePos+end;
						addSgm(strand, &bed);
					}
					break;
				}
				}
				break;
			case BED_GRAPH:
				chrom=readChrom(inputString);		//======== find chrom field
				beg=readInt();			//======== find beg field
				end=readInt();			//======== find end field
				score=readFloat();		//======== find score field
//if(score > 1000) deb("input=<%s>",testB);
				break;
			case WIG_TRACK:
				if(*inputString=='v'){
				if(strncmp(inputString,"variableStep", 12)==0){			//======== read parameters for variableStep
					chrom=getAttr(inputString+12,(char *)"chrom",chBuf);
					span=1; fg=0; dataFg=0;
					sx=getAttr(inputString+12,(char *)"span",abuf);
					if(sx!=0) span=atoi(sx);
				}}
                else if(*inputString=='f'){
                	if(strncmp(inputString,(char *)"fixedStep", 9)==0){	//======== read parameters for fixedStep
                	fg=1; dataFg=0;
					chrom=getAttr(inputString+10,(char *)"chrom",chBuf);
					sx=getAttr(inputString+10,(char *)"span",abuf);
					if(sx!=0) span=atoi(sx);
					sx=getAttr(inputString+10,(char *)"step",abuf);
					if(sx!=0) step=atoi(sx);
					sx=getAttr(inputString+10,(char *)"start",abuf);
					if(sx!=0) start=atol(sx);
					}}
				else{
					if(fg==0){									//======== variableStep
						if((sx=strtok(inputString," \t\n\r"))==0) continue;
						beg=atoi(sx); end=beg+span;
						if((sx=strtok(0," \t\n\r"))==0) continue;
						score=atof(sx);
					}
					else if(fg==1){										//======== fixedStep
						beg=start; end=beg+span; start=beg+step;
						sx=inputString; if(sx==0) break;
						score=atof(sx);
					}
					else errorExit("wrong WIG format");
				}
				break;
			case BROAD_PEAK:
				chrom=readChrom(inputString);				//======== find chrom field
				beg=readInt();					//======== find beg field
				end=readInt();					//======== find end field
				strtok(0," \t\n\r");							//======== skip name
				sx=strtok(0," \t\n\r"); if(sx==0) break;
				if(bpType==BP_SCORE) score=atof(sx);				//======== find score field
				sx=strtok(0," \t\n"); if(sx==0) break;			//======== take strand field
				if(*sx!=0 && *sx!='.') {strand=*sx; nStrand++;}
				if(bpType==BP_SCORE) break;
				sx=strtok(0," \t\n"); if(sx==0) break;			//======== take signal field
				if(bpType==BP_SIGNAL) {score=atof(sx); break;}
				sx=strtok(0," \t\n"); if(sx==0) break;			//======== take pval field
				if(bpType==BP_LOGPVAL) {score=atof(sx); break;}
				break;
			default:
				errorExit("track type undefined or unknown"); break;
		}
		if(dataFg && (chrom==0 || beg < 0  || end < 0)){
			if(syntax)
				errorExit("wrong line in input file <%s> line# <%i>\n",fname,i);
			else{
				writeLog("wrong line in input file <%s> line# <%i>\n",fname,i);
				verb("wrong line in input file <%s> line# <%i>\n",fname,i);
			}
			continue;
		}
		bed.chrom=chrom; bed.beg=beg; bed.end=end; bed.score=score;
		if(cage > 0) bed.end=bed.beg+cage;
		if(cage < 0) bed.beg=bed.end+cage;
		if(dataFg) {
			addSgm(strand, &bed);
		}
	}
	if(nStrand==0) hasCompl=0;
	fclose(f);
}
//=====================================================================================
//========================================================================================

void testDistrib(){
	float dd[1000]; memset(dd,0,sizeof(dd));
	for(int i=0; i<profileLength; i++){
		float xx=fProfile->get(i);
		if(xx==NA) dd[0]++;
		else{
			float x=log(1+xx);
			int k=(int)(x/10*1000)+1;
			dd[k]++;
		}
	}

	FILE *f=xopen("dstr","wt");
	for(int i=0; i<1000; i++){
		fprintf(f,"%5.2f\t%6f\n",i*10./1000.,dd[i]);
	}
	fclose(f);
}

void bTrack::finProfile(){
	errStatus="finProfile";
	double lprof=0.;
	av=0.;            // Average profile value
	sd=1.;          // Standard deviation
	minP= 5.e+20;      // Minimal profile value
	maxP=-5.e+20;      // Maximal profile value
	//============================ calculate min, max, average, std deviation
	double x2=0;
	if(trackType==BED_TRACK){
		for(int i=0; i<profileLength; i++){
			if(fProfile->get(i) == NA) fProfile->set(i,0);
			if(cbytes && cProfile->get(i) == NA) cProfile->set(i,0);
		}
	}
	int nn=0;
	for(int i=0; i<profileLength; i++){
		float z=fProfile->getLog(i);
		if(z != NA){
			// we take into account only valid profile values
			av+=z; x2+=z*z; nn++;
			lprof+=1;
			if(z < minP) minP=z;
			if(z > maxP) maxP=z;
		}
		if(cbytes && (z=cProfile->getLog(i)) != NA){
			// we take into account only valid profile values
			av+=z; x2+=z*z; nn++;
			lprof+=1;
			if(z < minP) minP=z;
			if(z > maxP) maxP=z;
		}
	}
	av/=nn;
	if(maxP < minP){errorExit("=== !!!!  The profile contains no data:  min=%.2e max=%.2e !!!===\n",minP,maxP);}
	if(maxP==minP){	errorExit("=== !!!!  The profile contains only zeros: min=%.2e max=%.2e  !!!===\n",minP,maxP);}
	x2-=av*av*nn;
	sd=sqrt(x2/(nn-1));

	scaleFactor=-minP;
	if(scaleFactor < maxP) scaleFactor = maxP;
	scaleFactor=(MAX_SHORT-2)/scaleFactor;
	for(int i=0; i<profileLength; i++){
		float z=fProfile->getLog(i);
		if(z < minP) z=minP;
		if(z > maxP) z=maxP;
		if(z !=NA)  {
			int x=int(scaleFactor*z+0.5);
			bytes->set(i,x);
		}
		else{
			bytes->set(i,NA);       // undefined values are presented as 0 in the byte profile
		}
		if(cbytes!=0){
			z=cProfile->getLog(i);
			if(z != NA){
				cbytes->set(i,int(scaleFactor*z));
			}
			else             cbytes->set(i,NA);       // undefined values are presented as 0 in the byte profile
		}
	};
	errStatus=0;
	bytes->writeBuff();
	if(cbytes) cbytes->writeBuff();
};
//=================================================================================
//=================================================================================
//#        Write the byte profile in two files.
//#            *.prm - file with average min and max values
//#            *.bprof - byte profile

//=================================================================================
unsigned int getChkSum(BuffArray *b, int n){
	unsigned int chksum=0;
	for(int i=0; i<n; i++){
		chksum+=b->get(i);
	}
	return chksum;
}

void bTrack::writeProfilePrm(){
	writeProfilePrm(profPath);
}
void bTrack::writeProfilePrm(const char *path){
    //============================================= Write parameters
	char prmFname[4096];
	makeFileName(prmFname,path,name,PRM_EXT);
	verb("write prm %s...\n",prmFname);
    FILE *f=xopen(prmFname, "wt"); if(f==0)return;

	fprintf(f,"#====== THIS IS GENERATED FILE. DO NOT EDIT!  ========\n");
	fprintf(f,"version=%s\n",version);
	fprintf(f,"#=== Input data ===\n");

    fprintf(f,"trackType=%i\n",trackType);
	fprintf(f,"input=%s\n",name);

    fprintf(f,"#=== Parameters ===\n");
    fprintf(f,"bin=%i\n",binSize);
    fprintf(f,"strand=%i\n",hasCompl);
    if(trackType == BROAD_PEAK)  fprintf(f,"bpType=%i\n",bpType);
    if(cage) fprintf(f, "cage=%i\n", cage);
    fprintf(f,"#=== Statistics ===\n");
    fprintf(f,"min=%g\n",minP);
    fprintf(f,"max=%g\n",maxP);
    fprintf(f,"average=%g\n",av);
    fprintf(f,"stdDev=%g\n",sd);
    fprintf(f,"ivFlag=%i\n",intervFlag0);
    unsigned int chk=getChkSum(bytes, profileLength);
    if(cbytes && hasCompl) chk+=getChkSum(cbytes, profileLength);
    fprintf(f,"chksum=%08x\n",chk);
    fprintf(f,"#=== Scale ===\n");
    fprintf(f,"scale=%f\n",scaleFactor);
    //==== calculate distribution

    int dstr[256];
    float cdstr[256],nn=0;
    memset(dstr,0,sizeof(dstr));

    for(int i=0; i<profileLength; i++){
    	int xx=bytes->get(i);
    	if(xx!=NA)
    		{int x=xx*128./MAX_SHORT; dstr[x+128]++;}
    	if(cbytes && hasCompl){
    		xx=cbytes->get(i);
    		if(xx!=NA) {
    			int x=xx*128./MAX_SHORT; dstr[x+128]++;
    		}
    	}
    }

    cdstr[0]=0; nn=0;
    for(int i=0; i<256; i++){
    	nn+=dstr[i];
    	if(i>0) cdstr[i]=nn;
    }
    fprintf(f,"\n#***** Distribution ***** nn=%i  pLength=%i\n",(int)nn,profileLength);
    for(int i=0; i<256; i++){
    	int b=(i-128.)/128.*MAX_SHORT;
    	fprintf(f,"#%3i\t%f\t%i\t%.0f\t%5.2f\t%5.2f\n",i,getVal(b),
    			dstr[i],cdstr[i], cdstr[i]*100./nn, (nn-cdstr[i])*100./nn);
    }

	fclose(f);
}
void bTrack::writeByteProfile(){
	clear();
}

//=================================================================================
void bTrack::makeBinTrack(const char *fname){
	name=strdup(fname);
	makeBinTrack();
}

void bTrack::makeBinTrack(){
	Timer tm;
	verb ("******   Make binary track <%s>   ******\n", name);
	writeLog("    Make binary track <%s>\n",name);
	//===================================================== prepare track file name
	if (pcorProfile!=0)	 verb("===          pcorProfile=  <%s>\n",pcorProfile);
	trackType=getTrackType(name);
	//=====================================================================================
	initProfile();                   					//============ Allocate arrays
	char pfil[4096];
	readInputTrack(makeFileName(pfil,trackPath,name));		//============ Read tracks
	//======================================================================
	verb("Finalize profiles... \n");
	finProfile(); //============ Calculate min,max,average; convert to bytes
	verb("Write profiles... \n");
	writeProfilePrm();
	writeByteProfile();					//============ write binary profiles
	writeLog("    Make binary track -> OK. Time=%s\n",tm.getTime());

	return;
}


//================================================================================
//===================================================================================
bTrack *tmpBTrack;
float* tmpProf;

bool isModel(const char *s){
	return getTrackType(s)==MODEL_TRACK;
}
//=======================================================================================
//=======================================================================================
//=======================================================================================
//=======================================================================================
int nPrepare=0;
void prepare(const char * fname){
	bTrack *tmp=new bTrack();
	if(isModel(fname)){
		Model *tmpModel=new Model();
		tmpModel->readModel(fname);
		for(int i=0 ; i < tmpModel->form->nTracks; i++){
			char *bfname=tmpModel->getTrackName(i);
			if(!tmp->check(bfname)){
				tmp->makeBinTrack(bfname);
				nPrepare++;
			}
		}
		delete tmpModel;
	}
	else if(!tmp->check(fname)) {
		tmp->makeBinTrack(fname);
		nPrepare++;
	}
	delete tmp;
}

void Preparator(){
	Timer tm;
	nPrepare=0;
	for(int i=0; i<nfiles; i++){
		char *fname=files[i].fname;
		if(fname==0 || strlen(trim(fname))==0) continue;
		prepare(fname);
	}
	if(pcorProfile){
		prepare(pcorProfile);
	}
	if(fProfile) delete fProfile;
	if(cProfile) delete cProfile;
	fProfile=cProfile=0;
	if(nPrepare) writeLog("Preparation %i tracks: Time=%s\n",nPrepare,tm.getTime());
}
