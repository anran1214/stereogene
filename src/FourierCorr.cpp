/*
 * FourierCorr.cpp
 *
 *  Created on: Feb 20, 2013
 *      Author: mironov
 */
#include "track_util.h"


Histogram bgHist(1000);					// Background Histogram initiation
Histogram fgHist(1000);					// Foreground Histogram initiation


double *BkgSet, *FgSet;			// background and foreground sets of the correlations
int nBkg, nFg;					// size of background and foreground sets of data
int maxPairs=0, fstep=0;
double *autoCorrx;				// autocorrelation
double *autoCorry;
double *Corrxy;
double *xDat,*yDat,*xyCorr;  	//Woriking arrays

PairEntry *pairs;				// array for pair's correlation (foreground)
int nPairs;						// number of foreground observations

void clear(){
	writeLog("clear Correlator\n");
	xfree(FgSet,"FgSet");
	xfree(pairs,"pairs");
	nBkg=0; nFg=0; nPairs=0;
}

//============================================== subtract profile from profile
double scalar(double *px, double *py, int l){	//partial correlation variant 1
	double res = 0;
	for (int i = 0; i < l; i++){
		res += px[i]*py[i];
	}
	return res;
}

void minusProf(double *profx, double *profz, double pcorCoef){	//partial correlation variant 1

	for (int i = LFlankProfSize; i < LFlankProfSize + profileLength; i++){
		profx[i] -= profz[i]*pcorCoef;
	}
}

Correlation::Correlation(){
	nCorr=nPlus=nMinus=0; min=max=av=sd=0;
	correlation=corrMinus=corrPlus=spectrumX=spectrumY=0;
}
void Correlation::init(){
	nCorr=nPlus=nMinus=0; min=max=av=sd=0;
	getMem0(correlation,profWithFlanksLength, "init correlation #1"); 	zeroMem(correlation,profWithFlanksLength);
	getMem0(corrMinus  ,profWithFlanksLength, "init correlation #2");	zeroMem(corrMinus,profWithFlanksLength);
	getMem0(corrPlus   ,profWithFlanksLength, "init correlation #3");	zeroMem(corrPlus,profWithFlanksLength);
	getMem0(spectrumX   ,profWithFlanksLength, "init correlation #4");	zeroMem(spectrumX,profWithFlanksLength);
	getMem0(spectrumY   ,profWithFlanksLength, "init correlation #5");	zeroMem(spectrumY,profWithFlanksLength);
}

void Correlation::calcWindowCorrelation(int pos, bool cmpl1, bool cmpl2, double corr){
//	kern->restore();
	for(int i=1; i<profWithFlanksLength; i++){
		double  ReX=kern->fx.re[i],
				ReY=kern->fy.re[i],
				ImX=kern->fx.im[i],
				ImY=kern->fy.im[i];
		ImX=cmpl1 ? -ImX : ImX; ImY=cmpl2 ? -ImY : ImY;
		wCorrelation.datRe[i]=(ReX*ReY+ImX*ImY);
		wCorrelation.datIm[i]=(-ReX*ImY+ImX*ReY);
		spectrumX[i]+=(float)(ReX*ReX+ImX*ImX);
		spectrumY[i]+=(float)(ReY*ReY+ImY*ImY);
	}

//	reverse transformation
	wCorrelation.calc(0);
	wCorrelation.norm();

	double delta=0.;
	Chromosome* chr=0;
	if(pos>=0) chr=getChromByPos(pos);
	for(int i=0; i<profWithFlanksLength; i++){
		double x=wCorrelation.re[i];
		correlation[i]+=x;
		if(chr) {chr->distDens[i]+=x; chr->densCount++;}
		if(corr < - 10) continue;
		if(corr >  delta) corrPlus [i]+=x;
		if(corr < -delta) corrMinus[i]+=x;
	}
	nCorr++;
	if(corr >  delta) nPlus++ ;
	if(corr < -delta) nMinus++;
}

