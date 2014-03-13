// Programme de simulation des évènements dans le scintillateur
// H. Halloin - 24/02/2014

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <time.h>


// Nombre max de dépots successifs d'énergie pour une seule particule
#define  NMaxDep 1000

// Nombre d'evts simulés 
const unsigned long int Nevts = 1000000;

// Nom du fichier d'évènements simulés
const char EvtsFileName[] = "EventsScintillator.txt";

// Paramètres des spectres incidents
// gammas
const double Agam    = 1e4;   // ph/(s.m2.MeV)
const double E0gam   = 1.0;   // MeV
const double Ggam    = 1.2;   // indice spectral
const double EminGam = 0.02;  // MeV
const double EmaxGam = 10.0;  // MeV

// électrons
const double Aele  = 1e4;   // ph/(s.m2.MeV)
const double E0ele = 1.0;   // MeV
const double Gele  = 2.0;   // indice spectral
const double EminEle = 1.0;  // MeV
const double EmaxEle = 20.0; // MeV

// facteur de bruit de fond sur le flux incident
const double BgndFact = 0.1;


// Taille d'un pixel
const double SizePix = 4.0;  // mm

// Profondeur du détecteur
const double DepthDet = 50.0; // mm

// Coef d'absorption linéique pour le LaBr3 en mm^-1. MeV3
const double AlphaPhotLaBr3 = 6.86e-4;

// Coef d'interaction Compton pour le LaBr3 en mm^-1
const double AlphaComptLaBr3 = 6.8e-2; 

// Densité du Labr3
const double RhoLaBr3 = 5.08;

// Densité du BC412
const double RhoBC412 = 1.032;


// Gamme de mesure Easiroc [MeV]
const double EminEasiroc = 0.02;
const double EmaxEasiroc = 2.0;

// facteur haut/bas gains easiroc
const double GFactEasiroc = 10;
const double EADCThreshEasiroc = EminEasiroc + (EmaxEasiroc-EminEasiroc)/(GFactEasiroc+1.0);

// Nombre de bits de codage Easiroc
const int NbBitsEasiroc = 14;
const unsigned int MaxValEasiroc = (1<<NbBitsEasiroc)-1;

// Granularité de l'horloge Easiroc
const double TimeGranularityEasiroc = 1e-10;

// Décalage de temps (6 mois)
const double TimeOffset = 86400*365/2.0;

// Prototypes Fonctions utiles
double GetRandUniform();
double GetPowerLawSpectrumFlux(double Emin, double Emax, double A, double E0, double G, double S);
double GetPowerLawSpectrumEnergy(double Emin, double Emax, double G);
double GetExpDeviate(double Rate);
double PropagPart(double Pos[3], double Vect[3], double lambda);
double InterSec(double Pos[3], double Vect[3], double HalfSizeBlock[3]);
double KNfact(double alpha);
double GetKNAngle(double Ein);
int InteractGamma(double PosPart[3], double VecPart[3], double *Epart,  double HalfSizeBlock[3], double AlphaPhot, double AlphaCompt);
double dSBCdE(double E);
double DistToEvt(double x, double y, double z, double DistSq[], int NBlock);

