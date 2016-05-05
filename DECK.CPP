// deck.cpp
// By Tom Weatherhead			September 11, 1994

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deck.h"


// **** Card_Class members


char * Card_Class::suit_name( void )
{
    char * rtn;

    switch( suit ) {
	case HEART:	rtn = "Hearts";		break;
	case DIAMOND:	rtn = "Diamonds";	break;
	case SPADE:	rtn = "Spades";		break;
	case CLUB:	rtn = "Clubs";		break;
	default:	rtn = "";		break;
    }

    return( rtn );
}


char * Card_Class::print_card( void )
{
    static char rtn_str[15];
    unsigned int len = 1;

    switch( face ) {
	case ACE:	strcpy( rtn_str, "A" );	break;
	case JACK:	strcpy( rtn_str, "J" );	break;
	case QUEEN:	strcpy( rtn_str, "Q" );	break;
	case KING:	strcpy( rtn_str, "K" );	break;
	default:
	    len = sprintf( rtn_str, "%d", face );
	    break;
    }

    // replace w. strcat
    sprintf( rtn_str + len, " %s", suit_name() );
    return( rtn_str );
}


unsigned int Card_Class::face_value( void )
{
//    return( min( (unsigned int)face, 10 ) );
    return( (face >= 10) ? 10 : (unsigned int)face );
}


// **** Hand_Class members ****


Hand_Class::Hand_Class( unsigned int max_cards, bool is_crib_parm )
{
    card = (Card_Class **)malloc( max_cards * sizeof(Card_Class *) );
    is_crib = is_crib_parm;
    size = 0;
}


void Hand_Class::init( void )
{
    size = 0;
}


void Hand_Class::add_card( Card_Class * card_parm )
{
    card[size++] = card_parm;
}


void Hand_Class::print( bool on_one_line )
{
    unsigned int i;

    for( i = 0; i < size; i++ ) {

	if( on_one_line ) {
	    printf( "%s   ", card[i]->print_card() );
	} else {
	    printf( "%d:  %s\n", i, card[i]->print_card() );
	}
    }

    printf( "\n" );
}


Card_Class * Hand_Class::extract_card( unsigned int card_num )
{
    Card_Class * rtn = card[card_num];

    if( card_num < --size ) {
	card[card_num] = card[size];
    }

    return( rtn );
}


void Hand_Class::count_faces( unsigned int face_count[] )
{
    unsigned int i;


    // Dangerous hard-coded const
    memset( face_count, 0, 14 * sizeof( unsigned int ) );

    for( i = 0; i < size; i++ ) {
	face_count[card[i]->face]++;
    }
}


Hand_Class::~Hand_Class( void )
{
    free( card );
}


// **** Deck_Class members ****


Deck_Class::Deck_Class( void )
{
    unsigned int i;

    for( i = 0; i < DECK_SIZE; i++ ) {
	card[i].suit = (suit_type)(i % 4);
	card[i].face = (face_type)((i / 4) + 1);
    }
}


void Deck_Class::shuffle( void )
{
    unsigned int i, j;
    Card_Class temp;

    card_idx = 0;
    srand( time( NULL ) );

    for( i = 1; i < DECK_SIZE; i++ ) {
	j = rand() % i;
	temp = card[i];
	card[i] = card[j];
	card[j] = temp;
    }
}


Card_Class * Deck_Class::deal_one( void )
{
    return( &card[card_idx++] );
}