void Correlation::norm(){
	min=1.e+18; max=-1.e+18; av=sd=0;
	int n=0;
	double e=bTrack1.av*bTrack2.av;
	double d=(bTrack1.sd*bTrack2.sd*profWithFlanksLength*(nCorr));
	for(int i=0; i<profWithFlanksLength; i++){
		double x=(correlation[i]-e*nCorr)/d;
		correlation[i]=x;
		corrPlus [i]=(corrPlus [i]-e*nPlus )/d;
		corrMinus[i]=(corrMinus[i]-e*nMinus)/d;
		if(min > x) min=x;
		if(max < x) max=x;
		av+=x; sd+=x*x; n++;
	}
	av/=n; sd=sd-av*av*n; sd/=n-1; sd=sqrt(sd);
}



//============================================== Calculate correlation for given pair of windows
//============================================== Add statistics to chromosome
void addChromStat(int pos, double corr, double lCorr, double av1, double av2){
	ScoredRange gr;

	filePos2Pos(pos,&gr,0);

	Chromosome *chr=gr.chr;
	chr->corr+=corr;
	chr->lCorr+=lCorr;
	chr->av1+=av1;
	chr->av2+=av2;
	chr->count++;
}
//=======  input: to position and complement flags. rnd: if the data comes from shuffling

double calcCorelations(int pos1, int pos2, bool cmpl1, bool cmpl2, bool rnd){
	int na1=bTrack1.countNA(pos1,cmpl1);			// count Na's  in the first profile
	int na2=bTrack2.countNA(pos2,cmpl2);			// count Na's  in the second profile
	int nz1=bTrack1.countZero(pos1,cmpl1);			// count zeros in the first profile
	int nz2=bTrack2.countZero(pos2,cmpl2);			// count zeros in the second profile
	if(na1 > maxNA) {return -100;}					// too many NA in the first profile
	if(na2 > maxNA) {return -200;}					// too many NA in the second profile
	if(nz1 > maxZero || nz2 > maxZero) {
		return -300; // too many zeros in the profiles
	}
	double *pr1=bTrack1.getProfile(pos1,cmpl1);		// decode the first profile. Decoder uses hasCompl and complFg flags and combines profiles
	double *pr2=bTrack2.getProfile(pos2,cmpl2);		// decode the second profile

	kern->fftx(pr1,bTrack1.deriv);					// do fft for the profiles
	kern->ffty(pr2,bTrack2.deriv);

	double corr=kern->dist(cmpl1);					// Kernel strand is selected by the first profile

	if(corr > -10){									// Error in the correlation => skip the pair of the windows
		if(!rnd) {									// foreground distribution
			double lCorr=0, av1, av2;
			//======= Calc the correlation
			correlation.calcWindowCorrelation(pos1,cmpl1, cmpl2,corr);
			if(outWIG){				                 // Make local correlation track
				lCorr=storeCorrTrack(pos1, cmpl1, cmpl2);
			}
			av1=bTrack1.addStatistics();
			av2=bTrack2.addStatistics();
			addChromStat(pos1,corr,lCorr, av1,av2);	// Add data to chromosome statistics
		}
		else
			bgcorrelation.calcWindowCorrelation(-1,cmpl1, cmpl2,-100); //Do background correlation function
	}
	return corr;
}
//======================================================= Calculate the total correlation
double calcCC(){
	double c11=prod11-eprod1*eprod1/nprod*profWithFlanksLength;
	double c22=prod22-eprod2*eprod2/nprod*profWithFlanksLength;
	double c12=prod12-eprod1*eprod2/nprod*profWithFlanksLength;
	double cc=c12/sqrt(c11*c22);
	return cc;
}

