/*  read sil data from .eve files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include "../rayt/ray.h"

char *outputBaseName = "testing" ;
char *rootName = "/data/bergth/4D/sk1/1998/apr" ;
/* selection parameters for phases */
double maxResidual = 1.1 ;
double maxDistance = 85.0 ;

int month2number( char *monthName)
{
	static char *mn[12] = { "jan","feb","mar","apr","may","jun","jul","aug","sep","oct","nov","dec" } ;
	int i ;
	for( i = 0 ; i < 12 ; i++) {
		if( 0==strcmp( monthName, mn[i] ) ) return(i) ;
	}
	return -1 ;
}
void writePhase( Phase *pp )
{
	static FILE *ff ;
	char fname[200] ;
	if( pp == NULL ) {
		fclose(ff) ;
		return ;
	}
	if( NULL == ff ) {
		sprintf(fname,"%s.phase",outputBaseName) ;
		ff = fopen(fname,"w") ;
	}
	fwrite(pp, sizeof(Phase),1,ff) ;
}
void writeEvent( Event *pp )
{
	static FILE *ff ;
	char fname[200] ;
	if( pp == NULL ) {
		fclose(ff) ;
		return ;
	}
	if( NULL == ff ) {
		sprintf(fname,"%s.event",outputBaseName) ;
		ff = fopen(fname,"w") ;
	}
	fwrite(pp, sizeof(Event),1,ff) ;
}
void readEFile( char *ename ) 
{
	FILE *efile ;
	char *bp, *cp ;
	static struct tm tm ;
	time_t ttEvent,ttPhase ;
	int indexms, indexSec ;
	static size_t n=100 ;
	int len,j,iPhase ;
	char line[100], buffer[100] ;
	double eventSec, sourceTime,phaseSec,phaseRes,eventFrac,phaseFrac,dist,ttime ;
	static Event ev ;
	static Phase phase ;
	Station *sp ;
	cp=strncpy(buffer,ename,100) ;
	if( shLogLevel > 4 )  printf("%s",ename) ; 
	while(*cp){ 
		if ((*cp == '/') ||(*cp == ':')) {*cp = 0 ; bp = cp+1 ; }
		cp++ ;
	}
	ename[cp-buffer-1] = 0 ;
	indexms = atoi(bp) ;
	indexSec = atoi(bp-3) ;
	efile = fopen(ename,"r") ;
	cp = fgets(line,n,efile) ;
/*
	j = atoi(line+60) ;
	tm.tm_sec = indexSec ;
	j = j / 100 ;
	tm.tm_min = j%100 ;
	tm.tm_hour = j/100 ;
	tm.tm_mday=atoi(bp-18) ;
	tm.tm_mon=month2number(bp-22) ;
	tm.tm_year=atoi(bp-27)-1900 ;
	ev.index = (((1900+tm.tm_year)*100+1+tm.tm_mon)*100+tm.tm_mday)*100+tm.tm_hour ;
	ev.index = ((ev.index*100+tm.tm_min)*100+tm.tm_sec)*1000+indexms ; 
	ttIndex = mktime(&tm) ;
	cp = asctime(&tm) ;
	if( shLogLevel > 4 ) {
		printf("%ld ",ev.index) ;
		printf("%s %s ",cp,bp);
		printf(" ename =%s_\n",ename) ;
	}
*/
	cp = fgets(line,n,efile) ;
	tm.tm_year = atoi(line+12) ;
	if( tm.tm_year < 70 ) tm.tm_year += 100 ;
	tm.tm_mon = atoi(line+15) - 1 ;
	tm.tm_mday= atoi(line+18) ;
	tm.tm_hour= atoi(line+23) ;
	tm.tm_min = atoi(line+27) ;
	eventSec = atof(line+31) ;
	tm.tm_sec = trunc(eventSec) ; 
	eventFrac = eventSec - tm.tm_sec ;
	indexms = round(1000.0 * eventFrac) ;
	ev.index = (((1900+tm.tm_year)*100+1+tm.tm_mon)*100+tm.tm_mday)*100+tm.tm_hour ;
	ev.index = ((ev.index*100+tm.tm_min)*100+tm.tm_sec)*1000+indexms ; 
	ttEvent = mktime(&tm) ;
/*	sourceTime = (ttEvent-ttIndex) + eventSec ;
	if( shLogLevel > 4 ) 
		printf("eventSec = %f sourceTime = %f \n",eventSec,sourceTime) ; */
	cp = fgets(line,n,efile) ; ev.lat = atof(line+10) ;
	cp = fgets(line,n,efile) ; ev.lon = atof(line+10) ;
	fgets(line,n,efile) ; ev.depth = atof(line+12) ;
	ev.time = eventFrac  ;
	if( shLogLevel > 4 ) 
		printf("lat lon depth time: %f %f %f %f\n",ev.lat,ev.lon,ev.depth,ev.time) ;
 	writeEvent(&ev) ; 
	cp = fgets(line,n,efile) ;
	cp = fgets(line,n,efile) ;
	iPhase = 1 ;
	while (*cp == ' ' ) {
		tm.tm_hour = atoi(line+6) ;
		tm.tm_min  = atoi(line+9) ;
		phaseSec = atof(line+12);
		phaseRes = atof(line+18);
		phase.weight = atof(line+24) ;
		tm.tm_sec =  trunc(phaseSec) ;
		phaseFrac = phaseSec - tm.tm_sec ;
		ttPhase = mktime(&tm) ;
		dist = atof(line+31) ;
		ttime = ttPhase-ttEvent+phaseFrac-eventFrac ;
		line[4] = 0 ;
		sp = lookUpStation(line+1) ;
		phase.index = ev.index ;
		phase.pTime = ttime ;
		if( shLogLevel > 6 )
			printf("index = %ld ttPhase = %d ttIndex = %d phaseFrac = %10.3f pTime = %8.3f\n",
			ev.index,ttPhase,ttEvent,phaseFrac,phase.pTime) ;
		phase.residual = phaseRes ;
		phase.type = (255-32) & line[5] ;   /* upper case */
		phase.iPhase = iPhase++ ;
		strncpy(phase.station,line+1,3) ;
		phase.station[3] = 0 ;
		if( shLogLevel > 4 ) 
			printf(" hour,min sec ttime phaseRes %d %d %f %2d %f %f %8.3f %8.3f %s\n",
			tm.tm_hour,tm.tm_min,phaseSec,ttPhase-ttEvent,dist,ttime,
			phaseRes,dist/ttime,sp->name) ;
		if( fabs(phaseRes) < maxResidual ) 
			if( dist < maxDistance ) 
				if((phase.weight) != 0.0 ) writePhase(&phase) ;
		cp = fgets(line,n,efile) ;
	}
	fclose(efile) ;
}
void getEList(  ) /* obsolete */
{
	FILE *efiles ;
	char cmd[100] ;
	char *line ;	
	size_t n ;
	int len ;
	sprintf(cmd,"find %s -name *.eve ",rootName) ;
	printf("%s\n",cmd) ;
	efiles = popen(cmd,"r") ;
	while ( 0 <   (len = getline(&line,&n,efiles))) { 
		readEFile(line) ;
/*		printf("%d %d _ %s",len, n,line) ; */
	} while (len > 0) ;
	writeEvent(NULL) ; /* close output files */
	writePhase(NULL) ;
	
}
time_t index2utime( long long index )
{
	int s,m,h,d,mo,y ;
	time_t tt ;
	struct tm tm ;
	index /= 1000 ;
	tm.tm_sec=index % 100 ;
	index /= 100 ;
	tm.tm_min=index % 100 ;
	index /= 100 ;
	tm.tm_hour=index % 100 ;
	index /= 100 ;
	tm.tm_mday=index % 100 ;
	index /= 100 ;
	tm.tm_mon=index % 100 ;
	tm.tm_year = index/100-1900 ;
	tt = mktime(&tm) ;
/*	printf("%s",ctime(&tt)) ; */
	return tt ;
}
int getPhases( Event *ep, char *evePath )
{
	static Phase phase ;
	FILE *eveFile ;
	time_t tt ;
	char line[300]  ;
	size_t n ;
	int ms,hour,min,sec,j,len,skip ;
	int pHour,pMin,iPhase ;
	double pSec, pTime,dist ;
	eveFile = fopen(evePath,"r") ;
	if( NULL == eveFile) return 0 ;
	tt = index2utime( ep->index ) ;
	ms = ep->index % 1000 ;
	sec = tt%60 ;
	j=tt/60 ;
	min = j%60 ;
	hour = (j/60)%24 ;
/*	printf("%2d %2d %2d %3d\n",hour,min,sec,ms) ;  */
	skip = 6 ;
	iPhase = 1 ;
	while ( fgets(line,299,eveFile)) 
	{
		if( skip-- > 0 ) continue ;
		if( *line == '0') break ;
		phase.index = ep->index ;
		pHour = atoi(line+7) ;
		pMin = atoi(line+10) ;
		pSec = atof(line+13) ;
		dist = atof(line+31) ;
		pSec += (pMin-min)*60.0 +(pHour-hour)*3600.0 - sec ;
		if(pSec < 0 ) pSec += 3600.0*24.0 ;
		phase.pTime = pSec - ms*0.001 ;
		phase.residual = atof(line+18) ;
		phase.weight = atof(line+24) ;
		phase.iPhase = iPhase++ ;
		phase.type =  (255-32) & line[5] ;   /* upper case */
		strncpy(phase.station,line+1,3) ;
		phase.station[3] = 0 ;
		if( shLogLevel > 6 ) {
			printf("%s",line) ;
			printf("%d %d %12.3f\n",pHour,pMin,pSec) ;
		}
		if( fabs(phase.residual) < maxResidual ) 
			if( dist < maxDistance ) 
				if((phase.weight) != 0.0 ) writePhase(&phase) ;
	}	 
	fclose(eveFile) ;
	return 1 ;
}
void getLibFile( char *path )
{
	FILE *libFile ;
	char buff[500] ;
	char evePath[200] ;
	int len,j ;
	size_t n ;
	char *line ;
	static Event ev ;
	static Phase phase ;
	int ymd ;
	double hms ;
	static long long indexScale = 1000000000 ;
	long long hmsll ;
	static char eveName[50] ;
	len = strlen(path) ;
	strncpy(buff,path,499) ;
	buff[len-1] = 0 ;
	libFile = fopen(buff,"r") ;
	if( shLogLevel > 6 ) printf("%x=%s=\n",libFile,buff ) ;
	line = NULL ;
	while( 5 < (len = getline(&line,&n,libFile))){
		if(line[18]==' ') line[18] = '0' ;
		if( shLogLevel > 6 ) printf("%d %s",len, line ) ;
		ymd = atoi(line+5) ;
		hms = atof(line+14) ;
		hmsll = hms*1000.0 ;
		ev.index = ymd*indexScale + hmsll ;
		ev.lat = atof(line+25) ;
		ev.lon = atof(line+34) ;
		ev.depth = atof(line+44) ;
		ev.time = 0.0 ;
		strncpy(eveName,line+61,31) ;
/*		printf("ymd=%d hms=%12.4f ev.index=%lld %f %f %f %s.eve\n",
			ymd,hms,ev.index,ev.lat,ev.lon,ev.depth,eveName) ; */
		strncpy(buff,path,100) ;
		j = strlen(path)-12;
		buff[j]=0 ;
		sprintf(evePath,"%s%s.eve",buff,eveName+12) ;
		if (getPhases( &ev, evePath ) ) writeEvent(&ev) ;

	}
	fclose(libFile) ;
	
}
void getLibList()
{
	FILE *libFiles ;
	char cmd[400] ;
	char *line ;
	size_t n ;
	int len ;
	sprintf(cmd,"find %s -name events.lib", rootName) ;
/*	printf("%s\n",cmd) ; */
	libFiles = popen(cmd,"r") ;
	line = NULL ;
	while( 5 < ( len = getline(&line,&n, libFiles))) {
/*		printf("%d %d %s",len,n,line) ;  */
		if(line) {
			getLibFile(line) ;  
			free(line) ;
		}
		line = NULL ;
	}
	fclose(libFiles) ;
/*	printf("exit getLibList\n") ; */
}
int main(int ac , char **av)
{
	int cc ;
	extern char *optarg ;
	shLogLevel = 2 ;
	while( EOF != ( cc = getopt(ac,av,"l:b:o:d:r:"))) {
		switch(cc) {
		case 'l' : shLogLevel = atoi(optarg) ; break ;
		case 'b' : rootName = optarg ; break ;
		case 'o' : outputBaseName = optarg ; break ;
		case 'd' : maxDistance = atof(optarg ) ; break ;
		case 'r' : maxResidual = atof(optarg ) ; break ;
	}}
	getLibList();
	exit(1) ;
}
