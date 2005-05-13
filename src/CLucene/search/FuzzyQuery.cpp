#include "CLucene/StdHeader.h"
#include "FuzzyQuery.h"
#ifndef NO_FUZZY_QUERY

namespace lucene{ namespace search{

	FuzzyTermEnum::FuzzyTermEnum(IndexReader& reader, Term* term): 
        FilteredTermEnum(reader,term){
	//Func - Constructor
	//Pre  - reader contains a valid reference to an IndexReader
	//       term != NULL
	//Post - The instance has been created

      CND_PRECONDITION(term != NULL,"term is NULL");

	  searchTerm = term->pointer();
      //field      = term->Field();
      //text       = term->Text();

      distance   = 0;
      fieldMatch = false;
      endEnum    = false;
      //Initialize e to NULL
      e          = NULL;
      eWidth     = 0;
      eHeight    = 0;

      //Set the fuzzy threshold AND the scale factor
      setFuzzyThreshold(0.5);

      Term* t = new Term(searchTerm->Field(), _T(""));
	  //Get all terms
	  TermEnum* terms = &reader.getTerms(t);
      //Set the enumeration 
      setEnum( terms );
      t->finalize();
  }

  FuzzyTermEnum::~FuzzyTermEnum(){
  //Func - Destructor
  //Pre  - true
  //Post - FuzzyTermEnum has been destroyed

	  //Close the enumeration
	  close();
      //Finalize the searchTerm
      searchTerm->finalize();
	  //Destroy e
      _DELETE_ARRAY(e);
  }

  bool FuzzyTermEnum::EndEnum() {
  //Func - Returns the fact if the current term in the enumeration has reached the end
  //Pre  - true
  //Post - The boolean value of endEnum has been returned

      return endEnum;
  }

  void FuzzyTermEnum::close(){
  //Func - Close the enumeration
  //Pre  - true
  //Post - The enumeration has been closed

      FilteredTermEnum::close();
  }

  void FuzzyTermEnum::setFuzzyThreshold(float_t value){
  //Func - Sets the fuzzy threshold and calculates the scale factor
  //Pre  - value contains the fuzzythreshold
  //Post - FUZZY_THRESHOLD contains value and
  //       SCALE_FACTOR = 1.0f / (1.0f - FUZZY_THRESHOLD);

      FUZZY_THRESHOLD = value;
      SCALE_FACTOR = 1.0f / (1.0f - FUZZY_THRESHOLD);
  }

  bool FuzzyTermEnum::termCompare(Term* term) {
  //Func - Compares term with the searchTerm using the Levenshtein distance.
  //Pre  - term is NULL or term points to a Term
  //Post - if pre(term) is NULL then false is returned otherwise
  //       if the distance of the current term in the enumeration is bigger than the FUZZY_THRESHOLD
  //       then true is returned 
	  
	  if (term == NULL){
		  return false;  //Note that endEnum is not set to true!
	      }

	  //Check if the field name of searchTerm of term match
      if (stringCompare(searchTerm->Field(), term->Field())==0 ) {
		  //Calculate the length of the text value of term
          const int_t targetlen = stringLength(term->Text());         
		  //Calculate the length of the text value of searchTerm
          const int_t textlen   = stringLength(searchTerm->Text()); 
		  //Calculate the Levenshtein distance
          int_t dist = editDistance(searchTerm->Text(), term->Text(), textlen, targetlen);
          distance = 1 - ((double)dist / (double)min(textlen, targetlen));
          //return the result of the check whether distance is bigger than the FUZZY_THRESHOLD
          return (distance > FUZZY_THRESHOLD);
          }
	  else{
          endEnum = true;
          return false;
	      }
  }

  float_t FuzzyTermEnum::difference() {
  //Func - Returns the difference between the distance and the fuzzy threshold
  //       multiplied by the scale factor
  //Pre  - true
  //Post - The difference is returned

     return (float_t)((distance - FUZZY_THRESHOLD) * SCALE_FACTOR);
  }
  
  int_t FuzzyTermEnum::Min(const int_t a, const int_t b, const int_t c){
  //Func - Static Method
  //       Finds and returns the smallest of three integers
  //Pre  - a,b,c contain values
  //Post - The smallest value has been returned

      int_t t = (a < b) ? a : b;
      return (t < c) ? t : c;
  }