void cleanCummulative(){
	prod11=0; prod12=0; prod22=0; eprod1=0; eprod2=0; nprod=0;
}
//======================================================== Calculate background distributions
void distrBkg(){
	errStatus="bkg Distrib.";
	verb("\nBakcground...");
	long l=bTrack1.lProf-wProfSize;				// max available position in the profiles
	long nSimul;								// number of shuffles
	char b[1024];
	srand(33);									// random seed

	nSimul=	(long)((double)nShuffle*l/wProfStep/100.);
	if(nSimul > maxShuffle) nSimul = maxShuffle;
	if(nSimul < minShuffle) nSimul = minShuffle;
	getMem0(BkgSet,(nSimul+10), "bkg Distr"); nBkg=0; 	// allocate array for background observations
	strcat(strcpy(b,outFile),".bkg");					// open file for background observations
	FILE *f=0;
	if(writeDistr) f=xopen(b,"wt");
	//================================================== simulations
	int tst=0;											// count runs with zero/NA frames

	cleanCummulative();
	int n_corr=0; BgAvCorr=0;
	for(int i=0; i<nSimul; i++){
		int p1;
		int p2;
		bool cmpl1=bTrack1.hasCompl && (rand() > RAND_MAX/2);
		if(cmpl1){if(bTrack1.ivsC.nIv==0) cmpl1=false;}
		else     {if(bTrack1.ivs .nIv==0) cmpl1=true;}

		bool cmpl2=bTrack2.hasCompl && (rand() > RAND_MAX/2);
		if(cmpl2){if(bTrack2.ivsC.nIv==0) cmpl2=false;}
		else     {if(bTrack2.ivs .nIv==0) cmpl2=true;}

		p1=bTrack1.getRnd(cmpl1);
		p2=bTrack2.getRnd(cmpl2);
		double d=calcCorelations(p1,p2, cmpl1, cmpl2, true);		// calculate correlation

		if(d<=-10) {							// invalid windows (too many NA's of Zeros)
			i--;
			if(tst++ > 10000){					// too many attempt to get a background correlations
				errorExit("too many empty/zero windows\n");
			}
			continue;
		}
		n_corr++; BgAvCorr+=d;
		if(i%10000 ==0) verb("\nShuffling: %i/%li",i,nSimul);
		else if(i%1000 ==0) verb(".");
		tst=0;
		bgHist.add(d);							// store in the histogram
		BkgSet[nBkg++]=d;						// store in distribution
		if(writeDistr) fprintf(f,"%f\n",d);		// write to distribution
	}

	double cc=calcCC();
	BgAvCorr/=n_corr;
	BgTotal=cc;
	xverb("\nbg_cc=%f \nbg_average=%f\n",cc,BgAvCorr);
	if(writeDistr) fclose(f);
	bgHist.normBeta();								// finalize the histogram
	errStatus=0;
}


//============================================ Store foreground distribution
inline void storePair(int i, double d){
	PairEntry *pe=pairs+(nPairs++);						//== store pair of positions
	if(i%fstep == 0) FgSet[nFg++]=d;					//== store distributions (the windows should not overlap)
	pe->profPos=i; pe->d=(float)d;// pe->rank=0;		//== define the pair
	fgHist.add(d);
}