// Programme principal
int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    
    double SurfLaBr3 = (4*2*SizePix*DepthDet+2*2*SizePix*2*SizePix)*1e-6;
    double SurfBC412 = (4*4*SizePix*DepthDet+2*4*SizePix*4*SizePix)*1e-6;
    
    double TimeEvt;
    unsigned char EvtKind;
    
    double Ngam, Nele, Nbgnd, Ntot;
    
    double y;
    
    unsigned long int il, jl;
    int i,j, iBlock;
    int status;
    
    double PosInc[3], VecInc[3], Einc, Efin, PosCur[3], VecCur[3], Ecur, Tmp, Etot;
    double MatEn[16], DistSq[4];
    unsigned int MatReadings[16], ADCRange[16], Multiplicity;
    
    double PosEvt[3*NMaxDep], VecEvt[3*NMaxDep], EdepEvt[NMaxDep];
    double HalfSizeBlockLaBr3[3], HalfSizeBlockBC412[3];
    long Ndep;
    double lExt, lStep, ltot, Norm;
    double xEvt, yEvt, zEvt, dTotSq;
    
    unsigned long NRec, TimeTag;
    
    time_t tNow = time(NULL);
    
    FILE *EvtsFile;
    
    // Init random number generation
    srandom(time(NULL)+pid);
    
    fprintf(stdout,"Programme %s (%d)\n\n", argv[0], pid);

    // Flux de particules incidentes (gamma, electrons, bruit de fond)
    Ngam = GetPowerLawSpectrumFlux(EminGam, EmaxGam, Agam, E0gam, Ggam, SurfLaBr3);
    Nele = GetPowerLawSpectrumFlux(EminEle, EmaxEle, Aele, E0ele, Gele, SurfBC412);
    Nbgnd = BgndFact*(Ngam+Nele);
    Ntot = Ngam + Nele + Nbgnd;

    fprintf(stdout,"Ngam = %.5f ph/s Ngam = %.5f e/s Nbgnd = %.5f evts/s \n",Ngam,Nele,Nbgnd);

    // Tailles des scintillateurs LaBr3 et BC412
    HalfSizeBlockLaBr3[0] = SizePix;
    HalfSizeBlockLaBr3[1] = SizePix;
    HalfSizeBlockLaBr3[2] = DepthDet/2.0;
    
    HalfSizeBlockBC412[0] = 2*SizePix;
    HalfSizeBlockBC412[1] = 2*SizePix;
    HalfSizeBlockBC412[2] = DepthDet/2.0;
    
    // Ouverture du fichier d'evènements
    EvtsFile = fopen(EvtsFileName,"w");
    
    // Ecriture de l'en-tête
    fprintf(EvtsFile,"# Fichier %s \n",EvtsFileName);
    fprintf(EvtsFile,"# Date :  %s",ctime(&tNow));
    fprintf(EvtsFile,"# Nombre d'evenements simules : %lu\n",Nevts);
    fprintf(EvtsFile,"# Description des colonnes :\n");
    i=1;
    fprintf(EvtsFile,"# Colonne %2d : Numero de l'evenement enregistre\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Numero de l'evenement simule\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Temps [s]\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Type d'evenement [0=photon, 1=electron, 2=bruit de fond]\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Energie de la particule incidente (<0 si bruit de fond) [MeV]\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Coordonnee X d'entree de la particule incidente (=0 si bruit de fond)\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Coordonnee Y d'entree de la particule incidente (=0 si bruit de fond)\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Coordonnee Z d'entree de la particule incidente (=0 si bruit de fond)\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Coordonnee X du vecteur d'entree de la particule incidente (=0 si bruit de fond)\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Coordonnee Y du vecteur d'entree de la particule incidente (=0 si bruit de fond)\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Coordonnee Z du vecteur d'entree de la particule incidente (=0 si bruit de fond)\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Multiplicite de l'evenement enregistre\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Energie recoltee par le detecteur [MeV]\n",i++);
    fprintf(EvtsFile,"# Colonne %2d : Mot de temps sur 60 bits avec une resolution de %.2e s (en hexadecimal)\n",i++,TimeGranularityEasiroc);
    
    for (j=0;j<16;j++) {
        fprintf(EvtsFile,"# Colonne %2d : Statut de l'ADC %2d (-1: evenement sous le seuil de detection, 0: ADC basses energies, 1: ADC hautes energies, 2: saturation)\n",i++,j+1);
        fprintf(EvtsFile,"# Colonne %2d : Valeur lue de l'ADC %2d (14 bits effectifs) en hexadecimal\n",i++,j+1);
    }
    
    
    // Boucle sur tous les evts simulés
    TimeEvt = 0.0;
    NRec = 0;
    for (il=0;il<Nevts;il++) {
    
        // Calcule le temps de l'evenement
        TimeEvt += GetExpDeviate(Ntot);

        // determine la nature de l'evenement
        y = Ntot*GetRandUniform();
        if ( y < Ngam ) EvtKind = 0;
        else if ( y < Ngam+Nele ) EvtKind = 1;
        else EvtKind = 2;

        
        //(stdout,"\n%ld : Tps = %.6f s ; %d   :  ",il+1, TimeEvt, EvtKind);
        //fflush(stdout);

        // Calcul des position(s) et energie(s) de l'evt en fonction de sa nature
        
        // Calcul d'un vecteur (unitaire) isotrope provenant du 1/2 espace supérieur
        VecInc[1] = GetRandUniform();
        y = GetRandUniform();
        VecInc[0] = sqrt(1-VecInc[1]*VecInc[1])*cos(2*M_PI*y);
        VecInc[2] = sqrt(1-VecInc[1]*VecInc[1])*sin(2*M_PI*y);
        
        switch (EvtKind) {
            case 0 :
                // gamma
                Einc = GetPowerLawSpectrumEnergy(EminGam, EmaxGam, Ggam);
                PosInc[0] = 2*SizePix*(GetRandUniform()-0.5);
                PosInc[1] = -SizePix;
                PosInc[2] = DepthDet*(GetRandUniform()-0.5);
                
                Ecur = Einc;
                for (i=0;i<3;i++) {
                    PosCur[i] = PosInc[i];
                    VecCur[i] = VecInc[i];
                }
                
                Ndep = 0;
                Efin = Ecur;
                Etot = 0.0;
                do {
                    status = InteractGamma(PosCur, VecCur, &Efin,  HalfSizeBlockLaBr3, AlphaPhotLaBr3, AlphaComptLaBr3);
                    
                    if ( status ) {
                        for (i=0;i<3;i++) {
                            PosEvt[Ndep*3+i] = PosCur[i];
                            VecEvt[Ndep*3+i] = VecCur[i];
                        }
                        EdepEvt[Ndep] = Ecur - Efin;
                        Etot += EdepEvt[Ndep];
                        Ndep++;
                    }
                    Ecur = Efin;
                    
                }while( status && Ecur>0.02 && Ndep<NMaxDep );
                
                //fprintf(stdout, " %.3f <-> %.3f", Einc, Etot);
                break;
                
            case 1 :
                // electron
                Einc = GetPowerLawSpectrumEnergy(EminEle, EmaxEle, Gele);
                PosInc[0] = 4*SizePix*(GetRandUniform()-0.5);
                PosInc[1] = -2*SizePix;
                PosInc[2] = DepthDet*(GetRandUniform()-0.5);
                
                Ecur = Einc;
                for (i=0;i<3;i++) {
                    PosCur[i] = PosInc[i];
                    VecCur[i] = VecInc[i];
                }
                
                do {
                    // Propagation d'un petit pas pour passer à l'intérieur du bloc
                    PropagPart(PosCur, VecCur, 1e-6);
                    
                    
                    // Longueur d'interaction dans le bloc externe
                    lExt = InterSec(PosCur, VecCur, HalfSizeBlockBC412);
                    if (lExt <= 0 ) continue;
                    
                    
                    // Propagation suivant la trace
                    lStep = 3e-1;
                    ltot = lStep;
                    
                    PropagPart(PosCur, VecCur, lStep/2);
                    
                    Ndep = 0;
                    while (ltot < lExt) {
                        for (i=0;i<3;i++) {
                            PosEvt[Ndep*3+i] = PosCur[i];
                            VecEvt[Ndep*3+i] = VecCur[i];
                        }
                        EdepEvt[Ndep] = lStep/dSBCdE(Ecur);
                        if ( fabs(PosEvt[Ndep]) < HalfSizeBlockLaBr3[0] && 
                             fabs(PosEvt[Ndep+1]) < HalfSizeBlockLaBr3[1] ) 
                            EdepEvt[Ndep] *= RhoLaBr3/RhoBC412;
                            
                        if ( EdepEvt[Ndep] > Ecur ) EdepEvt[Ndep] = Ecur;
                        
                        Ecur -= EdepEvt[Ndep];
                        Ndep++;
                        
                        if ( Ecur < 0.01 ) break;
                        PropagPart(PosCur, VecCur, lStep);
                        ltot += lStep;
                    }
                
                }while(0);
                
                break;
            
            case 2 :
                // bruit de fond => tirage aléatoire
                for (i=0;i<3;i++) {
                    PosInc[i] = 0;
                    VecInc[i] = 0;
                }
                Einc = -1.0;
                
                Ndep = (long)(-log2(1-GetRandUniform()))+1;
                for (jl=0;jl<Ndep;jl++) {
                    for (i=0;i<3;i++)
                        PosEvt[3*jl+i] = 2*(GetRandUniform()-0.5)*HalfSizeBlockBC412[i];
                    if (jl > 0) {
                        for (i=0;i<3;i++) {
                            VecEvt[3*(jl-1)+i] = PosEvt[3*jl+i] - PosEvt[3*(jl-1)+i];
                            Norm = VecEvt[3*(jl-1)+i]*VecEvt[3*(jl-1)+i];
                        }
                        for (i=0;i<3;i++) VecEvt[3*(jl-1)+i] /= Norm;
                    }
                    EdepEvt[jl] = EminGam + (EmaxEle-EminGam)*GetRandUniform();
                }
                for (i=0;i<3;i++)  VecEvt[3*(Ndep-1)+i] = (i==0?1:0);
                
                break;
                
            default :
                break;
        }
        
        // Utilisation des symétries pour réorganiser les évènements
        y = GetRandUniform();
        if ( y < 0.5 ) {
            for (jl=0;jl<Ndep;jl++) {
                PosInc[1] *= -1;
                VecInc[1] *= -1;
                PosEvt[3*jl+1] *= -1;
                VecEvt[3*jl+1] *= -1;
            }
        }
        
        y = GetRandUniform();
        if ( y < 0.5 ) {
            Tmp = PosInc[0];
            PosInc[0] = PosInc[1];
            PosInc[1] = -Tmp;
            Tmp = VecInc[0];
            VecInc[0] = VecInc[1];
            VecInc[1] = -Tmp;
            
            for (jl=0;jl<Ndep;jl++) {
                Tmp = PosEvt[3*jl];
                PosEvt[3*jl] = PosEvt[3*jl+1];
                PosEvt[3*jl+1] = -Tmp;
                
                Tmp = VecEvt[3*jl];
                VecEvt[3*jl] = VecEvt[3*jl+1];
                VecEvt[3*jl+1] = -Tmp;
            }
        }
        
    
        // Mesures par la matrice de détection
        for (i=0;i<16;i++) {
            MatEn[i] = 0;
            MatReadings[i] = 0;
        }
        for (jl=0;jl<Ndep;jl++) {
            xEvt = PosEvt[3*jl];
            yEvt = PosEvt[3*jl+1];
            zEvt = PosEvt[3*jl+2];
            if ( ( fabs(xEvt) < SizePix ) && (fabs(yEvt) < SizePix) ) {
                iBlock = 0;
                dTotSq = DistToEvt(xEvt, yEvt, zEvt, DistSq, iBlock);
                for (i=0;i<4;i++) {
                    MatEn[i] += EdepEvt[jl]/2.0*(0.25+DistSq[i]/dTotSq);
                    //fprintf(stdout, " dsq = %.3f",DistSq[i]);
                }
                //fprintf(stdout, " dsqtot = %.3f",dTotSq);
            }
            else {
                if ( (xEvt <= SizePix) && (yEvt <= -SizePix) ) {
                    iBlock = 1;
                }
                else if ( (xEvt <= -SizePix) && (yEvt >= -SizePix) )
                    iBlock = 2;
                else if ( (xEvt >= -SizePix) && (yEvt >= SizePix) )
                    iBlock = 3;
                else if ( (xEvt >= SizePix) && (yEvt <= SizePix) )
                    iBlock = 4;
                    
                dTotSq = DistToEvt(xEvt, yEvt, zEvt, DistSq, iBlock);
                for (i=0;i<3;i++) {
                    MatEn[4+(iBlock-1)*3+i] += EdepEvt[jl]/2.0*(1/3.+DistSq[i]/dTotSq);
                }
            }            
        }
        
        Multiplicity = 0;
        Ecur = 0;
        for (i=0;i<16;i++) {
            ADCRange[i] = -1;
            if ( MatEn[i] < EminEasiroc) continue;
            Ecur += MatEn[i];
            Multiplicity++;
            if ( MatEn[i] < EADCThreshEasiroc) {
                ADCRange[i] = 0;
                MatReadings[i] = (unsigned int)((MatEn[i] - EminEasiroc)/(EADCThreshEasiroc-EminEasiroc)*MaxValEasiroc);
                continue;
            }
            if ( MatEn[i] < EmaxEasiroc) {
                ADCRange[i] = 1;
                MatReadings[i] = (unsigned int)((MatEn[i] - EADCThreshEasiroc)/(EmaxEasiroc-EADCThreshEasiroc)*MaxValEasiroc);
                continue;
            }
            MatReadings[i] = MaxValEasiroc;
            ADCRange[i] = 2;
        }
        
        
        //for (i=0;i<16;i++) {
        //    fprintf(stdout, "  %.3f", MatEn[i]);
        //}
        //fprintf(stdout, "  %.3f <-> %.3f", Einc, Ecur);
    
         // Ecriture dans le fichier d'évènements
         if (Multiplicity > 0) {
            TimeTag = (unsigned long)((long double)((TimeOffset+TimeEvt))/TimeGranularityEasiroc);
            fprintf(EvtsFile,"%5ld %5ld %15.6f %d %10.4e %10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %3u %10.4f  %15lx", NRec++, il, TimeEvt, EvtKind, Einc, PosInc[0], PosInc[1], PosInc[2], VecInc[0], VecInc[1], VecInc[2],Multiplicity,Ecur,TimeTag);
            for (i=0;i<16;i++) {
                fprintf(EvtsFile,"   %1d  %4x",ADCRange[i],MatReadings[i]);
            }
            fprintf(EvtsFile,"\n");     
        }
    
        if (il%50000 == 0 ) {
            fprintf(stdout,"  -->  %lu / %lu evenements simules \n",il,Nevts);
        }
    
    }
    
    fclose(EvtsFile);

    return 0;
}

