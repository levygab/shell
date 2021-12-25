/*****************************************************
 * Copyright Grégory Mounié 2008-2015                *
 *           Simon Nieuviarts 2002-2009              *
 * This code is distributed under the GLPv3 licence. *
 * Ce code est distribué sous la licence GPLv3+.     *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "variante.h"
#include "readcmd.h"
#include "list.h"

#include <sys/wait.h>
#include <sys/types.h>

#include <unistd.h> //pour le tube

#include <sys/stat.h>
#include <fcntl.h>




#ifndef VARIANTE
#error "Variante non défini !!"
#endif

/* Guile (1.8 and 2.0) is auto-detected by cmake */
/* To disable Scheme interpreter (Guile support), comment the
 * following lines.  You may also have to comment related pkg-config
 * lines in CMakeLists.txt.
 */

#if USE_GUILE == 1
#include <libguile.h>

int question6_executer(char *line)
{
	/* Question 6: Insert your code to execute the command line
	 * identically to the standard execution scheme:
	 * parsecmd, then fork+execvp, for a single command.
	 * pipe and i/o redirection are not required.
	 */
	printf("Not implemented yet: can not execute %s\n", line);

	/* Remove this line when using parsecmd as it will free it */
	free(line);

	return 0;
}

SCM executer_wrapper(SCM x)
{
        return scm_from_int(question6_executer(scm_to_locale_stringn(x, 0)));
}
#endif


void terminate(char *line) {
#if USE_GNU_READLINE == 1
	/* rl_clear_history() does not exist yet in centOS 6 */
	clear_history();
#endif
	if (line)
	  free(line);
	printf("exit\n");
	exit(0);
}




