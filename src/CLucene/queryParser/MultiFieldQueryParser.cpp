#include "CLucene/StdHeader.h"
#include "MultiFieldQueryParser.h"
#include "CLucene/analysis/AnalysisHeader.h"
#include "CLucene/search/BooleanQuery.h"
#include "CLucene/search/SearchHeader.h"
#include "QueryParser.h"

using namespace lucene::index;
using namespace lucene::util;
using namespace lucene::search;
using namespace lucene::analysis;

namespace lucene{ namespace queryParser{

    //static 
    Query& MultiFieldQueryParser::Parse(const char_t* query, const char_t** fields, const int_t fieldsLen, Analyzer& analyzer)
    {
        BooleanQuery* bQuery = new BooleanQuery();
        for (int_t i = 0; i < fieldsLen; i++)
        {
			Query& q = QueryParser::Parse(query, fields[i], analyzer);
            bQuery->add(q, true, false, false);
        }
        return *bQuery;
    }

    //static 
    Query& MultiFieldQueryParser::Parse(const char_t* query, const char_t** fields, const int_t fieldsLen, const l_byte_t* flags, Analyzer& analyzer)
    {
        BooleanQuery* bQuery = new BooleanQuery();
        for (int_t i = 0; i < fieldsLen; i++)
        {
			Query& q = QueryParser::Parse(query, fields[i], analyzer);
            l_byte_t flag = flags[i];
            switch (flag)
            {
                case REQUIRED_FIELD:
                    bQuery->add(q, true, true, false);
                    break;
                case PROHIBITED_FIELD:
                    bQuery->add(q, true, false, true);
                    break;
                default:
                    bQuery->add(q, true, false, false);
                    break;
            }
        }
        return *bQuery;
    }
}}