// return a random number between 0 and 1 (1 excluded)
double GetRandUniform() {
    return (double)random()/(RAND_MAX+1.0);
}

// return the integrated flux for a power law spectrum
double GetPowerLawSpectrumFlux(double Emin, double Emax, double A, double E0, double G, double S) {
    return A*S/((G-1.0)*pow(E0,-G))*(pow(Emin,1.0-G) - pow(Emax, 1.0-G));
}

// Renvoie l'énergie d'un photon tiré dans une loi de puissance d'indice G entre Emin et Emax
double GetPowerLawSpectrumEnergy(double Emin, double Emax, double G) {
    double y, res;
    double EminG, EmaxG;
    
    y = GetRandUniform();
    EminG = pow(Emin,1-G);
    EmaxG = pow(Emax,1-G);
    
    res = pow(EminG - (EminG-EmaxG)*y,1.0/(1-G));;
    
    //fprintf(stdout, " %.3f %.3f %.3f => %.5f",Emin,Emax,G,res);
    //fflush(stdout);
    return res;
}

// Renvoie le temps depuis le dernier evts suivant une statistique exponentielle
double GetExpDeviate(double Rate) {
    double y;
    
    y = 1.0-GetRandUniform();
    
    return -log(y)/Rate;
}

// Propagation sur une distance lambda
// !!! Ecrase la position d'entrée !!!
double PropagPart(double Pos[3], double Vect[3], double lambda) {
    int i;
    
    for (i=0;i<3;i++) Pos[i] += lambda*Vect[i];
    
    return lambda;
}

