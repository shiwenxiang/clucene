#include "stdafx.h"
#include "..\contributions\highlighter\QueryTermExtractor.h"
#include "..\contributions\highlighter\QueryScorer.h"
#include "..\contributions\highlighter\Highlighter.h"
#include "..\contributions\highlighter\SimpleHTMLFormatter.h"

using namespace lucene::analysis;
using namespace lucene::queryParser;
using namespace lucene::search::highlight;

namespace lucene{ namespace test{


	/**
		Tests that the query term extractor really extracts the unique terms present in the query
	*/
	void QueryTermExtractorTest() {

		_cout << _T("- QueryTermExtractorTest -") << endl;

		SimpleAnalyzer analyzer;
		QueryParser parser(_T("contents"), analyzer);
		const int NUM_TESTS = 3;
		char_t* queries[NUM_TESTS] = {
			_T("a b"),	
			_T("\"a b\""),	 	
			_T("\"a b\" a a \"b b\" b")    
		};	
		int_t num_extracted_terms[NUM_TESTS] = {2, 2, 2};
		char_t* extracted_terms[NUM_TESTS][2] = {
			{_T("a"), _T("b")},
			{_T("a"), _T("b")},
			{_T("a"), _T("b")}
		};

		for (int j = 0; j < NUM_TESTS; j++) {		
			bool ok = true;

			// Parse query and extract terms
			std::auto_ptr<Query> query(&parser.Parse(queries[j]));
			WeightedTermList* term_list = QueryTermExtractor::getTerms(query.get());

			// Is number of extracted terms OK?
			_cout << endl << _T("Query ") << j << _T(": ") << queries[j] << _T(" ");
			_cout << _T(" ---- ") << term_list->size() << _T(" terms extracted: ");
			if (num_extracted_terms[j] != term_list->size()) {
				_cout << _T("ERROR: Expected number = ") << num_extracted_terms[j] << _T(" ");
				ok = false;
			}

			// Did we also extract the right terms?
			char_t** pTerms = extracted_terms[j];
			for (WeightedTermList::const_iterator cit = term_list->begin(); cit != term_list->end(); cit++, pTerms++) {
                _cout << (cit->first)->getTerm() << _T(" ");
				if (pTerms - extracted_terms[j] < num_extracted_terms[j] && stringCompare((cit->first)->getTerm(), *pTerms)!=0) {
					_cout << _T("(expected: ") << *pTerms << _T(" ) ");
					ok = false;
				}
			}
			
			_cout << (ok ? _T("OK") : _T("Failed")) << endl;

			// Free term list
			delete term_list;

		}
	}

	/**
		Tests the high level highlighting functions.
	 */

	void GlobalHighlightTest(){

		_cout << _T(" - global test -") << endl;

		SimpleAnalyzer analyzer;
		QueryParser parser(_T("contents"), analyzer);
		const int NUM_TESTS = 2;
		const int FRAGMENT_SIZE = 6;
		const char_t * SEPARATOR = _T("...");

		char_t* query_text = _T("a b");
		char_t * test_texts[NUM_TESTS] = {
			_T("a b c ") _T("a a a ") _T("b b b ") _T("a a b ") _T("c d e ") _T("a c e "),
			_T("c d e ") _T("a a a ") _T("b b b ") _T("a c e ") _T("c d e ") _T("b c e ")
		};

		char_t * expected_highlights[NUM_TESTS] = {
			_T("<b>a</b> <b>b</b> c ") 
				_T("...") 
				_T("<b>a</b> <b>a</b> <b>b</b> "), 
				
				_T("<b>a</b> <b>a</b> <b>a</b> ") 
				_T("<b>b</b> <b>b</b> <b>b</b> ") 
		};

		char_t * best_fragments[NUM_TESTS] = {
			_T("<b>a</b> <b>b</b> c "), 
			_T("<b>a</b> <b>a</b> <b>a</b> ") 
		};

		std::auto_ptr<Query> query(&parser.Parse(query_text));

		for (int j = 0; j < NUM_TESTS; j++) {		
			bool ok = true;
			

			Highlighter highlighter(new SimpleHTMLFormatter(_T("<b>"), _T("</b>")), new QueryScorer(query.get()), new SimpleFragmenter(FRAGMENT_SIZE));

			{
				StringReader r(test_texts[j]);
				TokenStream& ts = analyzer.tokenStream(_T("dummy"), &r);

				// Compare two best fragments
				char_t * highlighted_text = highlighter.getBestFragments(&ts, test_texts[j], 2, _T("..."));

				_cout << highlighted_text << endl;

				if (stringCompare(highlighted_text, expected_highlights[j]) != 0)
					_cout << _T("ERROR : expected ") << expected_highlights[j] << endl;
				else
					_cout << _T("OK") << endl;

				delete[] highlighted_text;

				ts.close();
				delete &ts;
			}
			{
				StringReader r(test_texts[j]);
				TokenStream& ts = analyzer.tokenStream(_T("dummy"), &r);

				// Compare best fragment
				char_t * best_fragment = highlighter.getBestFragment(&ts, test_texts[j]);

				_cout << best_fragment << endl;
	
				if (stringCompare(best_fragment, best_fragments[j]) != 0)
					_cout << _T("ERROR : expected ") << best_fragments[j] << endl;
				else
					_cout << _T("OK") << endl;

				delete[] best_fragment;

				ts.close();
				delete &ts;
			}
		}
	}

