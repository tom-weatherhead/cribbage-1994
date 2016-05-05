// deck.h
// By Tom Weatherhead			September 11, 1994

#ifndef NULL
#define NULL 0
#endif

#undef FALSE
#undef TRUE
typedef enum { FALSE, TRUE } bool;

#define DECK_SIZE		52
#define DEAL_SIZE		6
#define HAND_SIZE		4
#define ACE			1
#define JACK			11
#define QUEEN			12
#define KING			13

enum {
    HEART,
    DIAMOND,
    SPADE,
    CLUB
};

typedef unsigned short suit_type;
typedef unsigned short face_type;

class Card_Class {
    public:
	suit_type suit;
	face_type face;

	// void assign( suit_type, face_type );
	char * suit_name( void );
	char * print_card( void );
	unsigned int face_value( void );
};

class Hand_Class {
    public:
	Card_Class ** card;
	unsigned int size;
	bool is_crib;

	Hand_Class( unsigned int, bool );
	void init( void );
	void add_card( Card_Class * );
	void print( bool );
	Card_Class * extract_card( unsigned int );
	void count_faces( unsigned int * );
	~Hand_Class( void );
};

class Deck_Class {
    private:
	Card_Class card[DECK_SIZE];
	unsigned int card_idx;

    public:
	Deck_Class( void );
	void shuffle( void );
        Card_Class * deal_one( void );
};    	