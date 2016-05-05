// cribbage.cpp
// By Tom Weatherhead			September 11, 1994

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "deck.h"

#define COUNT_LIMIT		31
#define NUM_DISCARDS_PER_PLAYER	2
#define TWO_HANDS		2*HAND_SIZE
#define WIN_SCORE		121

typedef struct {
    Hand_Class * hand;
    unsigned int score;
    char * name;
    bool go_flag;
} player_data_type;

static Card_Class * upcard;


static unsigned int runs( Hand_Class * hand )
{
    unsigned int face_count[14], i, begin, end, value, total = 0;

    hand->count_faces( face_count );
    begin = end = 0;

    for( i = 1; i <= 13; i++ ) {

	if( face_count[i] > 0 ) {

	    if( begin == 0 ) {
		begin = i;
	    }

	} else if( begin > 0 ) {

	    if( i - begin >= 3 ) {
		end = i - 1;
		break;
	    } else {
		begin = 0;
	    }
	}
    } // for

    if( end == 0  &&  begin > 0  &&  begin < 12 ) {
	end = 13;
    }

    if( begin > 0  &&  end > 0 ) {
	value = end - begin + 1;

	for( i = begin; i <= end; i++ ) {
	    value *= face_count[i]; // for multiple runs
	}

	total += value;
    }

    return( total );
} // runs()


static unsigned int multiples( Hand_Class * hand )
{
    unsigned int face_count[14], i, total = 0;

    hand->count_faces( face_count );

    for( i = 1; i <= 13; i++ ) {

	switch( face_count[i] ) {
	    case 2: total += 2;		break;
	    case 3: total += 6;		break;
	    case 4: total += 12;	break;
	}
    }

    return( total );
}


static unsigned int eval_15s_in_subset( Card_Class * subset[],
  unsigned int subset_size )
{
    unsigned int total = 0, i;

    for( i = 0; i < subset_size; i++ ) {
	total += subset[i]->face_value();
    }

    return( (total == 15) ? 2 : 0 );
}


static unsigned int add_card_to_subset( Hand_Class * src_hand,
  unsigned int src_base_idx, Card_Class * subset[HAND_SIZE + 1],
  unsigned int subset_idx, unsigned int max_subset_size )
{
    unsigned int i, value = 0;

    for( i = src_base_idx; i < src_hand->size; i++ ) {
	subset[subset_idx] = src_hand->card[i];
	value += eval_15s_in_subset( subset, subset_idx + 1 );

	if( subset_idx < max_subset_size  &&  i < src_hand->size - 1 ) {
	    value += add_card_to_subset( src_hand, i + 1, subset,
	      subset_idx + 1, max_subset_size );
	}
    }

    return( value );
} // add_cards_to_subset()


static unsigned int find_15s( Hand_Class * hand )
{
    Card_Class * subset[HAND_SIZE + 1];

    return( add_card_to_subset( hand, 0, subset, 0, HAND_SIZE ) );
}


static unsigned int jack_of_upcard( Hand_Class * hand )
{
    unsigned int i;

    if( upcard == NULL ) return( 0 );

    for( i = 0; i < 4; i++ ) {

	if( hand->card[i]->suit == upcard->suit
	  && hand->card[i]->face == JACK ) return( 1 );
    }

    return( 0 );
} // jack_of_upcard()


static unsigned int same_suit( Hand_Class * hand )
{
    unsigned int i, limit = (hand->is_crib) ? 5 : 4;
    suit_type suit0 = hand->card[0]->suit;

    for( i = 1; i < hand->size; i++ ) {

	if( hand->card[i]->suit != suit0 ) break;
    }

    return( (i >= limit) ? i : 0 );
} // same_suit()


static unsigned int eval_hand( Hand_Class * hand )
{
    return( find_15s( hand ) + runs( hand ) + multiples( hand )
      + jack_of_upcard( hand ) + same_suit( hand ) );
} // eval_hand()


static void choose_discards( Hand_Class * hand, Hand_Class * crib )
{
    unsigned int i, j, k, cardnum, value, maxvalue = 0,
      discard1 = 0, discard2 = 1;
    Hand_Class testhand( DEAL_SIZE, FALSE );

    // Evaluate each 4-element subset of hand
    // Use subset operations in future

    for( i = 0; i < DEAL_SIZE - 1; i++ ) {

	for( j = i + 1; j < DEAL_SIZE; j++ ) {
	    testhand.init();

	    for( k = 0; k < DEAL_SIZE; k++ ) {

		if( k == i  ||  k == j ) continue;

		testhand.add_card( hand->card[i] );
	    }

	    if( ( value = eval_hand( &testhand ) ) > maxvalue ) {
		maxvalue = value;
		discard1 = i;
		discard2 = j;
	    }
	}
    }

    crib->add_card( hand->extract_card( discard2 ) ); // must be first
    crib->add_card( hand->extract_card( discard1 ) );
} // choose_discards()


static unsigned int player_choose_card( Hand_Class * hand )
{
    unsigned int i;

    hand->print( FALSE );

    do {
	printf( "Enter card number: " );
	scanf( "%d", &i );
    } while( i >= hand->size );

    return( i );
} // player_choose_card()


