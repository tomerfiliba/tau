package dbcourse2011.importer;

import java.io.IOException;
import java.util.List;


public class ActorsImporter implements Iterable<ActorInfo>
{
	public static class ActorInfo
	{

	}
	
	public ActorsImporter(String filename) throws IOException
	{
		ListFileParser fr = new ListFileParser(filename);
		fr.skipUntil("^Name\\s+Titles\\s$");
		fr.skipUntil("^-*\\s+-*\\s*$");
		
		for (int i = 0; i < 4; i++) {
			List<String> lines = fr.readUntil("^\\s*$");
			for (String l : lines) {
				System.out.println(l);
			}
		}
	}

}