//============================================= Calculate coherent correlations
void distrCorr(){
	verb("\nForeground...");
	int l=bTrack1.lProf-wProfSize;
	errStatus="distrCorr";
	maxPairs=l/wProfStep; fstep=wProfSize/wProfStep;
	if(fstep==0) fstep=1;
	initOutWig();
	int nTrkPair=1; if(bTrack1.hasCompl) nTrkPair*=2; if(bTrack2.hasCompl) nTrkPair*=2;
	maxPairs*=nTrkPair;

	int siz=(maxPairs+100);
	getMem0(FgSet, siz, "dist Corr #1");			//== array for foreground distribution
	getMem0(pairs, siz, "dist Corr #2");			//== array for pairs

	cleanCummulative();

	//=================== calculate correlations
	int n_corr=0; FgAvCorr=0;
	for(int i=0,k=0; i<l; i+=wProfStep,k++){

		double d;
		d=100.*k/(l/wProfStep);
		if(k%10000 ==0) verb("\ncoherent: %4.1f%% (%6i/%i) ",d,k,l/wProfStep);
		else if(k%1000 ==0) verb(".");
		if((complFg&COLLINEAR)!=0 ||								// analyze collinear chains
				   ((!bTrack1.hasCompl) && (!bTrack2.hasCompl))){	// OR if both tracks are NOT strand-dependent
			if((d=calcCorelations(i,i, false,false,false)) >=-10){	    // => =>  valid pair
				storePair(i,d); n_corr++; FgAvCorr+=d;
			}
			if((bTrack1.hasCompl && bTrack2.hasCompl) &&		// first tacks is strand-dependent
			   (d=calcCorelations(i,i,true,true,false)) >=-10){		// <= <=  valid pair
					storePair(i,d); n_corr++; FgAvCorr+=d;
			}
			if((!bTrack1.hasCompl && bTrack2.hasCompl) &&		// second tacks is strand-dependent
			   (d=calcCorelations(i,i,false,true,false)) >=-10){		// => <=  valid pair
					storePair(i,d); n_corr++; FgAvCorr+=d;
			}
			if((bTrack1.hasCompl && !bTrack2.hasCompl) &&		// both of the tacks is strand-dependent
			   (d=calcCorelations(i,i,true,false,false)) >=-10){		// <= =>  valid pair
					storePair(i,d); n_corr++; FgAvCorr+=d;
			}
		}
		if((complFg&COMPLEMENT)!=0 &&
				((bTrack1.hasCompl) && (bTrack2.hasCompl))){ 							// analyze complement chains
			if((bTrack1.hasCompl) &&						    // profile 1 is strand dependent
			   (d=calcCorelations(i,i, true, false,false)) >=-10){	// <= =>  valid pair
					storePair(i,d); n_corr++; FgAvCorr+=d;
			}
			if((bTrack2.hasCompl) && 							// profile 2 is strand dependent
				(d=calcCorelations(i,i, false, true,false)) >=-10) {	// => <=  valid pair
					storePair(i,d); n_corr++; FgAvCorr+=d;
			}
		}
	}
	//=================================================== Define rank for q-value calculation

	bTrack1.finStatistics(); bTrack2.finStatistics();
	FgAvCorr/=n_corr;

	finOutWig();
	fgHist.normF();
	totCorr=calcCC();
	if(n_corr==0)	xverb("\nno non-zero windows pairs\n",totCorr, FgAvCorr);
	else			xverb("\nCorrelation=%f\naverage Corrrelation=%f\n",totCorr, FgAvCorr);
	errStatus=0;
}

