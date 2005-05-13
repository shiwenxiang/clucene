#include "CLucene/StdHeader.h"
#ifndef CLUCENE_LITE

#include "DocumentWriter.h"

#include "FieldInfos.h"
#include "IndexWriter.h"
#include "FieldsWriter.h"
#include "Term.h"
#include "TermInfo.h"
#include "TermInfosWriter.h"

#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/document/Document.h"
#include "CLucene/document/Field.h"
#include "CLucene/search/Similarity.h"
#include "CLucene/store/Directory.h"
#include "CLucene/util/VoidMap.h"


//#include <iostream> //for DEBUG

using namespace std;
using namespace lucene::util;
namespace lucene{ namespace index {


  /*Posting*/

  Posting::Posting(Term& t, const int_t position):
  term (*t.pointer()){
  //Func - Constructor
  //Pre  - t contains a valid reference to a Term
  //Post - Instance has been created

  freq = 1;

  positions = new int_t[1];
  positionsLength = 1;
  positions[0] = position;
  }

  Posting::~Posting(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

  _DELETE_ARRAY(positions);
  term.finalize();
  }

  int_t Posting::getPositionsLength(){
  return positionsLength;
  }


  /*DocumentWriter =======================*/

  //static
  const char_t* DocumentWriter::segmentname(const char_t* segment, const char_t* ext ){
  char_t* buf = new char_t[MAX_PATH];
  stringPrintF(buf,_T("%s%s"), segment,ext );
  return buf;
  }

  DocumentWriter::DocumentWriter(lucene::store::Directory& d, lucene::analysis::Analyzer& a, const int_t mfl):
    analyzer(a),
    directory(d),
    maxFieldLength(mfl),
    termBuffer(*new Term( _T(""), _T("") ,true)){
  //Func - Constructor
  //Pre  - d contains a valid reference to a Directory
  //       d contains a valid reference to a Analyzer
  //       mfl > 0 and contains the maximum field length or
  //       mfl == IndexWriter::FIELD_TRUNC_POLICY__WARN
  //Post - Instance has been created

  CND_PRECONDITION(((mfl > 0) || (mfl == IndexWriter::FIELD_TRUNC_POLICY__WARN)),
	  "mfl is 0 or smaller than IndexWriter::FIELD_TRUNC_POLICY__WARN")

  fieldInfos     = NULL;
  fieldLengths   = NULL;
  }

  DocumentWriter::~DocumentWriter(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

  clearPostingTable();
  _DELETE(fieldInfos);
  _DELETE_ARRAY(fieldLengths);

  termBuffer.finalize();
  }

  void DocumentWriter::clearPostingTable(){
  map<Term*,Posting*,TermCompare>::iterator itr = postingTable.begin();
  while ( itr != postingTable.end() ){
    _DELETE(itr->second);
    itr->first->finalize();

    itr++;
  }
  postingTable.clear();
  }

  void DocumentWriter::addDocument(const char_t* segment, lucene::document::Document& doc) {
  // write field names
  fieldInfos = new FieldInfos();
  fieldInfos->add(doc);

  const char_t* buf = segmentname(segment, _T(".fnm"));
  fieldInfos->write(directory, buf);
  _DELETE_ARRAY(buf);

  // write field values
  FieldsWriter fieldsWriter(directory, segment, *fieldInfos);
  _TRY {
    fieldsWriter.addDocument(doc);
  } _FINALLY( fieldsWriter.close() );

  // invert doc into postingTable
  clearPostingTable();			  // clear postingTable
  fieldLengths = new int_t[fieldInfos->size()];	  // init fieldLengths
  for ( int_t i=0;i<fieldInfos->size();i++ )
    fieldLengths[i] = 0;
  invertDocument(doc);

  // sort postingTable into an array
  Posting** postings = NULL;
  int_t postingsLength = 0;
  sortPostingTable(postings,postingsLength);

  //DEBUG:
  /*for (int_t i = 0; i < postingsLength; i++) {
    Posting* posting = postings[i];

    char_t* b = posting->term.toString();
    _cout << b << " freq=" << posting->freq;
    delete b;

    _cout << " pos=" << posting->positions[0];
    for (int_t j = 1; j < posting->freq; j++)
    _cout <<"," << posting->positions[j];

    _cout << endl;
  }*/


  // write postings
  writePostings(postings,postingsLength, segment);

  // write norms of indexed fields
  writeNorms(doc, segment);

  _DELETE_ARRAY(postings);
  }

