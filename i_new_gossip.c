//MODELLO GOSSIP ISPIRATO A ESPERIMENTO VALENCIA - POSSIBILITÀ DI ERRORE NEL PASSAGGIO DI INFORMAZIONI
//ERRORI UNIDIREZIONALI (buoni segnalati come cattivi, mai viceversa), UGUALE PESO DI ESPERIENZA DIRETTA E GOSSIP
//GOSSIP SOLO TRA AGENTI CHE SI STIMANO (immagine reciproca non negativa)
#include<stdio.h>   
#include<stddef.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include"ran2.h"

#define M 100                  //numero di gruppi
#define G 4                   //taglia dei gruppi
#define F 0.35                 //frazione di membri del gruppo fissi. Il resto assegnati random

#define B0 0.85               // sinergia per membro del gruppo (se commentato vedi sotto)
#define C 1.0                 //costo della cooperazione
#define probab 0.9            //probabilità che l'agente con meno fitness imiti quello con maggiore, e non viceversa
#define X0 0.5                //frazione iniziale di cooperatori
#define P_ERR 0.0            //probabilità errore di trasmissione info (cattivo al posto di buono)

#define PHASES 5              //numero di fasi gioco+gossip dentro una generazione
#define T_MAX 100             //tempo massimo
#define S 2000                //numero di realizzazioni su cui mediare

