/*********************************************************************/
/*********************************************************************/
/***							      	   ***/
/***         	TP MinMax / Module TPGO / ESI 2016-2017            ***/
/***							      	   ***/
/*** Il est demandé d'améliorer la fonction estim(...) ci-dessous. ***/
/*** Son rôle est d'évaluer la qualité d'une configuration donnée. ***/
/*** Elle doit retourner une valeur dans ]-100 , +100[		   ***/
/***							      	   ***/
/*********************************************************************/
/*********************************************************************/
#include<omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>  // pour INT_MAX
#include<sys/time.h>


#define MAX +1	// Niveau Maximisant
#define MIN -1  // Niveau Minimisant
#define INFINI INT_MAX

#define KGRN "\x1B[32m"
#define KRED "\x1B[31m"
#define KWHT "\x1B[37m"


// Type d'une configuration formant l'espace de recherche
struct config {
	char mat[8][8];			// l'échiquier
	int val;			// estimation de la config
	char xrN, yrN, xrB, yrB;	// positions des rois Noir et Blanc
	char roqueN, roqueB;		// indicateurs de roque pour N et B : 
						// 'g'rand roque non réalisable  
						// 'p'etit roque non réalisable 
						// 'n'on réalisable des 2 cotés
						// 'r'éalisable (valeur initiale)
						// 'e'ffectué
};

// Pinalite pour le pion isole
int PawnIsolated = -30;

// Pinalite pour le pion doublé
int PawnDouble = -30;

// Pour valoriser une tour sur une colonne ouverte
int RookOpenFile = 70;

// Pour valoriser une tour sur une colonne semi ouverte
int RookSemiOpenFile = 50;

// Pour valoriser une reine sur une colonne ouverte
int QueenOpenFile = 50;

// Pour valoriser une reine sur une colonne semi ouverte
int QueenSemiOpenFile = 30;

// La valeur d'un pair de fous
int BishopPair = 50;

// L'evaluation de l'echequier
// Les positions sont comme suit : a1, b1, c1, ...
//                                 a2, b2, c2, ...
//Donc un pion sur d2 pour les blancs est evalue a -50, si il bouge a d4, il est a 90, ...

int PawnTable[64] = {
	0	    ,  	0   	,   0   	,   0   	,   0   	,  	0	    ,	0	    ,	0	    ,
	50	    ,  	50	    ,   0	    ,   -10     ,   -10     ,  	0	    ,	50	    ,	50	    ,
	25	    ,  	0	    ,  	0	    ,  	25	    ,  	25	    ,  	0	    ,	0	    ,	25	    ,
	0	    ,  	0	    ,  	50	    ,  	90	    ,  	90	    ,  	50	    ,	0	    ,	0	    ,
	25	    ,  	25	    ,  	25	    ,  	50	    ,  	50	    ,  	25	    ,	25	    ,	25	    ,
	50      ,  	50	    ,  	50	    ,  	90	    ,  	90	    ,  	50	    ,	50	    ,	50	    ,
	90	    ,   90 	    ,  	90	    ,  	90	    ,  	90	    ,  	90	    ,	90	    ,	90	    ,
	0	    ,  	0	    ,  	0	    ,  	0	    ,  	0	    ,  	0	    ,	0	    ,	0
};

int KnightTable[64] = {
	0   	,	-50	    ,	0	    ,	0	    ,	0	    ,	0	    ,	-50 	,	0   	,
	0   	,	0	    ,	0	    ,	25	    ,	25	    ,	0	    ,	0	    ,	0   	,
	0	    ,	0	    ,	50	    ,	50	    ,	50	    ,	50	    ,	0	    ,	0	    ,
	0	    ,	25	    ,	50	    ,	90	    ,	90	    ,	50	    ,	25	    ,	0   	,
	25	    ,	50	    ,	70	    ,	90	    ,	90	    ,	70	    ,	50	    ,	25	    ,
	25	    ,  	50	    ,	50	    ,	90	    ,	90	    ,	50	    ,	50	    ,	25	    ,
	0	    ,	0	    ,	25	    ,	50	    ,	50	    ,	25	    ,	0	    ,	0   	,
	0	    ,	0	    ,	0	    ,	0	    ,	0	    ,	0	    ,	0	    ,	0
};

int BishopTable[64] = {
	0	    ,	0	    ,	-50	    ,	0	    ,	0	    ,	-50	    ,	0	    ,	0   	,
	0	    ,  	0	    ,	0	    ,	50	    ,	50	    ,	0	    ,	0	    ,	0	    ,
	0	    ,	0	    ,	50	    ,	70	    ,	70	    ,	50  	,	0	    ,	0	    ,
	0	    ,	50	    ,	70	    ,	90	    ,	90	    ,	70	    ,	50	    ,	0   	,
	0	    ,	50	    ,	70	    ,	90	    ,	90	    ,	70	    ,	50	    ,	0   	,
	0	    ,	0	    ,	50	    ,	70	    ,	70	    ,	50	    ,	0	    ,	0   	,
	0	    ,	0	    ,	0	    ,	50	    ,	50	    ,	0   	,	0	    ,	0   	,
	0	    ,	0   	,	0	    ,	0	    ,	0	    ,	0   	,	0	    ,	0
};

int RookTable[64] = {
    0   	,	0	    ,	25  	,	50	    ,	50	    ,	25	    ,	0	    ,	0	    ,
	0	    ,	0	    ,	25	    ,	50	    ,	50	    ,	25	    ,	0	    ,	0   	,
	0	    ,	0	    ,	25	    ,	50	    ,	50	    ,	25	    ,	0	    ,	0	    ,
	0	    ,	0	    ,	25	    ,	50	    ,	50	    ,	25  	,	0	    ,	0   	,
	0	    ,	0	    ,	25	    ,	50	    ,	50	    ,	25	    ,	0	    ,	0	    ,
	0	    ,	0	    ,	25	    ,	50	    ,	50	    ,	25	    ,	0	    ,	0	    ,
	90	    ,	90	    ,	90	    ,	90	    ,	90	    ,	90	    ,	90	    ,	90	    ,
	0	    ,	0	    ,	25	    ,	50	    ,	50	    ,	25	    ,	0	    ,	0
};

