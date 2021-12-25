// Fichier list.c
#include "list.h"

// Pour plus de propreté des concepts,
// on définit l'identifier NIL (signifiant "fin de liste")
// plutôt que d'utiliser directement NULL (signifiant "pointeur nul")
#define NIL NULL

list_t list_new() {
  return NIL;
}

int list_is_empty( list_t l ) {
  return NIL == l;
}

// Precondition : liste non vide
element_t list_first(list_t l){
  assert(!list_is_empty(l));
  return l->val;
}

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

void list_print(list_t l) {
  list_t p;
  printf("( ");
  for ( p=l; ! list_is_empty(p); p = p->next) {
    element_print( p->val );
    printf( " " );
  }
  printf(")\n");
}

int list_length(list_t l) {
  int len = 0;
  list_t p;
  for( p=l; ! list_is_empty(p) ; p=p->next ) {
    len ++;
  }
  return len;
}

list_t list_find(element_t e, list_t l) {
  list_t p;
  for( p=l; ! list_is_empty(p) ; p=p->next ) {
    if( element_equal(&(p->val), &e) ) {
      return p;
    }
  }
  return NULL;
}


int list_count(element_t e, list_t l) {
  //compteur initialise
  int compteur = 0 ;

  //parcours de la liste
  list_t p ;
  for( p=l ; p != NIL ; p = p -> next ) {
    if (element_equal( &p -> val , &e ) == 1 ) compteur ++ ;
  }
  return compteur ;
}

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

list_t list_delete(list_t l) {
  //on balaye la liste et on libre avec list_del_first
  while (l != NIL) {
    l = list_del_first (l) ;
  }
  return l ;
}

list_t list_concat(list_t l1, list_t l2) {
  //on balaye tout et le pointeur pointant sur NIL de l1 va mointenant pointer sur le debut de l2
  list_t p ;
  for ( p=l1 ; p -> next != NIL ; p = p -> next ) {}
  p -> next = l2 ;

  return l1 ;
  //les listes l1 et l2 deviennent une seule liste
}

list_t list_copy(list_t l) {
  //creation d'une nouvelle liste
  list_t copie ;
  copie = list_new() ;
  // on va faire une boucle de balayge et utiliser list_add_last pour ajouter chaque nouvelle valeur a la copie

  element_t elem ;
  list_t p ;

  for ( p=l ; p != NIL ; p = p -> next) {

    //ajout de l'element a la copie
    copie = list_add_last ( p -> val , copie ) ;

  }
  return copie ;
}

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

  /*  on se deplace a la bonne position,la premiere valeur de element doit etre
  inferieur a la premiere dans la liste */

  /*on va utiliser un compteur pour pouvoir venir replacer la valeur a sa place :
  un cran avant que l'on s'arrete */
  int compteur = 0 ;

  while ( ((*p).val).x > e.x  ) {
    p = p -> next ;
    compteur ++ ;

    if (p == NIL) {
      list_add_last(e , l) ;
      return l ;
    }
  }
  /* on se deplace sur la deuxieme valeur */
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