// Calcule la prochaine intersection avec les parois d'un pavé aligné 
// suivant les axes du repère et centré en (0,0,0)
// Renvoie -1 si pas d'intersection suivant la direction donnée
double InterSec(double Pos[3], double Vect[3], double HalfSizeBlock[3]) {
    double lambda, ltemp, sgn, PosInt[3];
    int i,j,k,j2,j3;
    
    lambda = -1e20;
    sgn = -1.0;
    
    for (i=0;i<6;i++) {
        j = i/2;
        sgn *= -1.0;
        //fprintf(stdout, " Step 4 j=%d ...", j);
        //fflush(stdout);
        if (fabs(Vect[j])>1e-6) {
            ltemp = (sgn*HalfSizeBlock[j]-Pos[j])/Vect[j];
            
            if ( (ltemp >= 0) && (ltemp < fabs(lambda)) ) {
                for (k=0;k<3;k++) PosInt[k] = Pos[k];
                PropagPart(PosInt, Vect, ltemp);
                j2 = (j+1)%3;
                j3 = (j+2)%3;
                
                if ( (fabs(PosInt[j2]) <= HalfSizeBlock[j2]) &&
                     (fabs(PosInt[j3]) <= HalfSizeBlock[j3]) ) {
                    lambda = ltemp;
                }
            }
        }
    }
    
    if ( lambda < 0 ) return -1.0;
    
    return lambda;
}