// l'evaluation du roi pendant la fin de partie
int KingE[64] = {
	-500    ,	-50	    ,	0   	,	0   	,	0   	,	0   	,	-50	    ,	-500	,
	-50     ,	0	    ,	50 	    ,	50	    ,	50	    ,	50	    ,	0   	,	-50     ,
	0	    ,	50	    ,	90	    ,	90	    ,	90	    ,	90	    ,	50	    ,	0	    ,
	0	    ,	50	    ,	90	    ,	99	    ,	99	    ,	90	    ,	50	    ,	0	    ,
	0	    ,	50	    ,	90	    ,	99	    ,	99	    ,	90	    ,	50	    ,	0	    ,
	0	    ,	50	    ,	90	    ,	90	    ,	90	    ,	90	    ,	50	    ,	0	    ,
	-50     ,	0	    ,	50	    ,	50	    ,	50	    ,	50	    ,	0	    ,	-50	    ,
	-500 	,	-50	    ,	0	    ,	0	    ,	0	    ,	0	    ,	-50	    ,	-500
};

// L'evaluation du roi pendant l'ouverture, on donne la priorite au petit roque (plus sucurise)
int KingO[64] = {
	0	    ,	25	    ,	25	    ,	-50     ,	-50 	,	0	    ,	50	    ,	25	    ,
	-300	,	-300	,	-300	,	-300	,	-300	,	-300	,	-300	,	-300	,
	-500	,	-500	,	-500	,	-500	,	-500	,	-500	,	-500	,	-500	,
	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,
	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,
	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,
	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,
	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700	,	-700
};

// vecteurs des déplacements par type de piece ...
// cavalier
int dC[8][2] = { {-2,+1} , {-1,+2} , {+1,+2} , {+2,+1} , {+2,-1} , {+1,-2} , {-1,-2} , {-2,-1} };
// fou (indices impairs), tour (indices pairs), reine et roi (indices pairs et impairs) ...
int D[8][2] = { {+1,0} , {+1,+1} , {0,+1} , {-1,+1} , {-1,0} , {-1,-1} , {0,-1} , {+1,-1} }; 

// evalue avec alpha beta la configuration 'conf' du joueur 'mode' en descendant de 'niv' niveaux
int minmax_ab( struct config conf, int mode, int niv, int min, int max, long * nb_noeuds, long * nb_coupes);


/* Copie la configuration c1 dans c2  */
void copier( struct config *c1, struct config *c2 ) 
{
	int i, j;

	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			c2->mat[i][j] = c1->mat[i][j];

	c2->val = c1->val;
	c2->xrB = c1->xrB;
	c2->yrB = c1->yrB;
	c2->xrN = c1->xrN;
	c2->yrN = c1->yrN;

	c2->roqueB = c1->roqueB;
	c2->roqueN = c1->roqueN;
} // copier


/* Teste si les conf c1 et c2 sont égales */
int egal(char c1[8][8], char c2[8][8] )
{
	int i, j;

	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			if (c1[i][j] != c2[i][j]) return 0;
	return 1;
} // egal

int nbrPieces(struct config board, bool type) /// Compter le nombre des piéces
{
    int i, j, nbr = 0;
    if(type)
    {
        for(i = 0 ; i <= 7 ; i++)
        {
            for(j = 0 ; j <= 7 ; j++)
            {
                if(board.mat[i][j] > 0)
                {
                    nbr ++;
                }
            }
        }
    }
    else
    {
        for(i = 0 ; i <= 7 ; i++)
        {
            for(j = 0 ; j <= 7 ; j++)
            {
                if(board.mat[i][j] < 0)
                {
                    nbr ++;
                }
            }
        }
    }
    return nbr;
}