  void DocumentWriter::sortPostingTable(Posting**& Array, int_t& arraySize) {
  // copy postingTable into an array
  arraySize = postingTable.size();
  Array = new Posting*[arraySize];
  map<Term*,Posting*,TermCompare>::iterator postings = postingTable.begin();
  int_t i=0;
  while ( postings != postingTable.end() ){
    Array[i] = (Posting*)postings->second;
    postings++;
    i++;
  }
  // sort the array
  quickSort(Array, 0, i - 1);
  }


  
  void DocumentWriter::invertDocument(lucene::document::Document& doc) {
  //Func - Tokenizes the fields of a document into Postings.
  //Pre  - doc contains a valid reference to a document
  //Post -

	  //Get an enumeration of fields in the document
      lucene::document::DocumentFieldEnumeration* fields = doc.fields();

	  //Initialize iterator to NULL
      lucene::document::Field* field = NULL;

      _TRY {
          while (fields->hasMoreElements()){
              field = (lucene::document::Field*)fields->nextElement();
 
			  CND_CONDITION(field != NULL,"field is NULL")

			  //Get the name of the field
              const char_t* fieldName = field->Name();
              const int_t fieldNumber = fieldInfos->fieldNumber(fieldName);
              int_t position = fieldLengths[fieldNumber]; // position in field

			  //Check if the field must be indexed
              if (field->IsIndexed()) {
				  //Check if the field is not tokenized
                  if (!field->IsTokenized()) { 
					  // un-tokenized field
                      //FEATURE: this is bug in java too: if using a Reader this fails

                      //Check if the value of field is NULL
					  if (field->StringValue() == NULL) {
                          lucene::util::Reader* r = field->ReaderValue();
              
						  CND_CONDITION(r != NULL, "r is NULL")
						  
						  const long_t origReaderPos = r->position();
                          _TRY {
                              r->seek(0);
                              if (r->available() > INT_T_MAX) {
                                  _THROWC("CLucene is not equipped to handle field values"
                                          " longer than INT_T_MAX characters."
                                      );
                                  }
                
                              const int_t charsAvailable = static_cast<int_t>(r->available());
                
                              char_t* charBuf = new char_t[charsAvailable+1];
                             
							  CND_CONDITION(charBuf != NULL,"Could not allocate memory for charBuf")
                
                              _TRY {
                                  r->read(charBuf, 0, charsAvailable);
                                  charBuf[charsAvailable] = 0;
                                  addPosition(fieldName, charBuf, position++);
                                  } 
                              _FINALLY (
                                  _DELETE_ARRAY(charBuf);
                                  );
                              } 
                          _FINALLY(
                              r->seek(origReaderPos); // Reset to its original position.
                              );
                          } 
                      else{
                          addPosition(fieldName, field->StringValue(), position++);
                          }
                      } 
				  else{ 
					  // field must be tokenized
                      lucene::util::Reader* reader; // find or make Reader
                      bool delReader = false;
                      if (field->ReaderValue() != NULL){
                          reader = field->ReaderValue();
                          } 
					  else{ 
						  if (field->StringValue() != NULL) {
                              reader = new lucene::util::StringReader(field->StringValue());
                              delReader = true;
                              } 
                          else{
                              _THROWC("field must have either String or Reader value");
                              }
                          }

                      _TRY {
                          // Tokenize field and add to postingTable.
                          lucene::analysis::TokenStream& stream = analyzer.tokenStream(fieldName, reader);

                          _TRY {
                              // We use the ugly while(true) arrangement because the old
                              // logic sometimes failed to delete t.
                              lucene::analysis::Token* t = NULL;
                              while (true) {
                                  _TRY {
                                      if ((t = stream.next()) == NULL) {
                                          break;
                                          }
                                      position += (t->getPositionIncrement() - 1);
                                      addPosition(fieldName, t->TermText(), position++);

                                      // Apply field truncation policy.
                                      if (maxFieldLength != IndexWriter::FIELD_TRUNC_POLICY__WARN) {
                                          // The client programmer has explicitly authorized us to
                                          // truncate the token stream after maxFieldLength tokens.
                                          if (position > maxFieldLength) {
                                              break;
                                              }
                                          }
									  else{ 
                                          if (position > IndexWriter::DEFAULT_MAX_FIELD_LENGTH) {
                                              const char_t* errMsgBase =
                                                  _T("Indexing a huge number of tokens from a single")
                                                  _T(" field (\"%s\", in this case) can cause CLucene")
                                                  _T(" to use memory excessively.")
                                                  _T("  By default, CLucene will accept only %s tokens")
                                                  _T(" tokens from a single field before forcing the")
                                                  _T(" client programmer to specify a threshold at")
                                                  _T(" which to truncate the token stream.")
                                                  _T("  You should set this threshold via")
                                                  _T(" IndexWriter::maxFieldLength (set to INT_T_MAX")
                                                  _T(" to disable truncation).");
                                              
											  char_t defaultMaxAsChar[34];
                                              integerToString(IndexWriter::DEFAULT_MAX_FIELD_LENGTH,defaultMaxAsChar, 10);
                                              
											  char_t* errMsg = new char_t[
												  stringLength(errMsgBase) + stringLength(fieldName) + stringLength(defaultMaxAsChar)
                                                  ];

											  CND_CONDITION(errMsg != NULL,"Could not allocate memory for errMsg")

                                              stringPrintF(errMsg, errMsgBase, fieldName, defaultMaxAsChar);

                                              THROW_TYPE exc = _CREATE_THROWTYPE(errMsg);
                                              _DELETE_ARRAY(errMsg);
                                              throw exc;
                                              }
                                          }
                                      } 
                                  _FINALLY (
                                      _DELETE(t);
                                      );
                                  } // while true

                              //Because of the fact that the _FINALLY macro doesn't work in
                              //all cases (such as when the 'while(true)' loop above is
                              //exited via 'break'), it's necessary to explicitly release t
                              // after the loop.  This is *not* a "double free".
                              _DELETE(t);
                              } 
                          _FINALLY (
                              stream.close();
                              delete &stream;
                              );
                          } 
                      _FINALLY (
                          if (delReader) {
                              reader->close();
                              _DELETE(reader);
                              }
                          );
                      } // if/else field is to be tokenized

				  // save field length
                  fieldLengths[fieldNumber] = position; 
                  } // if field is to beindexed
              } // while more fields available
          } 
      _FINALLY (
          _DELETE(fields);
          );
  } // Document::invertDocument