	void doSimpleHTMLFormatterTest(int num_tests, Formatter & formatter, const char_t *texts[], const float_t scores[], const char_t * results[])
	{
		for (int i = 0; i < num_tests; i++) {
			char_t * hterm = formatter.highlightTerm(texts[i], texts[i], scores[i], 0);
			_cout << _T("Default simpleHTMLFormatter: highlight(") << texts[i] << _T(", ") << scores[i] << _T(") = ") << hterm << _T(" ");
			if (stringCompare(hterm, results[i]) == 0)
				_cout << _T("OK") << std::endl;
			else {
				_cout << _T("Failed - Expected ") << results[i] << endl;
			}
			delete hterm;
		}
	}

	/**
		Tests the simple formatter.
	 */

	void SimpleHTMLFormatterTest()
	{
		_cout << _T(" - SimpleHTMLFormatter test -") << endl;

		const int NUM_TESTS = 2;
		const char_t* textTerms[NUM_TESTS] = {
			_T("a"), _T("b")
		};
		float_t scores[NUM_TESTS] = {
			0.0, 1.0
		};
		// Default (bold) formatter
		{
			const char_t* highlightedTerm[NUM_TESTS] = {
				_T("a"), _T("<B>b</B>")
			};

			SimpleHTMLFormatter formatter;
			doSimpleHTMLFormatterTest(NUM_TESTS, formatter, textTerms, scores, highlightedTerm);
		}
		// Italic formatter
		{
			const char_t* highlightedTerm[NUM_TESTS] = {
				_T("a"), _T("<I>b</I>")
			};

			SimpleHTMLFormatter formatter(_T("<I>"), _T("</I>"));
			doSimpleHTMLFormatterTest(NUM_TESTS, formatter, textTerms, scores, highlightedTerm);
		}
	}

	/**
		Tests the simple fragmenter.
	 */

	void SimpleFragmenterTest()
	{
		_cout << _T(" - SimpleHTMLFormatter test -") << endl;

		// Create fragmenter
		const int FRAGMENT_SIZE = 10;
		SimpleFragmenter fragmenter(FRAGMENT_SIZE);
		if (fragmenter.getFragmentSize() != FRAGMENT_SIZE) {
			_cout << 
				_T("SimpleFragmenter constructor failed - Fragment size = ") << 
				fragmenter.getFragmentSize() << _T(" : ")<< 
				FRAGMENT_SIZE << _T(" expected.") << endl;
			return;
		}
	
		// Test text (odd tokens start fragment)
		char_t * test = _T("aaaa bbbb cccc dddd eeee ffff gggg");

		// Init fragmenter
		fragmenter.start(test);

		// Process tokens
		SimpleAnalyzer analyzer;
		StringReader reader = StringReader(test);
		TokenStream& ts = analyzer.tokenStream(_T("dummy"), &reader );

		Token * token;
		int i = 1;
		while ((token = ts.next()) != 0) {

			bool is_new = fragmenter.isNewFragment(token);
			if (is_new && i%2==0) {
				_cout << _T("ERROR: Token ") << token->TermText() << _T(" is not the first in fragment") << endl;
			} else if (!is_new && i%2==1 && i > 1) {
				_cout << _T("ERROR: Token " )<< token->TermText() << _T(" should be the first in fragment") << endl;
			} else {
				_cout << _T("OK") << endl;
			}
			i++;
			delete token;
		}

		ts.close();
		delete &ts;
	}