int estim(struct config board) /// La fonction d'evaluation de la configuration
{
    int i, j;
    int matrice = 0;
    int isole = 0, rowB, rowN, nbrPionB = 0, nbrPionN = 0, doubl = 0;
    bool pionPosB_1 = false, pionPosB_2 = false, pionPosB = false;
    bool pionPosN_1 = false, pionPosN_2 = false, pionPosN = false;
    int k, rockB_nbrOpen = 0, rockN_nbrOpen = 0, rockValue = 0;
    int queenN_nbrOpen = 0, queenB_nbrOpen = 0, queenValue = 0;
    int nbrBishopB = 0, nbrBishopN = 0, bishopValue = 0;
    int resultat, materiel=0;
    for(j = 0 ; j <= 7 ; j++)
    {
        for(i = 0 ; i <= 7 ; i++)
        {
            switch(board.mat[i][j])
            {
                case 'p':
                    materiel += 100;

                    matrice += PawnTable[j + i * 8];

                    nbrPionB ++;
                    pionPosB = true;
                    if(!pionPosB_1)
                    {
                        if(!pionPosB_2)
                        {
                            pionPosB_2 = true;
                            rowB = j;
                        }
                        else
                        {
                            if (rowB != j)
                            {
                                pionPosB_1 = true;
                            }
                        }
                    }
                break;
                case -'p':
                    materiel -= 100;

                    matrice -= PawnTable[j + (7 - i) * 8];

                    nbrPionN ++;
                    pionPosN = true;
                    if(!pionPosN_1)
                    {
                        if(!pionPosN_2)
                        {
                            pionPosN_2 = true;
                            rowN = j;
                        }
                        else
                        {
                            if (rowN != j)
                            {
                                pionPosN_1 = true;
                            }
                        }
                    }
                break;
                case 'C':
                    materiel += 300;

                    matrice += KnightTable[j + i * 8];
                break;
                case -'C':
                    materiel -= 300;

                    matrice -= KnightTable[j + (7 - i) * 8];
                break;
                case 'f':
                    materiel += 325;

                    matrice += BishopTable[j + i * 8];

                    nbrBishopB ++;
                break;
                case -'f':
                    materiel -= 325;

                    matrice -= BishopTable[j + (7 - i) * 8];

                    nbrBishopN ++;
                break;
                case 't':
                    materiel += 500;

                    matrice += RookTable[j + i * 8];

                    k = 0;
                    while((k <= 7) && (board.mat[k][j] != 'p'))
                    {
                        if(((board.mat[k][j] == 0) || (board.mat[k][j] == 't')) || (board.mat[k][j] < 0))
                        {
                            rockB_nbrOpen ++;
                        }
                        k++;
                    }
                break;
                case -'t':
                    materiel -= 500;

                    matrice -= RookTable[j + (7 - i) * 8];

                    k = 7;
                    while((k >= 0) && (board.mat[k][j] != -'p'))
                    {
                        if(((board.mat[k][j] == 0) || (board.mat[k][j] == -'t')) || (board.mat[k][j] < 0))
                        {
                            rockN_nbrOpen ++;
                        }
                        k --;
                    }
                break;
                case 'n':
                    materiel += 1000;
                    k = 0;
                    while((k <= 7) && (board.mat[k][j] != 'p'))
                    {
                        if(((board.mat[k][j] == 0) || (board.mat[k][j] == 'n')) || (board.mat[k][j] < 0))
                        {
                            queenB_nbrOpen ++;
                        }
                        k++;
                    }
                break;
                case -'n':
                    materiel -= 1000;
                    k = 7;
                    while((k >= 0) && (board.mat[k][j] != -'p'))
                    {
                        if(((board.mat[k][j] == 0) || (board.mat[k][j] == -'n')) || (board.mat[k][j] < 0))
                        {
                            queenN_nbrOpen ++;
                        }
                        k --;
                    }
                break;
                case 'r':
                    if(nbrPieces(board, true) > 8)
                    {
                        matrice += KingO[j + i * 8];
                    }
                    if(nbrPieces(board, true) < 7)
                    {
                        matrice += KingE[j + i * 8];
                    }
                break;
                case -'r':
                    if(nbrPieces(board, true) > 8)
                    {
                        matrice -= KingO[j + (7 - i) * 8];
                    }
                    if(nbrPieces(board, true) < 7)
                    {
                        matrice -= KingE[j + (7 - i) * 8];
                    }
                break;
            }
        }

        if(nbrPionB > 0)
        {
            doubl = doubl + nbrPionB - 1;
        }
        if(nbrPionN > 0)
        {
            doubl = doubl + nbrPionN - 1;
        }
        nbrPionB = 0;
        nbrPionN = 0;

        if(!pionPosB && !pionPosB_1 && pionPosB_2)
        {
            isole ++;
        }
        if(!pionPosB)
        {
            pionPosB_1 = false;
            pionPosB_2 = false;
        }
        pionPosB = false;

        nbrPionN = 0;
        if(!pionPosN && !pionPosN_1 && pionPosN_2)
        {
            isole --;
        }
        if(!pionPosN)
        {
            pionPosN_1 = false;
            pionPosN_2 = false;
        }
        pionPosN = false;

        if(rockB_nbrOpen == 8)
        {
            rockValue += RookOpenFile;
        }
        else
        {
            if(rockB_nbrOpen > 5)
            {
                rockValue += RookSemiOpenFile;
            }
        }
        if(rockN_nbrOpen == 8)
        {
            rockValue -= RookOpenFile;
        }
        else
        {
            if(rockN_nbrOpen > 5)
            {
                rockValue -= RookSemiOpenFile;
            }
        }
        rockB_nbrOpen = 0;
        rockN_nbrOpen = 0;

        if(queenB_nbrOpen == 8)
        {
            queenValue += QueenOpenFile;
        }
        else
        {
            if(queenB_nbrOpen > 5)
            {
                queenValue += QueenSemiOpenFile;
            }
        }
        if(queenN_nbrOpen == 8)
        {
            queenValue -= QueenOpenFile;
        }
        else
        {
            if(queenN_nbrOpen > 5)
            {
                queenValue -= QueenSemiOpenFile;
            }
        }
        queenB_nbrOpen = 0;
        queenN_nbrOpen = 0;

        if(nbrBishopB == 2)
        {
            bishopValue += BishopPair;
            nbrBishopB = 0;
        }
        if(nbrBishopN == 2)
        {
            bishopValue -= BishopPair;
            nbrBishopN = 0;
        }
    }
    resultat = materiel + doubl * PawnDouble + isole * PawnIsolated + rockValue + queenValue + bishopValue + matrice;

    return resultat;
}
 // estim

/***********************************************************/
/*********** Partie:  Evaluations et Estimations ***********/
/***********************************************************/


/* Teste s'il n'y a aucun coup possible dans la configuration conf */
int AucunCoupPossible( struct config conf )
{
      	// ... A completer pour les matchs nuls
	// ... vérifier que generer_succ retourne 0 configurations filles ...
	return 0;

} // AucunCoupPossible


/* Teste si conf représente une fin de partie et retourne dans 'cout' la valeur associée */
int feuille( struct config conf, int *cout )
{
	//int i, j, rbx, rnx, rby, rny;
	
	*cout = 0;

	// Si victoire pour les Noirs cout = -100
	if ( conf.xrB == -1 ) { 
	   *cout = -100;
	   return 1; 
	}

	// Si victoire pour les Blancs cout = +100
	if ( conf.xrN == -1 ) {
	   *cout = +100;
	   return 1;
	}

	// Si Match nul cout = 0
	if (  conf.xrB != -1 &&  conf.xrN != -1 && AucunCoupPossible( conf ) )
	   return 1;

	// Sinon ce n'est pas une config feuille 
	return 0;

}  // feuille



/***********************************************************/
/*********** Partie:  Génération des Successeurs ***********/
/***********************************************************/


/* Génère dans T les configurations obtenues à partir de conf lorsqu'un pion atteint la limite de l'échiq */
void transformPion( struct config conf, int a, int b, int x, int y, struct config T[], int *n )
{
	int signe = +1;
	if (conf.mat[a][b] < 0 ) signe = -1;
	copier(&conf, &T[*n]);
	T[*n].mat[a][b] = 0;
	T[*n].mat[x][y] = signe *'n';
	(*n)++;
	copier(&conf, &T[*n]);
	T[*n].mat[a][b] = 0;
	T[*n].mat[x][y] = signe *'c';
	(*n)++;
	copier(&conf, &T[*n]);
	T[*n].mat[a][b] = 0;
	T[*n].mat[x][y] = signe *'f';
	(*n)++;
	copier(&conf, &T[*n]);
	T[*n].mat[a][b] = 0;
	T[*n].mat[x][y] = signe *'t';
	(*n)++;

} // transformPion


