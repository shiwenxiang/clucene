using System;

using lucene.document;
using lucene.search;
namespace searchCS
{
	/// <summary>
	/// Summary description for Class1.
	/// </summary>
	class Class1
	{
		/// <summary>
		/// The main entry point for the application.
		/// </summary>
		[STAThread]
		static void Main(string[] args)
		{
			Console.Write("Enter index location: ");
			String path = Console.ReadLine();
			MIndexSearcher searcher = new MIndexSearcher(path);
			
			while (true) 
			{
				Console.WriteLine( "Enter query string: " );
				String line = Console.ReadLine();
				if ( line.Trim().Equals( "" ) )
					break;

				DateTime str = DateTime.Now;

				MQuery query = MQueryParser.Parse(line,"contents");
				Console.WriteLine( "Searching for: " + query.ToString("contents") );
				MHits hits = searcher.search(query);
				
				Console.WriteLine( "Returned " + hits.Length() + " hits." );
				for ( int i=0;i<hits.Length();i++ )
				{
					Console.WriteLine( hits.doc(i).get("path") );
				}
				Console.WriteLine("Time taken: " + ( (DateTime.Now.ToFileTime()-str.ToFileTime())/100000 ) + "ms.\n");

			}
		}
	}
}