// Klein-Nishina factor
double KNfact(double alpha) {
    double vt, lnvt;
    
    vt = 1+2*alpha;
    lnvt = log(vt);
    return (1+alpha)/(alpha*alpha*alpha)*(2*alpha*(1+alpha)/vt-lnvt)+lnvt/(2*alpha)-(1+3*alpha)/(vt*vt);
}

// Klein Nishina angular deviation from rejection sampling
// E en MeV
double GetKNAngle(double Ein) {
    double alpha, theta, KNVal, stheta, ctheta, vt, wt;
    double y;
    int ok=0;
    
    alpha = Ein/0.511;
    
    ok=0;
    while (!ok) {
        
        // La majoration fonctionne jusqu'à 10 MeV !!!
        theta = (1.2 - sqrt(1.44-1.4*GetRandUniform()))*M_PI;
        
        stheta = sin(theta);
        ctheta = cos(theta);
        vt = alpha*(1-ctheta);
        wt = 1+vt;
    
        KNVal = (1+ctheta*ctheta+vt*vt/wt)/(wt*wt*2*KNfact(alpha))*stheta;
        y = (1.2-theta/M_PI)*GetRandUniform();
    
        if ( y <= KNVal ) ok=1;
    }
    return theta;
}

// Calcule la prochaine interaction pour un photon gamma
// Prend en argument une position et une direction, calcule la prochaine
// interaction (position et dépôt d'energie, photoelectrique et compton)
// La routine renvoie 0 si la particule sort du détecteur sans interaction
// La routine met à jour la position, la direction et l'énergie de la particule
// coordonnées en mm, E en MeV
// AlphaPhot : atténuation photoélectrique en mm-1.MeV^3
// AlphaCompt : atténuation Compton en mm-1
int InteractGamma(double PosPart[3], double VecPart[3], double *Epart, double HalfSizeBlock[3], double AlphaPhot, double AlphaCompt) {

    double muPhot, muCompt;
    double Ein, theta, phi, t[3], cat, sat, VecRes[3];
    double stheta, ctheta, sphi, cphi, norm;
    double lPhot, lCompt, lBorder, lMin;
    
    int i, i1, i2;
    
    
    
    Ein = *Epart;
    
    //fprintf(stdout, "\n Gamma : Start Ein = %.3f ...",Ein);
    //fflush(stdout);
    
    // Norme le vecteur d'entrée
    norm = 0.0;
    for (i=0;i<3;i++) norm += VecPart[i]*VecPart[i];
    norm = sqrt(norm);
    for (i=0;i<3;i++) VecPart[i] /= norm;
    
    // Calcule des absorption linéaires
    muPhot = AlphaPhot/(Ein*Ein*Ein);
    muCompt = AlphaCompt*KNfact(Ein/0.511);
    
    //fprintf(stdout, " Step 1 ...");
    //fflush(stdout);
    
    // Propage d'un petit pas pour s'écarter d'une éventuelle bordure
    PropagPart(PosPart, VecPart, 1e-6);
    
    //fprintf(stdout, " Step 2 ...");
    //fflush(stdout);
    
    // Tire aléatoirement des longueurs d'interaction photo et Compton
    lPhot = GetExpDeviate(muPhot);
    lCompt = GetExpDeviate(muCompt);
    lMin = (lPhot < lCompt ? lPhot : lCompt);
    
    //fprintf(stdout, " Step 3 ...");
    //fflush(stdout);
    
    // Distance au prochain bord
    lBorder = InterSec(PosPart, VecPart, HalfSizeBlock);
    
    //fprintf(stdout, " lMin = %.3f lBorder = %.3f ...",lMin,lBorder);
    //fflush(stdout);
    
    // Test si interaction
    if ( lMin > lBorder ) {
        PropagPart(PosPart, VecPart, lBorder);
        return 0;
    }
    
    // OK, il y a interaction dans le détecteur
    //fprintf(stdout, " lPhot = %.3f lCompt = %.3f ...",lPhot,lCompt);
    //fflush(stdout);
    // Effet photoélectrique => absorption totale
    if ( lPhot < lCompt ) { 
        PropagPart(PosPart, VecPart, lPhot);
        *Epart = 0.0;
        return 1;
    }
    
    // Interaction compton, le plus compliqué ...
    PropagPart(PosPart, VecPart, lCompt);
    
    theta = GetKNAngle(Ein);
    ctheta = cos(theta);
    stheta = sin(theta);
    
    *Epart = Ein/(1+Ein/0.511*(1-cos(theta)));
    
    //fprintf(stdout, " Epart = %.4f ...",*Epart);
    //fflush(stdout);
    
    phi = 2*M_PI*GetRandUniform();
    cphi = cos(phi);
    sphi = sin(phi);
    
    // Calcule de la direction emergente
    if (fabs(VecPart[2]) < 0.9) {
        t[0] = 0 ; 
        t[1] = 0 ;
        t[2] = 1 ;
    }
    else {
        t[0] = 1 ; 
        t[1] = 0 ;
        t[2] = 0 ;
    }
    
    cat = 0.0;
    for (i=0;i<3;i++) cat += t[i]*VecPart[i];
    sat = sqrt(1-cat*cat);
    for (i=0;i<3;i++) {
        i1 = (i+1)%3;
        i2 = (i+2)%3;
        VecRes[i] = 
                    ctheta*VecPart[i] + 
                    stheta*cphi*(cat*VecPart[i]-t[i])/sat +
                    stheta*sphi*(VecPart[i1]*t[i2] - VecPart[i2]*t[i1])/sat;
    }
    for (i=0;i<3;i++) VecPart[i] = VecRes[i];
    
    return 1;
}