// Vérifie si la case (x,y) est menacée par une des pièces du joueur 'mode'
int caseMenaceePar( int mode, int x, int y, struct config conf )
{
	int i, j, a, b, stop;

	// menace par le roi ...
	for (i=0; i<8; i += 1) {
	   // traitement des 8 directions paires et impaires
	   a = x + D[i][0];
	   b = y + D[i][1];	  	 
	   if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 ) 
		if ( conf.mat[a][b]*mode == 'r' ) return 1;
	} // for

	// menace par cavalier ...
	for (i=0; i<8; i++)
	   if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
		if ( conf.mat[ x+dC[i][0] ] [ y+dC[i][1] ] * mode == 'c' )  
		   return 1;

	// menace par pion ...
	if ( (x-mode) >= 0 && (x-mode) <= 7 && y > 0 && conf.mat[x-mode][y-1]*mode == 'p' )
	   return 1;
	if ( (x-mode) >= 0 && (x-mode) <= 7 && y < 7 && conf.mat[x-mode][y+1]*mode == 'p' )
	   return 1;

	// menace par fou, tour ou reine ...
	for (i=0; i<8; i += 1) {
	   // traitement des 8 directions paires et impaires
	   stop = 0;
	   a = x + D[i][0];
	   b = y + D[i][1];	  	 
	   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) 
		if ( conf.mat[a][b] != 0 )  stop = 1;
		else {
		    a = a + D[i][0];
		    b = b + D[i][1];
		}
	   if ( stop )  {
		if ( conf.mat[a][b]*mode == 'f' && i % 2 != 0 ) return 1; 
		if ( conf.mat[a][b]*mode == 't' && i % 2 == 0 ) return 1;
		if ( conf.mat[a][b]*mode == 'n' ) return 1;
	   }
	} // for

	// sinon, aucune menace ...
	return 0;

} // caseMenaceePar


/* Génere dans T tous les coups possibles de la pièce (de couleur N) se trouvant à la pos x,y */
void deplacementsN(struct config conf, int x, int y, struct config T[], int *n )
{
	int i, j, a, b, stop;

	switch(conf.mat[x][y]) {
	// mvmt PION ...
	case -'p' : 
		//***printf("PION N à la pos (%d,%d) \n", x,y);
		if ( x > 0 && conf.mat[x-1][y] == 0 ) {				// avance d'une case
			copier(&conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x-1][y] = -'p';
			(*n)++;
			if ( x == 1 ) transformPion( conf, x, y, x-1, y, T, n );
		}
		if ( x == 6 && conf.mat[5][y] == 0 && conf.mat[4][y] == 0) {	// avance de 2 cases
			copier(&conf, &T[*n]);
			T[*n].mat[6][y] = 0;
			T[*n].mat[4][y] = -'p';
			(*n)++;
		}
		if ( x > 0 && y >0 && conf.mat[x-1][y-1] > 0 ) {		// mange à droite (en descendant)
			copier(&conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x-1][y-1] = -'p';
			// cas où le roi adverse est pris...
			if (T[*n].xrB == x-1 && T[*n].yrB == y-1) { 
				T[*n].xrB = -1; T[*n].yrB = -1; 
			}

			(*n)++;
			if ( x == 1 ) transformPion( conf, x, y, x-1, y-1, T, n ); 
		}
		if ( x > 0 && y < 7 && conf.mat[x-1][y+1] > 0 ) {		// mange à gauche (en descendant)
			copier(&conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x-1][y+1] = -'p';
			// cas où le roi adverse est pris...
			if (T[*n].xrB == x-1 && T[*n].yrB == y+1) { 
				T[*n].xrB = -1; T[*n].yrB = -1; 
			}

			(*n)++;
			if ( x == 1 ) transformPion( conf, x, y, x-1, y+1, T, n );
		}
		break;

	// mvmt CAVALIER ...
	case -'c' : 
		for (i=0; i<8; i++)
		   if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
			if ( conf.mat[ x+dC[i][0] ] [ y+dC[i][1] ] >= 0 )  {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   T[*n].mat[ x+dC[i][0] ][ y+dC[i][1] ] = -'c';
			   // cas où le roi adverse est pris...
			   if (T[*n].xrB == x+dC[i][0] && T[*n].yrB == y+dC[i][1]) { 
				T[*n].xrB = -1; T[*n].yrB = -1; 
			   }

			   (*n)++;
			}
		break;

	// mvmt FOU ...
	case -'f' : 
		for (i=1; i<8; i += 2) {
		   // traitement des directions impaires (1, 3, 5 et 7)
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];	  	 
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf.mat[ a ] [ b ] < 0 )  stop = 1;
			else {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] > 0 ) stop = 1;
			   T[*n].mat[a][b] = -'f';
			   // cas où le roi adverse est pris...
			   if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt TOUR ...
	case -'t' : 
		for (i=0; i<8; i += 2) {
		   // traitement des directions paires (0, 2, 4 et 6)
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];	  	 
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf.mat[ a ] [ b ] < 0 )  stop = 1;
			else {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] > 0 ) stop = 1;
			   T[*n].mat[a][b] = -'t';
			   // cas où le roi adverse est pris...
			   if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

			   if ( conf.roqueN != 'e' && conf.roqueN != 'n' ) {
			      if ( x == 7 && y == 0 && conf.roqueN != 'p')
			   	T[*n].roqueN = 'g'; // le grand roque ne sera plus possible
			      else if ( x == 7 && y == 0 )
			   	   T[*n].roqueN = 'n'; // ni le grand roque ni le petit roque ne seront possibles
			      if ( x == 7 && y == 7 && conf.roqueN != 'g' )
			   	T[*n].roqueN = 'p'; // le petit roque ne sera plus possible
			      else if ( x == 7 && y == 7 )
			   	   T[*n].roqueN = 'n'; // ni le grand roque ni le petit roque ne seront possibles
			   }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt REINE ...
	case -'n' : 
		for (i=0; i<8; i += 1) {
		   // traitement des 8 directions paires et impaires
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];	  	 
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf.mat[ a ] [ b ] < 0 )  stop = 1;
			else {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] > 0 ) stop = 1;
			   T[*n].mat[a][b] = -'n';
			   // cas où le roi adverse est pris...
			   if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt ROI ...
	case -'r' : 
		// vérifier possibilité de faire un roque ...
		if ( conf.roqueN != 'n' && conf.roqueN != 'e' ) {
		   if ( conf.roqueN != 'g' && conf.mat[7][1] == 0 && conf.mat[7][2] == 0 && conf.mat[7][3] == 0 )
		      if ( !caseMenaceePar( MAX, 7, 1, conf ) && !caseMenaceePar( MAX, 7, 2, conf ) && \
			   !caseMenaceePar( MAX, 7, 3, conf ) && !caseMenaceePar( MAX, 7, 4, conf ) )  {
			// Faire un grand roque ...
			copier(&conf, &T[*n]);
			T[*n].mat[7][4] = 0;
			T[*n].mat[7][0] = 0;
			T[*n].mat[7][2] = -'r'; T[*n].xrN = 7; T[*n].yrN = 2;
			T[*n].mat[7][3] = -'t';
			T[*n].roqueN = 'e'; // aucun roque ne sera plus possible à partir de cette config
			(*n)++;
		      }
		   if ( conf.roqueN != 'p' && conf.mat[7][5] == 0 && conf.mat[7][6] == 0 )
		      if ( !caseMenaceePar( MAX, 7, 4, conf ) && !caseMenaceePar( MAX, 7, 5, conf ) && \
			   !caseMenaceePar( MAX, 7, 6, conf ) )  {
			// Faire un petit roque ...
			copier(&conf, &T[*n]);
			T[*n].mat[7][4] = 0;
			T[*n].mat[7][7] = 0;
			T[*n].mat[7][6] = -'r'; T[*n].xrN = 7; T[*n].yrN = 6;
			T[*n].mat[7][5] = -'t';
			T[*n].roqueN = 'e'; // aucun roque ne sera plus possible à partir de cette config
			(*n)++;

		      }
		}
			
		// vérifier les autres mouvements du roi ...
		for (i=0; i<8; i += 1) {
		   // traitement des 8 directions paires et impaires
		   a = x + D[i][0];
		   b = y + D[i][1];	  	 
		   if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 ) 
			if ( conf.mat[a][b] >= 0 ) {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   T[*n].mat[a][b] = -'r'; T[*n].xrN = a; T[*n].yrN = b;
			   // cas où le roi adverse est pris...
			   if (T[*n].xrB == a && T[*n].yrB == b) { T[*n].xrB = -1; T[*n].yrB = -1; }

			   T[*n].roqueN = 'n'; // aucun roque ne sera plus possible à partir de cette config
			   (*n)++;
			}
		} // for
		break;

	}

} // deplacementsN