static unsigned int played_card_effect( Card_Class * card, Hand_Class * played,
  unsigned int played_idx_base, unsigned int count )
{
    int j, up_bound, low_bound;
    unsigned int card_effect = 0, k, run_val = 0, contig[14], num_contig_cards;
    Hand_Class temp_hand( TWO_HANDS, FALSE );

    played->add_card( card );
    count += card->face_value();

    // Calculate any points to peg

    if( count == 15  ||  count == COUNT_LIMIT ) {
	card_effect += 2;
    }

    // Runs
    // -2 because a run contains >= 3 cards

    for( j = played_idx_base; j < played->size-2; j++ ) {
	// Is played[j]...played[played->size-1] a run?
	// Test for contiguity

	memset( contig, 0, 14 * sizeof(unsigned int) );

	for( k = j; k < played->size; k++ ) {
	    contig[played->card[k]->face]++;
	}
#if 1
	for( low_bound = 1; low_bound <= 13  &&  contig[low_bound] == 0;
	  low_bound++ );

	for( up_bound = 13; up_bound >= 1  &&  contig[up_bound] == 0;
	  up_bound-- );

	for( k = low_bound; k <= up_bound  &&  contig[k] > 0; k++ );

        if( k <= up_bound ) continue; // not contiguous
#else
	num_contig_cards = played->size - j;

	for( ; contig[k] == 1  &&  --num_contig_cards > 0; k++ );

	if( num_contig_cards != 0 ) continue;
#endif
	temp_hand.size = played->size - j;
        memcpy( temp_hand.card, &played->card[j],
	  temp_hand.size * sizeof( Card_Class * ) );
	run_val = runs( &temp_hand );
	card_effect += run_val;
	break;
    } // for

    // Multiples

    if( run_val == 0 ) {
	// Cards in multiple must be contiguous

	for( j = played->size - 2, k = 1; j >= 0  &&  j >= played_idx_base;
	  j--, k++ ) {

	    if( played->card[j]->face != card->face ) break;
	}

	switch( k ) {
	    case 2: card_effect += 2; break;
	    case 3: card_effect += 6; break;
	    case 4: card_effect += 12; break;
	}
    }

    if( played->size == TWO_HANDS ) {
	// One for last card
	card_effect++;
    }

    played->extract_card( played->size - 1 );
    return( card_effect );
} // played_card_effect();


static void reveal_and_peg( player_data_type * crib_owner,
  player_data_type * not_crib_owner )
{
    int j;
    unsigned int k, played_idx_base = 0, card_effect, cardnum, count = 0,
      max_card_effect;
    Card_Class * card;
    player_data_type * up, * not_up, * temp;
    Hand_Class pick_hand( DEAL_SIZE, FALSE ), played( TWO_HANDS, FALSE ),
      owner_temp( DEAL_SIZE, FALSE ), not_owner_temp( DEAL_SIZE, FALSE );

    for( j = 0; j < crib_owner->hand->size; j++ ) {
	owner_temp.add_card( crib_owner->hand->card[j] );
    }

    for( j = 0; j < not_crib_owner->hand->size; j++ ) {
	not_owner_temp.add_card( not_crib_owner->hand->card[j] );
    }

    up = not_crib_owner;
    not_up = crib_owner;

    while( played.size < TWO_HANDS ) {
	// Determine if "up" can go
	pick_hand.init();

	for( j = 0; j < up->hand->size; j++ ) {

	    if( up->hand->card[j]->face_value() <= COUNT_LIMIT - count ) {
		pick_hand.add_card( up->hand->card[j] );
	    }
	}

	if( pick_hand.size == 0 ) { // ie. can't play any card

	    if( up->go_flag ) {
		printf( "%s has completed his 'go'\n", up->name );
		up->go_flag = FALSE;
		played_idx_base = played.size;
		count = 0;
	    } else if( not_up->hand->size > 0 ) { // Other player goes
		printf( "%s: 'go'\n", up->name );

		if( ++not_up->score >= WIN_SCORE ) return;

		not_up->go_flag = TRUE;
            } else {
		printf( "(Counter reset without 'go')\n" );
		played_idx_base = played.size;
		count = 0;
	    }
	} else { // Choose card to throw down

	    if( up->name[0] == 'P' ) { // Player chooses card
		printf( "\nPlayable cards in your hand:\n" );
		j = player_choose_card( &pick_hand );
		card_effect = played_card_effect( pick_hand.card[j], &played,
		  played_idx_base, count );
	    } else { // Computer chooses card
		j = 0;
		max_card_effect = 0;

		for( k = 0; k < pick_hand.size; k++ ) {
		    card_effect = played_card_effect( pick_hand.card[k],
		      &played, played_idx_base, count );

		    if( card_effect > max_card_effect ) {
			j = k;
			max_card_effect = card_effect;
		    }
		}

		card_effect = max_card_effect;
	    } // if

	    // Find idx of selected card in temp hand

	    for( cardnum = 0; pick_hand.card[j] != up->hand->card[cardnum];
	      cardnum++ );

	    card = up->hand->extract_card( cardnum );
	    printf( "%s plays %s\n", up->name, card->print_card() );
	    played.add_card( card );
	    count += card->face_value();

	    if( card_effect == 0 ) {
		printf( "  %d\n", count );
	    } else {
		printf( "  %d for %d\n", count, card_effect );
	    }

	    if( (up->score += card_effect) >= WIN_SCORE ) return;

	    if( count == COUNT_LIMIT ) {
		played_idx_base = played.size;
		count = 0;

		if( up->go_flag ) {
		    up->go_flag = FALSE;
		    printf( "%s has completed his 'go'\n", up->name );
		}
	    }
	}

	if( not_up->hand->size > 0  &&  !up->go_flag ) {
	    temp = up;
	    up = not_up;
	    not_up = temp;
	}
    }

    crib_owner->hand->init();
    not_crib_owner->hand->init();

    for( j = 0; j < owner_temp.size; j++ ) {
	crib_owner->hand->add_card( owner_temp.card[j] );
    }

    for( j = 0; j < not_owner_temp.size; j++ ) {
	not_crib_owner->hand->add_card( not_owner_temp.card[j] );
    }

} // reveal_and_peg()