//=========================  Calculate autocorrelation
int resultAutoCorrelation(int from, int to, bool cmpl1, bool cmpl2){
	if(bTrack1.readProfileToArray(xDat,corrScale,from,to,false)==0) return 0;
	if(bTrack2.readProfileToArray(yDat,corrScale,from,to,false)==0) return 0;

	if(corrFunc(xDat,yDat,xyCorr,lProfAuto)==0) return 0;
	int fg=0;
	for(int i=0; i<lProfAuto; i++){
		autoCorrx[i]+=xDat[i];
		autoCorry[i]+=yDat[i];
		Corrxy[i]	+=xyCorr[i];
		if(xDat[i]!=0 && yDat[i]!=0 && xyCorr[i]!=0) fg=1;
	}
	return fg;
}
//============================================= Auto correlations
int nCorrelation=10000;
int lAutoScale=1000;
void resultAutoCorrelation(){
	char b[1024];
	FILE *fil;
	if(lAuto==0) return;
	errStatus="resultAutoCorrelation";
	verb("Autocorrelation...\n");

	corrScale=lAuto*lAutoScale/(binSize*nCorrelation);
	if(corrScale <1) corrScale=1;

	lProfAuto=lAuto*1000/(binSize*corrScale);
	getMem0(xDat,lProfAuto     , "resultAutoCorrelation #1");
	getMem0(yDat,lProfAuto     , "resultAutoCorrelation #2");
	getMem0(xyCorr,lProfAuto   , "resultAutoCorrelation #3");
	getMem0(autoCorrx,lProfAuto, "resultAutoCorrelation #4"); 	zeroMem(autoCorrx,lProfAuto);
	getMem0(autoCorry,lProfAuto, "resultAutoCorrelation #5");	zeroMem(autoCorry,lProfAuto);
	getMem0(Corrxy,lProfAuto   , "resultAutoCorrelation #6");	zeroMem(Corrxy,lProfAuto);
	int from=0, to=lProfAuto;
	int nn=0;
	for(; to< profileLength; from+=lProfAuto, to+=lProfAuto){
		if(nn%1000==0) verb("Autocorr %i/%i\n",from,profileLength);
		int fg=resultAutoCorrelation(from,to,false,false);
		nn+=fg;
	}

	strcat(strcpy(b,outFile),AC_EXT);
	fil=xopen(b,"wt");
	fprintf(fil,"l\tauto1\tauto2\tcorrelation\n");
	for(int i=0; i<lProfAuto/2; i++){
		double k=1./1000*binSize*corrScale*i;
		autoCorrx[i]/=nn; autoCorry[i]/=nn; Corrxy[i]/=nn;
		if(abs(autoCorrx[i]) < 0.001 &&
		   abs(autoCorry[i]) < 0.001 &&
		   abs(Corrxy[i])    < 0.001 ) continue;
		fprintf(fil,"%.4f\t%.5f\t%.5f\t%.5f\n",k,autoCorrx[i],
				autoCorry[i],Corrxy[i]);
	}
	fclose(fil);
	errStatus=0;
}

//================================================================================================
//================================================================================================
//================================================================================================
//================================================================================================
char *resFileName(const char* n1,const char* n2){
	char b[2048];
//	int iw=wSize/1000;
	sprintf(b,"%s~%s",n1,n2);
	return strdup(b);
}
//================================================================== Make name for outfile
char * makeOutFilename(char * prof1, char*prof2){
	char p1Fname[4096], p2Fname[4096], b[4096];
	getFnameWithoutExt(b, prof1);
	if(strchr(b,'~')) sprintf(p1Fname,"(%s)",b);
	else strcpy(p1Fname,b);
	getFnameWithoutExt(b, prof2);
	if(strchr(b,'~')) sprintf(p2Fname,"(%s)",b);
	else strcpy(p2Fname,b);

	sprintf(b,"%s%s",resPath,p1Fname);
	return resFileName(b,p2Fname);
}


