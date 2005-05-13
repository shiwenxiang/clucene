#include "CLucene/StdHeader.h"
#include "BooleanScorer.h"

#include "Scorer.h"
#include "Similarity.h"

using namespace lucene::util;
namespace lucene{ namespace search{

  BooleanScorer::BooleanScorer(){
  //Func - Constructor
  //Pre  - true
  //Post - The instance has been created

      coordFactors   = NULL;
      maxCoord       = 1;
      currentDoc     = 0;
      requiredMask   = 0;
      prohibitedMask = 0;
      nextMask       = 1;
      scorers        = NULL;

      bucketTable = new BucketTable(*this);
	  
	  CND_CONDITION(bucketTable != NULL,"Could not allocate memory for bucketTable")
  }

  BooleanScorer::~BooleanScorer(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

      _DELETE(bucketTable);
	  _DELETE_ARRAY(coordFactors);
      _DELETE(scorers);
  }


  void BooleanScorer::add(Scorer& scorer, const bool required, const bool prohibited) {
    int_t mask = 0;
    if (required || prohibited) {
      if (nextMask == 0)
        _THROWC( "More than 32 required/prohibited clauses in query.");
      mask = nextMask;
      nextMask = ( nextMask << 1 );
    } else
      mask = 0;

    if (!prohibited)
      maxCoord++;

    if (prohibited)
      prohibitedMask |= mask;			  // update prohibited mask
    else if (required)
      requiredMask |= mask;			  // update required mask

        //scorer, HitCollector, and scorers is delete in the SubScorer
    scorers = new SubScorer(&scorer, required, prohibited,
          bucketTable->newCollector(mask), scorers);
  }

  void BooleanScorer::computeCoordFactors(){
    coordFactors = new float_t[maxCoord];
    for (int_t i = 0; i < maxCoord; i++)
      coordFactors[i] = Similarity::coord(i, maxCoord);
  }

  void BooleanScorer::score(HitCollector& results, const int_t maxDoc) {
    if (coordFactors == NULL)
    computeCoordFactors();

    while (currentDoc < maxDoc) {
      currentDoc = (currentDoc+BucketTable::SIZE<maxDoc?currentDoc+BucketTable::SIZE:maxDoc);
      for (SubScorer* t = scorers; t != NULL; t = t->next)
        t->scorer->score(*(t->collector), currentDoc);
      bucketTable->collectHits(results);
    }
  }


  SubScorer::SubScorer(Scorer* scr, const bool r, const bool p, HitCollector* c, SubScorer* nxt){
  //Func - Constructor
  //Pre  - scr != NULL, 
  //       c   != NULL
  //       nxt may or may not be NULL
  //Post - The instance has been created

      CND_PRECONDITION(scr != NULL,"scr is NULL")
      CND_PRECONDITION(c != NULL,"c is NULL")

      scorer      = scr;
      collector   = c;
      required    = r;
      prohibited  = p;
      next        = nxt;
  }

  SubScorer::~SubScorer(){
  //Func - Destructor
  //Pre  - true
  //Post - The instance has been destroyed

    if (next) {
        std::vector<SubScorer *> subscorers;
		for (SubScorer * ptr = next; ptr; ptr = ptr->next){
            subscorers.push_back(ptr);
		    }
        for (std::vector<SubScorer *>::reverse_iterator rit = subscorers.rbegin(); 
			 rit != subscorers.rend(); 
			 rit++){
             (*rit)->next = 0;
             delete (*rit);
			 }
		} 
	_DELETE(scorer);
	_DELETE(collector);
  }

  Bucket::Bucket():
      doc(-1)
  {
  }
  Bucket::~Bucket(){
    //if ( next != NULL )
    //	delete next;
  }




  BucketTable::BucketTable(BooleanScorer& scr):
    scorer(scr),
    first(NULL)
  {
  }

  void BucketTable::clear(){
    //delete first;
    first = NULL;
  }
  void BucketTable::collectHits(HitCollector& results) {
    const int_t required = scorer.requiredMask;
    const int_t prohibited = scorer.prohibitedMask;
    const float_t* coord = scorer.coordFactors;

    for (Bucket* bucket = first; bucket!=NULL; bucket = bucket->next) {
      if ((bucket->bits & prohibited) == 0 &&	  // check prohibited
          (bucket->bits & required) == required){// check required

        results.collect(bucket->doc,		  // add to results
          bucket->score * coord[bucket->coord]);
      }
    }
    clear(); //first = NULL;				  // reset for next round
  }

  int_t BucketTable::size() const { return SIZE; }

  HitCollector* BucketTable::newCollector(const int_t mask) {
    return new Collector(mask, *this);
  }









  Collector::Collector(const int_t msk, BucketTable& bucketTbl):
    mask(msk),
    bucketTable(bucketTbl)
  {
  }

  void Collector::collect(const int_t doc, const float_t score) {
    BucketTable& table = bucketTable;
    const int_t i = doc & BucketTable::MASK;
    Bucket& bucket = table.buckets[i];
    //if (bucket == NULL)
    //	bucket = new Bucket();
    //	table.buckets[i] = bucket;

    if (bucket.doc != doc) {			  // invalid bucket
      bucket.doc = doc;			  // set doc
      bucket.score = score;			  // initialize score
      bucket.bits = mask;			  // initialize mask
      bucket.coord = 1;			  // initialize coord

      bucket.next = table.first;		  // push onto valid list
      table.first = &bucket;
    } else {					  // valid bucket
      bucket.score += score;			  // increment score
      bucket.bits |= mask;			  // add bits in mask
      bucket.coord++;				  // increment coord
    }
  }




}}
