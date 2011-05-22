typedef enum
{
	OP_LEXWORD,
	OP_DIGIT,
	OP_LOWERENG,
	OP_ASCII,
} rule_operator_t;

typedef struct _rule_term
{
	rule_operator_t			op;
	int						count;
} rule_term_t;

typedef struct 
{
	int						num_of_terms;
	rule_term_t*			terms;
	BasicHashFunctionPtr	hashfunc;
	int						num_of_words;
	const char**			lexicon;
	int						num_of_passwords; // -1 means "all"
} rule_info_t;

typedef struct
{
	rule_info_t *			rule;
	//int						state;
} rule_generator_t;


int rule_generate_all()
{

}