int Correlator(){
	Timer timer;
	id=0;	// id is undefined yet
	wProfSize=wSize/binSize;       		// size of widow (profile scale)
	wProfStep=wStep/binSize;       		// window step   (profile scale)
	wProfSize=wSize/binSize;
	LFlankProfSize=flankSize/binSize;
	int ll=nearFactor(2*LFlankProfSize+wProfSize);
	LFlankProfSize=(ll-wProfSize)/2;
	profWithFlanksLength=ll;
	RFlankProfSize=ll-wProfSize-LFlankProfSize;

	//================================================================== print parameters
	verb("========== Parameters ===========\n");
	verb("==         chrom=<%s>\n", chromFile);
	if (pcorProfile != 0) verb("==         pcorProfile=<%s>\n", pcorProfile);
	verb("===        bin=%i\n",binSize);
	verb("==         wSize=%i\n",wSize);
	verb("==         kernelType=%i\n",kernelType);
	verb("==         kernelSigma=%.0f\n",kernelSigma);
	verb("==         nShuffle=%i\n",nShuffle);

	writeLog("Correlations:   wSize=%i   kernelType=%s   kernelSigma=%.0f\n",
			wSize,getKernelType(),kernelSigma);
	//====================================================================== Prepare parameters
	kernelProfSigma=kernelSigma/binSize;   // kernel width ((profile scale)
	kernelProfShift=kernelShift/binSize;   // kernel shift ((profile scale)
	miv.scale(binSize);
	maxNA   =(int)(maxNA0  *wProfSize/100);			// rescale maxNA
	maxZero =(int)(maxZero0*wProfSize/100);			// rescale maxZero
	if(maxZero>=wProfSize) maxZero=wProfSize-1;
	if(maxNA  >=wProfSize) maxNA  =wProfSize-1;
	//===================================================================== generate Kernels
	switch(kernelType){
	case KERN_NORM:
		kern=new NormKernel    (kernelProfShift, kernelProfSigma, profWithFlanksLength); break;
	case KERN_LEFT_EXP:
		kern=new LeftExpKernel (kernelProfShift, kernelProfSigma, profWithFlanksLength); break;
	case KERN_RIGHT_EXP:
		kern=new RightExpKernel(kernelProfShift, kernelProfSigma, profWithFlanksLength); break;
	default: errorExit("Kernel not defined"); break;
	}

	//============ Read Map File
	if(pcorProfile) projTrack.read(pcorProfile);
	if(mapFil     ) {
		mapTrack.read(mapFil);
		mapTrack.makeMapIntervals();
		mapIntervals();
		fillMap();
	}

	int n_cmp=0;
	int nnf=nfiles; if(nnf>1) nnf--;
	wCorrelation.init(profWithFlanksLength);
	getMem0(wCorrelation.datRe,profWithFlanksLength, "Correlator");
	for(int i=0; i<nnf; i++){
		id=(unsigned int)time(0);	// id is undefined yet
		profile1=files[i].fname;
		verb("read profile1...\n");
		bTrack1.read(profile1);					// read byte profiles
		if(pcorProfile) bTrack1.ortProject();
		bTrack1.makeIntervals();
		for(int j=i+1; j<nfiles; j++){
			Timer thisTimer;
			if(files[j].id==files[i].id) continue;
			profile2=files[j].fname;
			outFile=makeOutFilename(profile1, profile2);
			makeId();
			writeLog("  in1=<%s> in2=<%s> out=<%s>\n", profile1, profile2, outFile);

			xverb("in1=\"%s\"\n", profile1);
			xverb("in2=\"%s\"\n", profile2);
			xverb("out=\"%s\"\n", outFile);

			verb("read profile2...\n");
			bTrack2.read(profile2);
			if(pcorProfile) bTrack2.ortProject();
			bTrack2.makeIntervals();
			if(bTrack2.lProf != bTrack1.lProf) errorExit("Incompatible length of profiles");
			if(mapFil!=0 && mapTrack.lProf != bTrack1.lProf) errorExit("Incompatible length of profiles and map");
			//===================================================================== Calculate
			verb("=== OutFile = <%s>\n",outFile);
			correlation.init();
			bgcorrelation.init();
			clearChromosomes();
			if(corrOnly==0){
				writeLog("Background\n");
				distrBkg();						// Make background distribution
			}
			writeLog("Foreground\n");
			distrCorr();					// Calculate correlations
			writeLog("Correlations -> Done\n");
			printCorrelations();			// write correlations
			printStat();					// write report
			resultAutoCorrelation();		// write autocorrelation
			if(RScriptFg) {
//				writeLog("Write R\n");
				printR();
				printRreport();
				printRmd();
			}
			n_cmp++;
			clear();
			bTrack2.clear();
			writeLog("<%s> => Done  time=%s\n",outFile,thisTimer.getTime());
		}
	bTrack1.clear();
	writeLog("====== DONE ======\n");
	}
    verb("***   calculation time for %i comparisons = %s\n",n_cmp, timer.getTime());

	//====================================================================== Read Data
	return 0;
}
