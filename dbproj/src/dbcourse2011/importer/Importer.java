package dbcourse2011.importer;

import java.io.IOException;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import dbcourse2011.db.Schema;


public class Importer
{
	private Schema schema;
	
	public Importer(Schema schema)
	{
		this.schema = schema;
	}
	
	public void import_all() throws IOException
	{
		import_movies("lists/movies.short");
		//import_actors("lists/actors.short");
	}

	private static void import_movies(String filename) throws IOException
	{
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^MOVIES\\s+LIST\\s*$");
		parser.skipUntil("^=+\\s*$");

		Pattern line_pat = Pattern.compile("(.+?)\\s+(?:\\{(.*?)\\})??(?:\t+(.*))??");
		Pattern name_pat = Pattern.compile("(.+?)\\s+\\((\\d+)\\)");
		Matcher m;

		for (int i = 0; i < 10; i++) {
			String line = parser.readLine();
			if (line == null) {
				break;
			}
			if (line.trim().isEmpty()) {
				continue;
			}
			m = line_pat.matcher(line);
			if (!m.matches()) {
				continue;
			}
			String name = m.group(1);
			String episode = m.group(2);
			String s_year = m.group(3);
			boolean tvshow = false;
			int year = -1;
			
			try {
				year = Integer.parseInt(s_year);
			}
			catch (NumberFormatException ex) {
				s_year = null;
			}
			if (s_year == null) {
				m = name_pat.matcher(name);
				if (m.matches()) {
					year = Integer.parseInt(m.group(2));
				}
			}
			if (name.startsWith("\"")) {
				tvshow = true;
			}

			System.out.println(name + " | " +  year);
		}
	}
	
	private static void import_actors(String filename) throws IOException
	{
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^Name\\s+Titles\\s*$");
		parser.skipUntil("^-*\\s+-*\\s*$");
		
		while (true) {
			List<String> lines = parser.readUntil("^\\s*$");
			if (lines == null) {
				break;
			}
			String name = null;
			for (String line : lines) {
				String role;
				if (name == null) {
					String parts[] = line.split("\t+");
					name = parts[0];
					role = parts[1];
				}
				else {
					String parts[] = line.split("\t+");
					role = parts[0];
				}
			}
			
		}
	}

}
