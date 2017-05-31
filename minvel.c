/*
	Code to imporive velocity functions
	(c) Einar Kjartansson 2017

*/
#include <stdlib.h>
#include <stdio.h>
#define _GNU_SOURCE
#include <fenv.h>
#include <math.h>
#include "ray.h"


Phase *phases, *eventPhases, *ip ;
Solution *location, *lp, *workingLoc ;
char *phaseFile = "/data/bergth/landsveit_eik/1991-2000/phase.dat" ;
char *solFile = "/data/bergth/landsveit_eik/1991-2000/ctloc" ;
int nSolutions, nWorking ;
char *pModel = "test.vel" ;
char *sModel = "sils.vel" ;
extern VelModel mp,ms ;

void getData() 
{
	int np,ns,iSol ;
	long long index ;
	np = readPhases(phaseFile,&phases ) ;
	ns =  readCtloc(solFile,&location ) ;
	eventPhases = calloc(ns,sizeof(Phase)) ;
	printf("np = %d ns= %d\n",np,ns) ;
	ip = phases ;
	for( iSol = 0 ; iSol < ns ; iSol++) {	
		lp =  location +iSol ;
		index = lp->index ;
		while( ip->index < index ) ip++ ;
		lp->phase = ip ;
		while( ip->index == index ) ip++ ;
		lp->nPhase = ip - lp->phase ;
		if( shLogLevel > 4 )printf("iSol = %3d  nPhase=%2d\n",iSol,lp->nPhase) ;
	}
	nSolutions = ns ;
}
double processVel( Solution *sol, int n )
{
	Solution *lp ;
	Phase *pp ;
	int i ;
	double sst ;
	sst = 0 ;
	vFInitFromMemory() ;
	for( i = 0 ; i < n ; i++ ) {
		lp = sol + i ;
		pp = lp->phase ;
		locate( lp,pp) ;
		sst +=  ( 0.5 * lp->stdP + 0.3 * lp->stdS ) ;
	}
	return sqrt(sst/n) ;
}
void pass1()
{
	Solution *lp,*op ;
	Phase *pp ;
	int ii,sumi ;
	initVelModel(20,&mp) ; readVelModel(pModel,&mp) ; 
	initVelModel(20,&ms) ; readVelModel(sModel,&ms) ; 
	rayTrace = 0  ;
	sumi = 0 ;
/* 	
	if( rayTrace == 0 ) vFInit() ;  
*/
	if( rayTrace == 0 ) vFInitFromMemory() ; 
	for( ii = 0 ; ii < nSolutions ; ii++) {
		lp = location + ii ;
		lp->nIter = 1 ;
		pp = lp->phase ;
		locate( lp,pp) ;	
		if(shLogLevel > 4 )
		   printf("nP=%3d nS=%3d nIter=%3d stdP=%10.4f stdS=%10.4f length=%10.4f\n",
			lp->nP,lp->nS,lp->nIter,lp->stdP, lp->stdS,lp->length) ;
		sumi += lp->nIter ;
	}
/* select solutions for further use */
	workingLoc = (Solution*) malloc(nSolutions* sizeof(Solution)) ;
	op = workingLoc ;
	for( ii = 0 ; ii < nSolutions ; ii++) {
		lp = location + ii ;
		if( 	(lp->nIter < 12 ) &&
			(lp->stdP < 0.2 ) &&
			(lp->stdS < 0.3 )) *op++ = *lp ;
	}
	nWorking = op - workingLoc ;
	printf("%d iterations, nSolutions = %d nWorking = %d\n",sumi, nSolutions,nWorking) ;
	
}
void test3()
{
	int i ;
	double vv ;
	printf("ProcessVel returns %10.5f\n", processVel(workingLoc,nWorking)) ;
	for( i = 0 ; i < 5 ; i++) {
		vv = mp.v[i] ;
		mp.v[i] += 0.02 ;
		printf("ProcessVel %d returns %10.5f\n",i, processVel(workingLoc,nWorking))  ;
		mp.v[i] = vv ;
	}
}
main()
{
	
	shLogLevel = 2 ;
	getData() ;
	pass1() ;	
	test3() ;
}