int main(){

  FILE *fa,*fb,*fc;
  int i,i2,j,k,N,GG,r,t,tau,p,h,qq,tries,counter,sort0,sort,vbl,iter;
  unsigned long int tt;
  int m[G],comp[M][G];
  double B,extr,pot,s_m,sort1,qwe;
  int *str,*str0,*verifica,*v;
  double *fitn,**s,*ms;
  unsigned long int *ct,*mosse,*coop;
  time_t Time;  
  long a2,*a;

  a2=-(unsigned)time(&Time);//-630598617;//-2573150310;//
  a=&a2;

  B=1.0*B0*G;   //B fattore moltiplicativo (sinergia)
  GG=F*G;      //numero di membri fissi di ognio gruppo
  N=M*GG;      //taglia totale del sistema
  tries=1+N*10;   //tentativi prima di accettare un membro con immagine negativa

  s=(double**)calloc(N,sizeof(double*));//immagine degli altri agenti
  for (i=0;i<N;i++)
    s[i]=(double*)calloc(N,sizeof(double));

  str=(int*)calloc(N,sizeof(int)); //strategia degli agenti, 1=cooperation, 0=defection
  str0=(int*)calloc(N,sizeof(int)); //vettore ausiliario della strategia degli agenti per la riproduzione
  v=(int*)calloc(N,sizeof(int));//vettore ausiliario per estrazione gruppi
  verifica=(int*)calloc(N,sizeof(int));//altro vettore ausiliario per estrazione gruppi
  fitn=(double*)calloc(N,sizeof(double));//fitness degli agenti
  ct=(unsigned long int*)calloc(T_MAX+1,sizeof(unsigned long int));//numero di volte in cui un giocatore ha cooperato
  mosse=(unsigned long int*)calloc(T_MAX+1,sizeof(unsigned long int));//numero di volte in cui un giocatore ha giocato
  coop=(unsigned long int*)calloc(T_MAX+1,sizeof(unsigned long int));//numero di cooperatori al tempo t
  ms=(double*)calloc(T_MAX+1,sizeof(double));//immagine media al tempo t

  for (t=0;t<=T_MAX;t++){
    ct[t]=0;
    mosse[t]=0;
    coop[t]=0;
    ms[t]=0.0;
  }

  for (r=0;r<S;r++){

    //INIZIALIZZAZIONI

    for (i=0;i<N;i++){
      extr=1.0*ran2(a);
      if (extr<X0){
	str[i]=1;
	coop[0]++;
      }
      else
	str[i]=0;
    }

    for (t=0;t<T_MAX;t++){

      for (i=0;i<N;i++){
	fitn[i]=0.0;
	for (j=0;j<N;j++)
	  s[i][j]=0.0;
      }

      for (tau=0;tau<PHASES;tau++){
	//FASE PGG
	
	//CREAZIONE GRUPPI
	for (j=0;j<M;j++){
	  for (p=0;p<G;p++)
	    comp[j][p]=-10;
	}
	h=0;
	for (j=0;j<M;j++){
	  for (p=0;p<GG;p++){
	    comp[j][p]=h;
	    h++;
	  }
	}
	for (j=0;j<M;j++){ //qui aggiungo i membri supplementari
	  for(qq=0;qq<N;qq++)
	    verifica[qq]=0;
	  for (p=GG;p<G;p++){
	    counter=0;
	  paperinik:
	    sort0=N*ran2(a);
	    vbl=sort0/GG;
	    if (vbl==j || verifica[sort0]>0)
	      goto paperinik;
	    else {
	      s_m=0.0;
	      for (i=0;i<GG;i++)
		s_m=s_m+1.0*s[comp[j][i]][sort0];
	      if (s_m<0){
		counter++;
		if (counter<=tries)
		  goto paperinik;
		else {
		  comp[j][p]=sort0;
		  verifica[sort0]=1;
		}
	      }
	      else {
		comp[j][p]=sort0;
		verifica[sort0]=1;
	      }
	    }
	  }
	}
	//FINE CREAZIONE GRUPPI
	//////////////////////////////////////////////////////////////////////////////////////////

	//GIOCO PROPRIAMENTE DETTO
	for (j=0;j<M;j++){
	  pot=0.0;
	  for (i=0;i<G;i++){
	    if (str[comp[j][i]]==1){
	      ct[t]++;
	      mosse[t]++;
	      pot=pot+1.0;
	      fitn[comp[j][i]]=fitn[comp[j][i]]-1.0;
	      for (i2=0;i2<G;i2++){
		if (i2!=i)
		  s[comp[j][i2]][comp[j][i]]=s[comp[j][i2]][comp[j][i]]+1.0;
	      }
	    }
	    else {
	      mosse[t]++;
	      for (i2=0;i2<G;i2++){
		if (i2!=i)
		  s[comp[j][i2]][comp[j][i]]=s[comp[j][i2]][comp[j][i]]-1.0;
	      }
	    }
	  }
	  pot=1.0*B*pot;
	  for (i=0;i<G;i++)
	    fitn[comp[j][i]]=fitn[comp[j][i]]+1.0*pot/G;
	}
	//FINE PGG

	//FASE GOSSIP
	for (iter=0;iter<100*N;iter++){
	  
	  //ESTRAZIONE GIOCATORI
	    for (j=0;j<N;j++)
	      v[j]=j;
	    for (j=0;j<G;j++){
	    paperinika:
	      m[j]=N*ran2(a);
	      if (v[m[j]]==-10)
		goto paperinika;
	      else
		v[m[j]]=-10;
	    } //FINE ESTRAZIONE GIOCATORI*/

	    //QUI GOSSIP PROPRIAMENTE DETTO
	    i=N*ran2(a);
	tuba_mascherata:
	    j=N*ran2(a);
	    if (i==j)
	      goto tuba_mascherata;
	paperbat:
	    sort=N*ran2(a);
	    if (sort==i || sort==j)
	      goto paperbat;
	    if (s[i][j]>=0 && s[j][i]>=0) {
	      if (s[i][sort]>=0){
		qwe=1.0*ran2(a);
		if (qwe<P_ERR)
		  s[j][sort]=(1.0*s[j][sort]-1.0*s[i][sort])/2.0;
		else
		  s[j][sort]=(1.0*s[j][sort]+1.0*s[i][sort])/2.0;
	      }
	      else
		s[j][sort]=(1.0*s[j][sort]+1.0*s[i][sort])/2.0;
	    }
	    
	} //FINE GOSSIP*/

      } //fine ciclo su tau

      for (i=0;i<N;i++){
	for (j=0;j<N;j++){
	  if (j!=i)
	    ms[t+1]=ms[t+1]+1.0*s[i][j];
	}
      }

      //RIPRODUZIONE
      for (i2=0;i2<N;i2++){
	  i=N*ran2(a);
	superpippo:
	  j=N*ran2(a);
	  if (i==j)
	    goto superpippo;
	  sort1=1.0*ran2(a);
	  if (sort1<probab){
	    if (fitn[j]>fitn[i])
	      str0[i2]=str[j];
	    else
	      str0[i2]=str[i];
	  }
	  else {
	    if (fitn[j]>fitn[i])
	      str0[i2]=str[i];
	    else
	      str0[i2]=str[j];
	  }
      }
      for (i2=0;i2<N;i2++)
	str[i2]=str0[i2];
      //FINE RIPRODUZIONE

      for (i2=0;i2<N;i2++){
	if (str[i2]==1)
	  coop[t+1]++;
      }
      
    }

                                           //plottigat
    if ((fa=fopen("don_freq_M100G04F0.35B0.85_phases05Perr0.00.dat","w"))==NULL){
      printf ("Impossibile aprire il file-a \n");
      exit (1);
    }
    if ((fb=fopen("coopfreq_M100G04F0.35B0.85_phases05Perr0.00.dat","w"))==NULL){
      printf ("Impossibile aprire il file-b \n");
      exit (1);
    }
    if ((fc=fopen("av_image_M100G04F0.35B0.85_phases05Perr0.00.dat","w"))==NULL){
      printf ("Impossibile aprire il file-c \n");
      exit (1);
    }

    fprintf(fa,"#realizzazioni=%d\n\n",r+1);
    for (tt=0;tt<T_MAX;tt++)
      fprintf(fa,"%lu\t%g\n",tt+1,1.0*(double)ct[tt]/(mosse[tt]*1.0));

    fclose(fa);
    
    fprintf(fb,"#realizzazioni=%d\n\n",r+1);
    for (tt=0;tt<=T_MAX;tt++)
      fprintf(fb,"%lu\t%g\n",tt,1.0*(double)coop[tt]/(1.0*(r+1)*N));

    fclose(fb);
    
    fprintf(fc,"#realizzazioni=%d\n\n",r+1);
    for (tt=0;tt<=T_MAX;tt++)
      fprintf(fc,"%lu\t%g\n",tt,1.0*ms[tt]/(1.0*(r+1)*N*(N-1)));

    fclose(fc);

  }

}