// Dérivée de la longueur d'arrêt pour le plastique
// E en MeV
// Resultat en mm.MeV^-1
double dSBCdE(double E) {
    return (0.8*log(7*E+0.96)/pow(E,0.2) + 7*pow(E,0.8)/(7*E+0.96))/0.6;
}


// Distance to the centre of a given detector
double DistToDet(double x, double y, double z, int DetID) {
    double xDet, yDet, ctmp;
    double dDet;
    double iRot;
    
    if ( DetID < 5 ) {
        xDet = -SizePix/2.0;
        yDet = -SizePix/2.0;
    
        if ( (DetID-1)/2 ) xDet += SizePix;
        if ( ((DetID-1)%2) != ((DetID-1)/2) ) yDet += SizePix;
    }
    else {
        xDet = (1-2*(DetID-5)%3)*SizePix;
        yDet = -3*SizePix;
        
        iRot = (DetID-5)/4;
        ctmp = xDet;
        if (iRot == 1) {
            xDet =  yDet;
            yDet = -ctmp;
        }
        else if (iRot == 1) {
            xDet = -xDet;
            yDet = -yDet;
        }
        else {
            xDet = -yDet;
            yDet =  ctmp;
        }
    }

    dDet = (x-xDet)*(x-xDet) + (y-yDet)*(y-yDet) + (z+DepthDet/2)*(z+DepthDet/2);
    
    return sqrt(dDet);
}