/* Génere dans T tous les coups possibles de la pièce (de couleur B) se trouvant à la pos x,y */
void deplacementsB(struct config conf, int x, int y, struct config T[], int *n )
{
	int i, j, a, b, stop;

	switch(conf.mat[x][y]) {
	// mvmt PION ...
	case 'p' :  
		if ( x <7 && conf.mat[x+1][y] == 0 ) {				// avance d'une case
			copier(&conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x+1][y] = 'p';
			(*n)++;
			if ( x == 6 ) transformPion( conf, x, y, x+1, y, T, n );
		}
		if ( x == 1 && conf.mat[2][y] == 0 && conf.mat[3][y] == 0) {	// avance de 2 cases
			copier(&conf, &T[*n]);
			T[*n].mat[1][y] = 0;
			T[*n].mat[3][y] = 'p';
			(*n)++;
		}
		if ( x < 7 && y > 0 && conf.mat[x+1][y-1] < 0 ) {		// mange à gauche (en montant)
			copier(&conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x+1][y-1] = 'p';
			// cas où le roi adverse est pris...
			if (T[*n].xrN == x+1 && T[*n].yrN == y-1) { 
				T[*n].xrN = -1; T[*n].yrN = -1; 
			}

			(*n)++;
			if ( x == 6 ) transformPion( conf, x, y, x+1, y-1, T, n );
		}
		if ( x < 7 && y < 7 && conf.mat[x+1][y+1] < 0 ) {		// mange à droite (en montant)
			copier(&conf, &T[*n]);
			T[*n].mat[x][y] = 0;
			T[*n].mat[x+1][y+1] = 'p';
			// cas où le roi adverse est pris...
			if (T[*n].xrN == x+1 && T[*n].yrN == y+1) { 
				T[*n].xrN = -1; T[*n].yrN = -1; 
			}

			(*n)++;
			if ( x == 6 ) transformPion( conf, x, y, x+1, y+1, T, n );
		}
		break;

	// mvmt CAVALIER ...
	case 'c' : 
		for (i=0; i<8; i++)
		   if ( x+dC[i][0] <= 7 && x+dC[i][0] >= 0 && y+dC[i][1] <= 7 && y+dC[i][1] >= 0 )
			if ( conf.mat[ x+dC[i][0] ] [ y+dC[i][1] ] <= 0 )  {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   T[*n].mat[ x+dC[i][0] ][ y+dC[i][1] ] = 'c';
			   // cas où le roi adverse est pris...
			   if (T[*n].xrN == x+dC[i][0] && T[*n].yrN == y+dC[i][1]) { 
				T[*n].xrN = -1; T[*n].yrN = -1; 
			   }

			   (*n)++;
			}
		break;

	// mvmt FOU ...
	case 'f' : 
		for (i=1; i<8; i += 2) {
		   // traitement des directions impaires (1, 3, 5 et 7)
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];	  	 
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf.mat[ a ] [ b ] > 0 )  stop = 1;
			else {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] < 0 ) stop = 1;
			   T[*n].mat[a][b] = 'f';
			   // cas où le roi adverse est pris...
			   if (T[*n].xrN == a && T[*n].yrN == b) { T[*n].xrN = -1; T[*n].yrN = -1; }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt TOUR ...
	case 't' : 
		for (i=0; i<8; i += 2) {
		   // traitement des directions paires (0, 2, 4 et 6)
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];	  	 
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf.mat[ a ] [ b ] > 0 )  stop = 1;
			else {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] < 0 ) stop = 1;
			   T[*n].mat[a][b] = 't';
			   // cas où le roi adverse est pris...
			   if (T[*n].xrN == a && T[*n].yrN == b) { T[*n].xrN = -1; T[*n].yrN = -1; }

			   if ( conf.roqueB != 'e' && conf.roqueB != 'n' ) {
			     if ( x == 0 && y == 0 && conf.roqueB != 'p')
			   	T[*n].roqueB = 'g'; // le grand roque ne sera plus possible
			     else if ( x == 0 && y == 0 )
			   	   T[*n].roqueB = 'n'; // ni le grand roque ni le petit roque ne seront possibles
			     if ( x == 0 && y == 7 && conf.roqueB != 'g' )
			   	T[*n].roqueB = 'p'; // le petit roque ne sera plus possible
			     else if ( x == 0 && y == 7 )
			   	   T[*n].roqueB = 'n'; // ni le grand roque ni le petit roque ne seront possibles
			   }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt REINE ...
	case 'n' : 
		for (i=0; i<8; i += 1) {
		   // traitement des 8 directions paires et impaires
		   stop = 0;
		   a = x + D[i][0];
		   b = y + D[i][1];	  	 
		   while ( !stop && a >= 0 && a <= 7 && b >= 0 && b <= 7 ) {
			if ( conf.mat[ a ] [ b ] > 0 )  stop = 1;
			else {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   if ( T[*n].mat[a][b] < 0 ) stop = 1;
			   T[*n].mat[a][b] = 'n';
			   // cas où le roi adverse est pris...
			   if (T[*n].xrN == a && T[*n].yrN == b) { T[*n].xrN = -1; T[*n].yrN = -1; }

			   (*n)++;
		   	   a = a + D[i][0];
		   	   b = b + D[i][1];
			}
		   } // while
		} // for
		break;

	// mvmt ROI ...
	case 'r' : 
		// vérifier possibilité de faire un roque ...
		if ( conf.roqueB != 'n' && conf.roqueB != 'e' ) {
		   if ( conf.roqueB != 'g' && conf.mat[0][1] == 0 && conf.mat[0][2] == 0 && conf.mat[0][3] == 0 )
		      if ( !caseMenaceePar( MIN, 0, 1, conf ) && !caseMenaceePar( MIN, 0, 2, conf ) && \
			   !caseMenaceePar( MIN, 0, 3, conf ) && !caseMenaceePar( MIN, 0, 4, conf ) )  {
			// Faire un grand roque ...
			copier(&conf, &T[*n]);
			T[*n].mat[0][4] = 0;
			T[*n].mat[0][0] = 0;
			T[*n].mat[0][2] = 'r'; T[*n].xrB = 0; T[*n].yrB = 2;
			T[*n].mat[0][3] = 't';
			T[*n].roqueB = 'e'; // aucun roque ne sera plus possible à partir de cette config
			(*n)++;
		      }
		   if ( conf.roqueB != 'p' && conf.mat[0][5] == 0 && conf.mat[0][6] == 0 )
		      if ( !caseMenaceePar( MIN, 0, 4, conf ) && !caseMenaceePar( MIN, 0, 5, conf ) && \
			   !caseMenaceePar( MIN, 0, 6, conf ) )  {
			// Faire un petit roque ...
			copier(&conf, &T[*n]);
			T[*n].mat[0][4] = 0;
			T[*n].mat[0][7] = 0;
			T[*n].mat[0][6] = 'r'; T[*n].xrB = 0; T[*n].yrB = 6;
			T[*n].mat[0][5] = 't';
			T[*n].roqueB = 'e'; // aucun roque ne sera plus possible à partir de cette config
			(*n)++;

		      }
		}
			
		// vérifier les autres mouvements du roi ...
		for (i=0; i<8; i += 1) {
		   // traitement des 8 directions paires et impaires
		   a = x + D[i][0];
		   b = y + D[i][1];	  	 
		   if ( a >= 0 && a <= 7 && b >= 0 && b <= 7 ) 
			if ( conf.mat[a][b] <= 0 ) {
			   copier(&conf, &T[*n]);
			   T[*n].mat[x][y] = 0;
			   T[*n].mat[a][b] = 'r'; T[*n].xrB = a; T[*n].yrB = b;
			   // cas où le roi adverse est pris...
			   if (T[*n].xrN == a && T[*n].yrN == b) { T[*n].xrN = -1; T[*n].yrN = -1; }

			   T[*n].roqueB = 'n'; // aucun roque ne sera plus possible à partir de cette config
			   (*n)++;
			}
		} // for
		break;

	}

} // deplacementsB


