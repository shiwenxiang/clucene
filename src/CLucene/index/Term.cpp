#include "CLucene/StdHeader.h"
#include "Term.h"

namespace lucene{ namespace index{
	
	Term::Term(const char_t* fld, const char_t* txt ){
    //Func - Constructor.
    //       Constructs a Term with the given field and text. Field and text are copied.
    //Pre  - fld != NULL and contains the name of the field
    //       txt != NULL and contains the value of the field
    //Post - An instance of Term has been created.Field and text have been copied and stored.  

        CND_PRECONDITION(fld != NULL, "fld contains NULL")
        CND_PRECONDITION(txt != NULL, "txt contains NULL")

		//Term is not interned
		intrn = false;
		//Duplicated fld and assign it to field
	    field = stringDuplicate(fld);
		//Duplicate txt and assign it to text
		text  = stringDuplicate(txt);
	}

	Term::Term(char_t* fld, char_t* txt,const bool intern,const bool canDelete){
    //Func - Constructor.
    //       Constructs a Term with the given field and text. Field and text are not copied
	//       Field and text are deleted in destructor only if intern is false. 
    //Pre  - fld != NULL and contains the name of the field
    //       txt != NULL and contains the value of the field
	//       intrn is true or false and indicates if term is interned or not
    //       canDelete defaults to true but can be false and indicates if Term can be deleted when finalized
    //Post - An instance of Term has been created.Field and txt have not been copied but assigned

        CND_PRECONDITION(fld != NULL, "fld contains NULL")
        CND_PRECONDITION(txt != NULL, "txt contains NULL")

		intrn  = intern;
		field  = fld;
		text   = txt;

		lucene::util::IGCollectable<Term>::_canDelete = canDelete;
	}
	
	Term::~Term(){
	//Func - Destructor.
	//Pre  - true
	//Post - The instance has been destroyed. field and text have been deleted if pre(intrn) is false

		//Check if the term is interned
		if ( !intrn ){
			if(field){
			    delete[] field;
			    }
			if(text){
			    delete[] text;
			    }
            }
	}

    const char_t* Term::Field() const {
    //Func - Returns the field of this term, an interned string. The field indicates
    //       the part of a document which this term came from. 
    //Pre  - true
    //Post - field has been returned

        return field;
    }

    const char_t* Term::Text() const {
    //Func - Returns the text of this term.  In the case of words, this is simply the
    //       text of the word.  In the case of dates and other types, this is an
    //       encoding of the object as a string.
	//Pre  - true
	//Post - text has been returned

        return text;
    }

    bool Term::equals(const Term* o) const {
    //Func - Compares two terms, returning true if they have the same
    //       field and text. 
    //Pre  - o is NULL or o points to a Term
    //Post - if pre(o) is NULL then false is returned, otherwise the result
	//       of the comparison has been returned which can be true or false
	        
        //Check if o points to a term
		if (o == NULL){
            return false;
            }
		//compare field with field of o and text with text of o and return the result of the comparisons
		return stringCompare(field,o->field) == stringCompare(text,o->text) == 0;
	}

	void Term::set(char_t* fld, char_t* txt,const bool intern){
    //Func - Resets the field and text of a Term.
    //Pre  - fld != NULL and contains the name of the field
    //       txt != NULL and contains the value of the field
	//       intern is true or false
    //Post - field and text of Term have been reset

        CND_PRECONDITION(fld != NULL, "fld contains NULL")
        CND_PRECONDITION(txt != NULL, "txt contains NULL")

        //Check if the term is interned
		if ( !intrn ){
			if (field){
			    delete[] field;
			    }
			if(text){
			    delete[] text;
			    }
		    }
		

	    //Reset Term
		field = fld;
		text  = txt;
		intrn = intern;
	}

	int_t Term::compareTo(const Term& other) const {
    //Func - Compares two terms, to see if this term belongs before,is equal to or after
	//       after the argument term.
	//Pre  - other is a reference to another term
	//Post - A negative integer is returned if this term belongs before the argument, 
    //       zero is returned if this term is equal to the argument, and a positive integer 
	//       if this term belongs after the argument.

		//Compare field with field of other
		int_t ret = stringCompare(field,other.field);
		
		//Check ret to see if text needs to be compared
		if ( ret == 0){
			//Compare text with text of other and return the result
			return stringCompare(text,other.text);
		    }
		else
			return ret;
	}

	char_t* Term::toString() const{
	//Func - Forms the contents of Field and term in some kind of tuple notation
	//       <field:text>
	//Pre  - true
	//Post - a string formatted as <field:text> is returned if pre(field) is NULL and
	//       text is NULL the returned string will be formatted as <:>

		return lucene::util::Misc::join( _T("Term<"), field, _T(":"), text, _T(">"));
	}
		
}}