  void DocumentWriter::addPosition(const char_t* field,
           const char_t* text,
           const int_t position) {
  termBuffer.set(stringDuplicate(field),stringDuplicate(text), false );

  Posting* ti = postingTable.get(&termBuffer);
  if (ti != NULL) {				  // word seen before
    int_t freq = ti->freq;
    if (ti->getPositionsLength() == freq) {	  // positions array is full
    int_t *newPositions = new int_t[freq * 2];	  // double size
    int_t *positions = ti->positions;
    for (int_t i = 0; i < freq; i++)		  // copy old positions to new
      newPositions[i] = positions[i];

    _DELETE_ARRAY(ti->positions);
    ti->positions = newPositions;
    ti->positionsLength = freq*2;
    }
    ti->positions[freq] = position;		  // add new position
    ti->freq = freq + 1;			  // update frequency
  }
  else {					  // word not seen before
    Term* term = new Term( field, text);
    postingTable.put(term->pointer(), new Posting(*term, position));
    term->finalize();
  }
  }

  //static
  void DocumentWriter::quickSort(Posting**& postings, const int_t lo, const int_t hi) {
  if(lo >= hi)
    return;

  int_t mid = (lo + hi) / 2;

  if(postings[lo]->term.compareTo(postings[mid]->term) > 0) {
     Posting* tmp = postings[lo];
    postings[lo] = postings[mid];
    postings[mid] = tmp;
  }

  if(postings[mid]->term.compareTo(postings[hi]->term) > 0) {
    Posting* tmp = postings[mid];
    postings[mid] = postings[hi];
    postings[hi] = tmp;

    if(postings[lo]->term.compareTo(postings[mid]->term) > 0) {
    Posting* tmp2 = postings[lo];
    postings[lo] = postings[mid];
    postings[mid] = tmp2;
    }
  }

  int_t left = lo + 1;
  int_t right = hi - 1;

  if (left >= right)
    return;

  const Term& partition = postings[mid]->term; //not kept, so no need to finalize

  for( ;; ) {
    while(postings[right]->term.compareTo(partition) > 0)
    --right;

    while(left < right && postings[left]->term.compareTo(partition) <= 0)
    ++left;

    if(left < right) {
    Posting* tmp = postings[left];
    postings[left] = postings[right];
    postings[right] = tmp;
    --right;
    } else {
    break;
    }
  }

  quickSort(postings, lo, left);
  quickSort(postings, left + 1, hi);
  }