/* Génère les successeurs de la configuration conf dans le tableau T, 
   retourne aussi dans n le nb de configurations filles générées */
void generer_succ( struct config conf, int mode, struct config T[], int *n )
{
	int i, j, k, stop;

	*n = 0;

	if ( mode == MAX ) 
	{		// mode == MAX
	   for (i=0; i<8; i++)
	      for (j=0; j<8; j++)
		 if ( conf.mat[i][j] > 0 )
		    deplacementsB(conf, i, j, T, n );

	   // vérifier si le roi est en echec, auquel cas on ne garde que les succ évitants l'échec...
	   for (k=0; k < *n; k++) {
	      	i = T[k].xrB; j = T[k].yrB;  // pos du roi B dans T[k]
		// vérifier s'il est menacé dans la config T[k] ...
		if ( caseMenaceePar( MIN, i, j, T[k] ) ) {
		    T[k] = T[(*n)-1];	// alors supprimer T[k] de la liste des succ...
		    (*n)--;
		    k--;
		}
	    } // for k
	}

	else 
	{ 				// mode == MIN
	   for (i=0; i<8; i++)
	      for (j=0; j<8; j++)
		 if ( conf.mat[i][j] < 0 )
		    deplacementsN(conf, i, j, T, n );

	   // vérifier si le roi est en echec, auquel cas on ne garde que les succ évitants l'échec...
	   for (k=0; k < *n; k++) 
	   {
		i = T[k].xrN; j = T[k].yrN;
		// vérifier s'il est menacé dans la config T[k] ...
		if ( caseMenaceePar( MAX, i, j, T[k] ) ) {
		    T[k] = T[(*n)-1];	// alors supprimer T[k] de la liste des succ...
		    (*n)--;
		    k--;
		}
	   } // for k	
	} // if (mode == MAX) ... else ...

} // generer_succ