static void game( void )
{
    unsigned int i, cardnum, hand_val;
    Hand_Class crib( DEAL_SIZE, TRUE );
    player_data_type * crib_owner, * not_crib_owner, player, computer, * temp;
    Deck_Class Deck;

    player.hand = new Hand_Class( DEAL_SIZE, FALSE );
    computer.hand = new Hand_Class( DEAL_SIZE, FALSE );
    player.score = computer.score = 0;
    player.name = "Player";
    computer.name = "Computer";
    player.go_flag = computer.go_flag = FALSE;
    crib_owner = &player;
    not_crib_owner = &computer;

    for( ; ; ) {
	printf( "----------------------------------------\n" );
	printf( "%s owns crib\n", crib_owner->name );
	upcard = NULL;

	// Deal
	Deck.shuffle();
	not_crib_owner->hand->init();
	crib_owner->hand->init();
	crib.init();

	for( i = 0; i < DEAL_SIZE; i++ ) {
	    not_crib_owner->hand->add_card( Deck.deal_one() );
	    crib_owner->hand->add_card( Deck.deal_one() );
	}

	// Put discards into crib

	for( i = 0; i < NUM_DISCARDS_PER_PLAYER; i++ ) {
	    printf( "\nYour hand:\n" );
	    cardnum = player_choose_card( player.hand );
	    crib.add_card( player.hand->extract_card( cardnum ) );
	}

	choose_discards( computer.hand, &crib );

	// Select upcard
	upcard = Deck.deal_one();
	printf( "\nUpcard drawn: %s\n", upcard->print_card() );

	if( upcard->face == JACK ) {
	    crib_owner->score += 2;

	    if( crib_owner->score >= WIN_SCORE ) {
		// Can't win by turning a jack
		crib_owner->score = WIN_SCORE - 1;
	    }
	}

	// Lay 'em down, one at a time
	reveal_and_peg( crib_owner, not_crib_owner );

	if( crib_owner->score >= WIN_SCORE
	  ||  not_crib_owner->score >= WIN_SCORE ) break;

	// Prelude to scoring
	printf( "\nUpcard is %s\n", upcard->print_card() );

	// Score non-crib owner
	printf( "%s's hand: ", not_crib_owner->name );
	not_crib_owner->hand->print( TRUE );
	not_crib_owner->hand->add_card( upcard );
	hand_val = eval_hand( not_crib_owner->hand );
	printf( "  Value: %d\n", hand_val );

	if( (not_crib_owner->score += hand_val) >= WIN_SCORE ) break;

	// Score crib owner
	printf( "%s's hand: ", crib_owner->name );
	crib_owner->hand->print( TRUE );
	crib_owner->hand->add_card( upcard );
	hand_val = eval_hand( crib_owner->hand );
	printf( "  Value: %d\n", hand_val );

	if( (crib_owner->score += hand_val) >= WIN_SCORE ) break;

	printf( "Crib: " );
	crib.print( TRUE );
	crib.add_card( upcard );
	hand_val = eval_hand( &crib );
	printf( "  Value: %d\n", hand_val );

	if( (crib_owner->score += hand_val) >= WIN_SCORE ) break;

	printf( "Player's score: %d\n", player.score );
	printf( "Computer's score: %d\n", computer.score );

	temp = crib_owner;
	crib_owner = not_crib_owner;
	not_crib_owner = temp;
    } // for

    printf( "The %s wins!\n", (player.score >= WIN_SCORE)
      ? "player" : "computer" );
    printf( "Player's score: %d\n", player.score );
    printf( "Computer's score: %d\n", computer.score );
    delete computer.hand;
    delete player.hand;
} // game()


void main( void )
{
    char c;

    do {
	game();

	printf( "\nAnother game? (y/n): " );
	fflush( stdin );

	do {
	    c = getchar();
	} while( c != 'y'  &&  c != 'n' );

    } while( c == 'y' );

} // main()