  void DocumentWriter::writePostings(Posting** postings, const int_t postingsLength, const char_t* segment){
  OutputStream* freq = NULL;
  OutputStream* prox = NULL;
  TermInfosWriter* tis = NULL;

  try {
    const char_t* buf = segmentname( segment, _T(".frq"));
    freq = &directory.createFile( buf );
    _DELETE_ARRAY(buf);

    buf = segmentname( segment, _T(".prx"));
    prox = &directory.createFile( buf );
    _DELETE_ARRAY(buf);

    tis = new TermInfosWriter(directory, segment, *fieldInfos);
    TermInfo* ti = new TermInfo();

    for (int_t i = 0; i < postingsLength; i++) {
    const Posting* posting = postings[i];

    // add an entry to the dictionary with pointers to prox and freq files
    ti->set(1, freq->getFilePointer(), prox->getFilePointer());
    tis->add(posting->term, *ti);

    // add an entry to the freq file
    int_t f = posting->freq;
    if (f == 1)				  // optimize freq=1
      freq->writeVInt(1);			  // set low bit of doc num.
    else {
      freq->writeVInt(0);			  // the document number
      freq->writeVInt(f);			  // frequency in doc
    }

    int_t lastPosition = 0;			  // write positions
    int_t* positions = posting->positions;
    for (int_t j = 0; j < f; j++) {		  // use delta-encoding
      int_t position = positions[j];
      prox->writeVInt(position - lastPosition);
      lastPosition = position;
    }
    }
    _DELETE(ti);
  }_FINALLY (
    if (freq != NULL) { freq->close(); delete freq; }
    if (prox != NULL) { prox->close(); delete prox; }
    if (tis  != NULL)  { tis->close(); delete tis; }
  );
  }

  void DocumentWriter::writeNorms(lucene::document::Document& doc, const char_t* segment) {
  lucene::document::DocumentFieldEnumeration* fields  = doc.fields();
  while (fields->hasMoreElements()) {
    lucene::document::Field* field
        = (lucene::document::Field*)fields->nextElement();
    if (field->IsIndexed()) {
    int_t fieldNumber = fieldInfos->fieldNumber(field->Name());

    char_t* fb = new char_t[MAX_PATH];
    stringPrintF(fb,_T("%s.f%d"),segment,fieldNumber);
    OutputStream& norm = directory.createFile(fb);
    _DELETE_ARRAY(fb);

    _TRY {
      norm.writeByte(lucene::search::Similarity::normb(fieldLengths[fieldNumber]));
    }_FINALLY (
      norm.close();
      delete &norm;
    );
    }
  }
  _DELETE(fields);
  }

}}
#endif