// Retourne une matrice des distances au carré à un evt
// NBlock : identification du bloc (0=LaBr3, 1-4=BC412)
double DistToEvt(double x, double y, double z, double DistSq[], int NBlock) {
    double dTot;
    double xDet, yDet, ctmp;
    int i;
    
    dTot = 0.0;
    if (NBlock == 0) {
        xDet = -SizePix/2.0;
        yDet = -SizePix/2.0;
        for (i=0;i<4;i++) {
            DistSq[i] = 
            (x-xDet)*(x-xDet) + (y-yDet)*(y-yDet) + (z+DepthDet/2)*(z+DepthDet/2);
            dTot += DistSq[i];
            ctmp = xDet;
            xDet =  yDet;
            yDet = -ctmp;
        }
        return dTot;
    }
    
    ctmp = x;
    switch (NBlock) {
        case 2 :
            x = -y;
            y = ctmp;
            break;
        case 3 : 
            x = -y;
            y = -ctmp;
            break;
        case 4 :
            x = y;
            y = -ctmp;
            break;
        default :
            break;
    }
    
    dTot = 0.0;
    xDet = +SizePix/2.0;
    yDet = -3*SizePix/2.0;
    for (i=0;i<3;i++) {
        DistSq[i] = 
            (x-xDet)*(x-xDet) + (y-yDet)*(y-yDet) + (z+DepthDet/2)*(z+DepthDet/2);
        dTot += DistSq[i];
        xDet -= SizePix;
    }
    
    return dTot;
}