	/**
		Tests the query scorer.
	 */

	void QueryScorerTest()
	{
		_cout << _T(" - QueryScorer test -") << endl;

		const int FRAGMENT_SIZE = 6;
		SimpleAnalyzer analyzer;
		SimpleFragmenter fragmenter(FRAGMENT_SIZE);
		QueryParser parser(_T("contents"), analyzer);
		char_t * query_text = _T("a b");
		char_t * test = _T("a a a ") // 1 unique term -> score : 1.0
						_T("b b b ") // 1 unique term -> score : 1.0
						_T("a a b ") // 2 unique terms -> score : 2.0
						_T("b a b ") // 2 unique terms -> score : 2.0
						_T("a b c ") // 2 unique terms -> score : 2.0
						_T("a c d ") // 1 unique term  -> score : 1.0
						_T("c d e "); // no unique terms -> score : 0.0
		float_t fragment_scores[] = {1.0, 1.0, 2.0, 2.0, 2.0, 1.0, 0.0};				    
		float_t token_scores[] = {
			1.0, 1.0, 1.0,
			1.0, 1.0, 1.0,
			1.0, 1.0, 1.0, 
			1.0, 1.0, 1.0,
			1.0, 1.0, 0.0,
			1.0, 0.0, 0.0,
			0.0, 0.0, 0.0};

		std::auto_ptr<Query> query(&parser.Parse(query_text));
		QueryScorer scorer(query.get());

		scorer.startFragment(0);

		lucene::analysis::Token * token;
		fragmenter.start(test);

		StringReader r(test);
		TokenStream& ts = analyzer.tokenStream(_T("dummy"), &r);
		int i_token = 0;
		int i_fragment= 0;
		int n_mismatches = 0;
		while ((token = ts.next()) != 0)
		{

			if (fragmenter.isNewFragment(token))
			{
				float_t fragment_score = scorer.getFragmentScore();
				if (fragment_score != fragment_scores[i_fragment]) {
					_cout <<_T("ERROR: fragment ") << i_fragment << _T(" scores ") << fragment_score << _T(" : ") << fragment_scores[i_fragment] << _T(" expected.") << endl;
					n_mismatches++;
				}
				scorer.startFragment(0);
				i_fragment++;
			}

			_cout << _T("Fragment ") << i_fragment << _T(" : Token ") << i_token << _T(" : " )<< token->TermText() << _T(" : ");

			// does query contain current token?
			float_t token_score=scorer.getTokenScore(token);			

			_cout << token_score << _T(" : ") << scorer.getFragmentScore() << endl;

			if (token_score != token_scores[i_token]) {
				_cout << endl << _T("ERROR: token ") << i_token << _T(" scores ") << token_score << _T(" : ") << token_scores[i_token] << _T(" expected.") << endl;
				n_mismatches++;
			}
			i_token++;

			delete token;
		}
		ts.close();
		delete &ts;
		float_t fragment_score = scorer.getFragmentScore();
		if (fragment_score != fragment_scores[i_fragment]) {
			_cout << _T("ERROR: fragment ") << i_fragment << _T(" scores ") << fragment_score << _T(" : ") << fragment_scores[i_fragment] << _T(" expected.") << endl;
			n_mismatches++;
		}

		if (n_mismatches == 0)
			_cout << _T("OK") << endl;
	}

	/**
		Tests the highlight module.
	 */

	void HighlightTest()
	{
		// Raw memory leak detection
		const int NUM_ITERATIONS = 1;

		for (int i = 0; i < NUM_ITERATIONS;i++) {
			QueryTermExtractorTest();
			SimpleHTMLFormatterTest();
			SimpleFragmenterTest();
			QueryScorerTest();
			GlobalHighlightTest();
		}
	}

}}