/***********************************************************************/
/*********** Partie:  AlphaBeta, Initialisation et affichahe ***********/
/***********************************************************************/
int comp (const void * elem1, const void * elem2) 
{
    struct config f = *((struct config *)elem1);
    struct config s = *((struct config *)elem2);
    if (f.val < s.val) return  1;
    if (f.val > s.val) return -1;
    return 0;
}

void trier_config_table(struct config T [], int n)
{
	int i;
	for (i = 0; i < n; i++)
		T[i].val = estim(T[i]);

	qsort(T, n, sizeof(T[0]), comp);
	return;
}


/* MinMax avec elagage alpha-beta + Hill Climbing*/
int minmax_ab2( struct config conf, int mode, int niv, int alpha, int beta, long * nb_noeuds, long * nb_coupes)
{
	
 	int n, i, score, score2;
 	struct config T[100];
	*nb_noeuds += 1;

   	if ( feuille(conf, &score) ) 
		return score;

   	if ( niv == 0 ) 
		return estim(conf);

   	if ( mode == MAX ) 
	{

	   generer_succ( conf, MAX, T, &n );
	   trier_config_table(T, n);
	  

	   score = alpha;
	   for ( i=0; i<n; i++ ) 
	   {
   	    	score2 = minmax_ab2( T[i], MIN, niv-1, score, beta, nb_noeuds, nb_coupes);
			if (score2 > score) score = score2;
			if (score > beta) 
			{
				// Coupe Beta
					*nb_coupes += 1;
					//printf("Beta %d\n", beta);
					return beta;   
			}
	   } 
	}
	else  
	{ // mode == MIN 

	   generer_succ( conf, MIN, T, &n );
	   trier_config_table(T, n);
	  
	   score = beta;
	   for ( i=n-1; i >=0; i--) 
	   {
   	    	score2 = minmax_ab2( T[i], MAX, niv-1, alpha, score, nb_noeuds, nb_coupes);
			if (score2 < score) score = score2;
			if (score < alpha) 
			{
				// Coupe Alpha
					*nb_coupes += 1;
					//printf("Alpha %d\n", alpha);
					return alpha;   
	    	}
	   }
	}


	return score;

} // minmax_ab

/* MinMax avec elagage alpha-beta */
int minmax_ab( struct config conf, int mode, int niv, int alpha, int beta, long * nb_noeuds, long * nb_coupes)
{
	
 	int n, i, score, score2;
 	struct config T[100];
	*nb_noeuds += 1;

   	if ( feuille(conf, &score) ) 
		return score;

   	if ( niv == 0 ) 
		return estim(conf);

   	if ( mode == MAX ) 
	{

	   generer_succ( conf, MAX, T, &n );

	   score = alpha;
	   for ( i=0; i<n; i++ ) 
	   {
   	    	score2 = minmax_ab( T[i], MIN, niv-1, score, beta, nb_noeuds, nb_coupes);
			if (score2 > score) score = score2;
			if (score > beta) 
			{
				// Coupe Beta
					*nb_coupes += 1;
					//printf("Beta %d\n", beta);
					return beta;   
			}
	   } 
	}
	else  
	{ // mode == MIN 

	   generer_succ( conf, MIN, T, &n );

	   score = beta;
	   for ( i=0; i<n; i++ ) 
	   {
   	    	score2 = minmax_ab( T[i], MAX, niv-1, alpha, score, nb_noeuds, nb_coupes);
			if (score2 < score) score = score2;
			if (score < alpha) 
			{
				// Coupe Alpha
					*nb_coupes += 1;
					//printf("Alpha %d\n", alpha);
					return alpha;   
	    	}
	   }
	}

	if ( score == +INFINI ) score = +100;
    if ( score == -INFINI ) score = -100;
	return score;

} // minmax_ab

int minmax( struct config conf, int mode, int niv)
{
 	int n, i, score, score2;
 	struct config T[100];

   	if ( feuille(conf, &score) ) 
		return score;

   	if ( niv == 0 ) 
		return estim( conf );

   	if ( mode == MAX ) {

	   generer_succ( conf, MAX, T, &n );

	   score = -INFINI;
	   for ( i=0; i<n; i++ ) 
	   {
   	    	score2 = minmax( T[i], MIN, niv-1);
			if (score2 > score) score = score2;
		
	   } 
	}
	else  { // mode == MIN 

	   generer_succ( conf, MIN, T, &n );

	   score = +INFINI;
	   for ( i=0; i<n; i++ ) {
   	    	score2 = minmax( T[i], MAX, niv-1);
		if (score2 < score) score = score2;
	
	   }
	}

        if ( score == +INFINI ) score = +100;
        if ( score == -INFINI ) score = -100;

	return score;

}
/* Intialise la disposition des pieces dans la configuration initiale conf */
void init( struct config *conf )
{
   	int i, j;

    	for (i=0; i<8; i++)
		for (j=0; j<8; j++)
			conf->mat[i][j] = 0;	// Les cases vides sont initialisées avec 0

	conf->mat[0][0] =  't'; conf->mat[0][1] =  'c'; conf->mat[0][2] = 'f'; conf->mat[0][3] =  'n';
	conf->mat[0][4] =  'r'; conf->mat[0][5] =  'f'; conf->mat[0][6] = 'c'; conf->mat[0][7] =  't';

	for (j=0; j<8; j++) {
		conf->mat[1][j] = 'p';
 		conf->mat[6][j] = -'p'; 
		conf->mat[7][j] = -conf->mat[0][j];
	}

	conf->xrB = 0; conf->yrB = 4;
	conf->xrN = 7; conf->yrN = 4;

	conf->roqueB = 'r';
	conf->roqueN = 'r';

	conf->val = 0;

} // init