int main() {
        printf("Variante %d: %s\n", VARIANTE, VARIANTE_STRING);

#if USE_GUILE == 1
        scm_init_guile();
        /* register "executer" function in scheme */
        scm_c_define_gsubr("executer", 1, 0, 0, executer_wrapper);
#endif

//variable pour stocker les commandes passées
list_t stock = list_new() ;
int k  ;
int wstatus = 0;
int retour ;
char jobs = 0 ;
int tube[2] ; //creation du tuyau : varaible servant pour le pipe
int pid_pipe ;



	while (1) {
		jobs = 0 ;
		struct cmdline *l;
		char *line=0;
		int i, j;
		char *prompt = "ensishell>";

		/* Readline use some internal memory structure that
		   can not be cleaned at the end of the program. Thus
		   one memory leak per command seems unavoidable yet */
		line = readline(prompt);
		if (line == 0 || ! strncmp(line,"exit", 4)) {
			terminate(line);
		}




		else if (strncmp(line, "jobs",4) == 0 ) {
			//affiche les commande en memoire
			list_t pa = stock ; //pointeur pour se deplacer
			list_t pp = stock ;  //pointeur pour stocker

			while (pa != NULL) {
				k = 0 ; // variable servant a savoir si on traite le premier element de la chaine
				//on regarde si le processus est envore en vie :
				retour = waitpid( (pa -> val).num ,&wstatus, 0|WNOHANG) ;



				if (retour == (pa -> val).num) { //cas ou le processus est fini : on previent l'utilisateur puis on le supprime
					printf("commande %s est finie. Son pid est %d.\n", (pa -> val).nom , (pa -> val).num ); //previent utilisateur

					if (k == 0) { //on traite le premier element,
						pa = pa -> next ; //on avance pa sur le deuxieme element
						free (pp) ; //on libere le premier element
						pp = pa ; //pp revient a pa
						stock = pa ; //stock doit lui aussi pointer sur le premier element
						//on a dc on nv premier element
					}

					else {
					pp -> next = pa -> next ; //on fait le changement de next
					free(pa) ; //on libere pa
					pa = pp -> next ; //on lui redonne la valeur pour le prochain tour de boucle
					}
				}

				else { //cas ou le processus n'est pas finit : on previent l'utilisateur, puis on se deplace
					printf("commande %s n'est pas finie. Son pid est %d.\n", (pa -> val).nom , (pa -> val).num );

					if (k == 0) { //si on est sur le premier element, on doit juste avance pa
						pa = pa -> next ;
						k ++ ;
					}

					else { //on est pas sur le premier element, on avance pa et pp
						pa = pa -> next ;
						pp = pp -> next ;

					}

				}

			}
			jobs = 1 ;
		}




#if USE_GNU_READLINE == 1
		add_history(line);
#endif


#if USE_GUILE == 1
		/* The line is a scheme command */
		if (line[0] == '(') {
			char catchligne[strlen(line) + 256];
			sprintf(catchligne, "(catch #t (lambda () %s) (lambda (key . parameters) (display \"mauvaise expression/bug en scheme\n\")))", line);
			scm_eval_string(scm_from_locale_string(catchligne));
			free(line);
                        continue;
                }
#endif

		/* parsecmd free line and set it up to 0 */
		l = parsecmd( & line);

		/* If input stream closed, normal termination */
		if (!l) {

			terminate(0);
		}



		if (l->err) {
			/* Syntax error, read another command */
			printf("error: %s\n", l->err);
			continue;
		}

		if (l->in) printf("in: %s\n", l->in);
		if (l->out) printf("out: %s\n", l->out);
		if (l->bg) printf("background (&)\n");

		/* Display each command of the pipe */
		for (i=0; l->seq[i]!=0; i++) {
			char **cmd = l->seq[i];
			printf("seq[%d]: ", i);
                        for (j=0; cmd[j]!=0; j++) {
                                printf("'%s' ", cmd[j]);
                        }
			printf("\n");
		}
		//fais tourner la commande
		if (jobs == 0) {

			pid_t pro_id = fork() ;
			if (pro_id <= -1) printf("erreur (dans le fork)\n" );
			if (pro_id == 0) { //on doit utiliser le execvp, c'est le fils

				if (l -> seq[1] != NULL) {
					pipe (tube) ; //creation du tuyau
					//gestion du pipe

					//on commence par creer un nv processus
					pid_pipe = fork() ;

					if (pid_pipe > 0) { //c'est le pere dans le nv processus


						close(tube[0]) ; //on ferme la lecture
						dup2(tube[1], 1) ; //on relie stdout a tube [1] qui est l'entre
						close(tube[1]) ; //on ferme tube[1]

						execvp(l -> seq[0][0],l -> seq[0]) ; //on execute la premiere commande
					}

					if (pid_pipe == 0) { //c'est le fils dans le nouveau processus

						close (tube[1]) ;
						dup2(tube[0],0) ; //on relie stdin a tube[0] qui est la sortie
						close(tube[0]) ; //on ferme tube[0]

						execvp( l -> seq[1][0], l -> seq[1]) ; //on execute
					}


			}

			else {

				//cas ou on a une entree : l-> in ==1
				if (l -> in ) {
					int entree ;
					entree = open(l -> in ,0, "rt") ; //on ouvre le fichier a lire en read car on ne va pas ecrire dedans
					dup2(entree, 0) ; //on remplace stdin par le fichier
					//close(entree) ; //on ferme
				}

				if (l -> out) {
					int sortie = open( l -> out , O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO , "wt") ; //on ouvre le fichier dans lequel on veut ecrire
					if (sortie == -1) printf("pb de open\n");
					retour = ftruncate(sortie,0) ;
					dup2(sortie, 1) ; //on remplace stdout par le fichier de dortie
					//close (sortie) ;
					//printf("je suis sortie apres le close\n" );
				}
				execvp( l -> seq[0][0], l -> seq[0]) ; //pas de pipe : on execute la commande habituelle qu'on execute qu'il y ait des fichiers entree et sortie ou non
			}

			}



				if (pro_id > 0 ) { //c'est le pere son pro_id a la valeur du pid de son fils. cette partie s'occupe aussi du papa de la deuxieme commande en cas de pipe


					//gestion de la sauvegarde des mises en fond
						if ( l -> bg) {
							//on commence par creer un nouvel element
							element_t elem ;
							strcpy(elem.nom , l -> seq[0][0]) ; //on stock la commande dans l'element
							elem.num = pro_id ; //pareil pour le pid

							//on ajoute l'elem a la liste
							stock = list_add_first(elem, stock) ;
						}
						if (! l->bg ) {
							waitpid(pro_id, &wstatus , 0 ) ; //pas de pipe : on le fait attendre son fiston
					}
				}
	}
	}


}



//on fct sur les element
void element_print (element_t elem) {
  printf("son nom est %s, son pid est %d\n",elem.nom, elem.num );
}


//fonction faite l'annee derniere servant a travailler sur les listes
#define NIL NULL
#include <assert.h>
list_t list_new() {
  return NIL;
}

int list_is_empty( list_t l ) {
  return NIL == l;
}

/*
// Precondition : liste non vide
element_t list_first(list_t l){
  assert(!list_is_empty(l));
  return l->val;
}
*/
list_t list_add_first( element_t e, list_t l ) {
  list_t p = calloc( 1, sizeof( *p ) );
  if ( NULL == p ) {
    fprintf( stderr, "Fatal: Unable to allocate new list link.\n" );
    return l;
  }
  p->val  = e;
  p->next = l;
  return p;
}

