package dbcourse2011.importer;

import java.io.IOException;
import java.util.List;


public class ActorsImporter
{
	private ListFileParser parser;
	
	public ActorsImporter(String filename) throws IOException
	{
		parser = new ListFileParser(filename);
		parser.skipUntil("^Name\\s+Titles\\s$");
		parser.skipUntil("^-*\\s+-*\\s*$");
	}

	public ActorInfo getNext() throws IOException
	{
		List<String> lines = parser.readUntil("^\\s*$");
		return null;
	}

}