/* Affiche la configuration conf */
void affich( struct config conf )
{
	int i, j, k;
	for (i=0;  i<8; i++)
		printf("\t  %c", i+'a');
   	printf("\n");

	for (i=0;  i<8; i++)
		printf("\t----- ");
   	printf("\n");

	for(i=8; i>0; i--)  {
		printf("    %d", i);
		for (j=0; j<8; j++)
			if ( conf.mat[i-1][j] < 0 ) {
				printf("\t %s-%c",KRED, -conf.mat[i-1][j]);
				printf("%s", KWHT);
			}
			else if ( conf.mat[i-1][j] > 0 ) {
				printf("\t%s +%c",KGRN,  conf.mat[i-1][j]);
				printf("%s", KWHT);
			}
				  else printf("\t  ");
		printf("\n");

		for (k=0;  k<8; k++)
			printf("\t----- ");
   		printf("\n");

	}
	printf("\n");

} // affich





/*******************************************/
/*********** Programme princiapl ***********/
/*******************************************/


int main( int argc, char *argv[] )
{
	char sy, dy, ch[10];
	int sx, dx, n, i, j, score, stop, cout, cout2, legal, hauteur, sauter;
	int cmin, cmax, mode, cpt;
   	double stats1[100];
	double stats2[100];
	long nb_noeuds1=0;
	long nb_noeuds2=0;
	long nb_coupes1 = 0;
	long nb_coupes2 = 0;


   struct config T[100], conf, conf1;

   if ( argc == 1 ) 
	hauteur = 5;  // par défaut on fixe la profondeur d'évaluation à 4
   else
	hauteur = atoi( argv[1] ); // sinon elle est récupérée depuis la ligne de commande

   printf("\n\nProfondeur d'exploration = %d\n\n", hauteur);

   // Initialise la configuration de départ
   init( &conf );
  
   

   // Boucle principale du dérouleùment d'une partie ...
   stop = 0;
   mode = MAX;
   
   struct timeval begin, end;
   int alpha, beta;
   double result;
   cpt = 0;
   nb_noeuds1=0;
   nb_noeuds2=0;
   long local_nb_noeuds1 = 0, local_nb_coupes1= 0, local_nb_coupes2 = 0, local_nb_noeuds2 = 0;

   while (!stop && (cpt < 50))
   {
	    alpha= -INFINI;
		beta= +INFINI;

		affich( conf );

		printf("%d\n", cpt);
		
		generer_succ(conf, mode, T, &n);
		
		score = -INFINI*mode;
		j = -1;
		//Timing
		gettimeofday(&begin, NULL);
		#pragma omp parallel private (local_nb_noeuds2, local_nb_coupes2, local_nb_noeuds1, local_nb_coupes1) 
		{
		
			#pragma omp for  schedule (dynamic) 
			for (i=0; i<n; i++) 
			{
				
				if (mode == MAX)
				{

					local_nb_coupes1 = 0;
					local_nb_noeuds1 = 0;
					//printf("thread %d iteration %d\nnb_noeuds: %ld \nlocal_nb_noeuds: %ld\n\n",omp_get_thread_num(),i, nb_noeuds1, local_nb_noeuds1);
					cout = minmax_ab(T[i], MIN, hauteur-1, alpha, beta, &local_nb_noeuds1, &local_nb_coupes1);
					#pragma omp critical
					{
						if (cout > score) 
						{  		
							alpha =  cout;
							score = cout;
							j = i;
						}
						nb_noeuds1 += local_nb_noeuds1;
						nb_coupes1 += local_nb_coupes1;
						
					}
				//printf("thread %d iteration %d\nnb_noeuds: %ld \nlocal_nb_noeuds: %ld\n\n",omp_get_thread_num(),i, nb_noeuds1, local_nb_noeuds1);

				}
				else
				{
					local_nb_coupes2 = 0;
					local_nb_noeuds2 = 0;
					cout = minmax_ab2(T[i], MAX, hauteur-1, alpha, beta, &local_nb_noeuds2, &local_nb_coupes2);
					
					#pragma omp critical
					{
						if ( cout < score ) 
						{  
							beta = cout;
							score = cout;
							j = i;
						}
						nb_coupes2 += local_nb_coupes2;
						nb_noeuds2 += local_nb_noeuds2;
					}
					
				}
				
			}
		}
		gettimeofday(&end, NULL);
		result = (double)(end.tv_usec - begin.tv_usec)/1000000 + end.tv_sec - begin.tv_sec;
		if (mode == MAX)
			stats1[cpt] = result;
		else
		{
			stats2[cpt] = result;
			cpt++;
		}
		//Fin timing 2


		if ( j != -1 ) 
		{ // jouer le coup et aller à la prochaine itération ...
			copier( &T[j], &conf );
			conf.val = score;
			//printf("score: %d\n", conf.val);
		}
		else 
		{ // S'il n'y a pas de successeur possible, l'ordinateur à perdu
			printf(" *** J'ai perdu ***\n");
			stop = 1;
		}
	   mode *= -1;
	  
   } // while
   
	int iiii;
	char res[30];
	FILE * f = fopen("results.txt", "w");

	snprintf(res, 30, "%ld", nb_noeuds1);
	fputs(res, f);
	fputs("\n", f);

	snprintf(res, 30, "%ld", nb_noeuds2);
	fputs(res, f);
	fputs("\n", f);


	for (iiii = 0;iiii < cpt; iiii++)
	{
		snprintf(res, 30, "%f", stats1[iiii]);
		fputs(res, f);
		fputs(" ", f);
	}

	fputs("\n", f);
	for (iiii = 0;iiii < cpt; iiii++)
	{
		snprintf(res, 30, "%f", stats2[iiii]);
		fputs(res, f);
		fputs(" ", f);
	}

	fclose(f);

	return 0;
}