  int_t FuzzyTermEnum::editDistance(const char_t* s, const char_t* t, const int_t n, const int_t m) {
  //Func - Calculates the Levenshtein distance also known as edit distance is a measure of similiarity
  //       between two strings where the distance is measured as the number of character
  //       deletions, insertions or substitutions required to transform one string to
  //       the other string.
  //Pre  - s != NULL and contains the source string
  //       t != NULL and contains the target string
  //       n >= 0 and contains the length of the source string
  //       m >= 0 and containts the length of th target string
  //Post - The distance has been returned

      CND_PRECONDITION(s != NULL, "s is NULL");
      CND_PRECONDITION(t != NULL, "t is NULL");
	  CND_PRECONDITION(n >= 0," n is a negative number");
	  CND_PRECONDITION(n >= 0," n is a negative number");

      int_t i;     // iterates through s
      int_t j;     // iterates through t
      char_t s_i; // ith character of s

      if (n == 0) 
          return m;
      if (m == 0) 
          return n;

	//Check if the array must be reallocated because it is too small or does not exist
    if (e == NULL || eWidth <= n || eHeight <= m) {
        //Delete e if possible
        _DELETE_ARRAY(e);
        //resize e
        eWidth  = max(eWidth, n+1);
        eHeight = max(eHeight, m+1);
        e = new int_t[eWidth*eHeight];
        }
    
    CND_CONDITION(e != NULL,"e is NULL")

    // init matrix e
	for (i = 0; i <= n; i++){
        e[i + (0*eWidth)] = i;
        }
	for (j = 0; j <= m; j++){
        e[0 + (j*eWidth)] = j;
        }

    // start computing edit distance
    for (i = 1; i <= n; i++) {
        s_i = s[i - 1];
        for (j = 1; j <= m; j++) {
			if (s_i != t[j-1]){
                e[i + (j*eWidth)] = Min(e[i-1 + (j*eWidth)], e[i + ((j-1)*eWidth)], e[i-1 + ((j-1)*eWidth)])+1;
			    }
			else{
                e[i + ((j)*eWidth)] = Min(e[i-1 + ((j)*eWidth)]+1, e[i + ((j-1)*eWidth)]+1, e[i-1 + ((j-1)*eWidth)]);
			    }
            }
        }

    // we got the result!
    return e[n + ((m)*eWidth)];
  }


  FuzzyQuery::FuzzyQuery(Term* term):MultiTermQuery(term){
  //Func - Constructor
  //Pre  - term != NULL
  //Post - The instance has been created

	  CND_PRECONDITION(term != NULL,"term is NULL")

      MultiTermQuery::LUCENE_STYLE_TOSTRING = true;

	  //Get a reference pointer to term
	  fuzzyTerm = term->pointer();
    }

    FuzzyQuery::~FuzzyQuery(){
    //Func - Destructor
	//Pre  - true
	//Post - Instance has been destroyed

        fuzzyTerm->finalize();
    }

    void FuzzyQuery::prepare(IndexReader& reader) {
	//Func - Prepares a fuzzy query
	//Pre  - reader contains a valid reference to an IndexReader
	//Post - The fuzzy query has been prepared

		FuzzyTermEnum *FzEnum = new FuzzyTermEnum(reader, fuzzyTerm);
       
		CND_CONDITION(FzEnum != NULL,"Could not allocated FuzzyTermEnum FzEnum");

        setEnum(FzEnum);
    }

    const char_t* FuzzyQuery::toString(const char_t* field){
	//Func - Returns the query string
	//Pre  - field != NULL
	//Post - The query string has been returned

        CND_PRECONDITION(field != NULL,"field is NULL")

        StringBuffer buffer;
        const char_t* b = MultiTermQuery::toString(field);
    
        buffer.append ( b );
       _DELETE_ARRAY(b);
        buffer.append( _T("~") );

        return buffer.ToString();
    }

  const char_t* FuzzyQuery::getQueryName() const{
  //Func - Returns the name of the query
  //Pre  - true
  //post - The string FuzzyQuery has been returned

     return _T("FuzzyQuery");
  }
}}
#endif