// Precondition : liste non vide
list_t list_del_first( list_t l ) {
  assert(!list_is_empty(l));

  list_t p = l->next;
  free( l );
  return p;
}
/*
void list_print(list_t l) {
  list_t p;
  printf("( ");
  for ( p=l; ! list_is_empty(p); p = p->next) {
    element_print( p->val );
    printf( " " );
  }
  printf(")\n");
}
*/

int list_length(list_t l) {
  int len = 0;
  list_t p;
  for( p=l; ! list_is_empty(p) ; p=p->next ) {
    len ++;
  }
  return len;
}

/*
list_t list_add_last(element_t e, list_t l) {
  //creation de l'ajout
  list_t ajout ;
  ajout = list_new() ;

  //on ajoute e au debut de l'ajout, comme ca on a juste a remplacer NIL par ajout dans l
  ajout = list_add_first ( e, ajout ) ;

  //on verifie que l ne soit pas vide, si elle l'est, on return ajout
  if ( list_is_empty(l) == 1 ) {
    return ajout ;
  }

  //parcours de la liste l, on s'arrete juste avant que p = NIL
  list_t p ;
  for( p=l ; p -> next != NIL ; p = p-> next ) {}

  p -> next = ajout ;

  return l ;
}
*/
/*
list_t list_delete(list_t l) {
  //on balaye la liste et on libre avec list_del_first
  while (l != NIL) {
    l = list_del_first (l) ;
  }
  return l ;
}
*/
/*
list_t list_concat(list_t l1, list_t l2) {
  //on balaye tout et le pointeur pointant sur NIL de l1 va mointenant pointer sur le debut de l2
  list_t p ;
  for ( p=l1 ; p -> next != NIL ; p = p -> next ) {}
  p -> next = l2 ;

  return l1 ;
  //les listes l1 et l2 deviennent une seule liste
}
*/
/*
list_t list_remove_n(int n, list_t l) {
  //verif coherence
  int longueur ;
  longueur = list_length(l) ;
  if (n < 1 || n > longueur ) return l ;

  //cas n=1
  if (n==1){
      list_t p ;
      p = list_new() ;
      p = l -> next ;
      free (l) ;
      return p ;
      //return l -> next ;
  }  //

  //cas ou on enelve une autre valeur
  //p2 sera pour liberer la memoire
  list_t p = l ;
  list_t p2 = l;
  int i ;

  //on deplace p sur n-2, un cran avant la valeur a enlever
  for ( i = 0 ; i < n-2 ; i++ ) {
    p = p -> next ;
    p2 = p2 -> next ;
  }
  // sur n-2, on se deplace de deux valeur
  p2 = p2 -> next ; //on deplace p2 sur la valeur a enlver
  p -> next = p2 -> next ; //p -> next prend la valeur apres la valeur a enlever

  //on libere
  free ( p2) ;

  return l ;
}
*/
/*
list_t list_add_sort(element_t e, list_t l) {
  //cas ou l est vide
  if (list_is_empty(l) == 1) {
    l = list_add_first( e , l ) ;
    return l ;
    }
  //si l n'est pas vide
  //pointeur pour se deplace
  list_t p = l ;
  list_t p2 = l ;

  //il faut transformer l'element en list_t
  list_t  ajout ;
  ajout = calloc (1,sizeof (*ajout)) ;
  ajout -> val = e ;

    on se deplace a la bonne position,la premiere valeur de element doit etre
  inferieur a la premiere dans la liste

  on va utiliser un compteur pour pouvoir venir replacer la valeur a sa place :
  un cran avant que l'on s'arrete

  int compteur = 0 ;

  while ( ((*p).val).x > e.x  ) {
    p = p -> next ;
    compteur ++ ;

    if (p == NIL) {
      list_add_last(e , l) ;
      return l ;
    }
  }
   on se deplace sur la deuxieme valeur
  while ( ((*p).val).y > e.y ) {
    p = p -> next ;
    compteur ++ ;

    if (p == NIL) {
      list_add_last(e , l) ;
      return l ;
      }
  }

  //on vient deplacer p2 jusqu'a la valeur necessaire
  if (compteur == 0 ) {
    ajout -> next = l ;
    return ajout ;
  }
  //on se deplace jusqu'a la bonne valeur
  int i ;
  for (i = 0 ; i < compteur-1 ; i++ ) {
    p2 = p2 -> next ;
  }

  //on change les pointages
  p2 -> next = ajout ;
  ajout -> next = p ;


  return l ;
}
*/
