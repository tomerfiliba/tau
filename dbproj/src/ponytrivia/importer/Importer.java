package ponytrivia.importer;

import java.io.File;
import java.io.IOException;
import java.sql.Date;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import ponytrivia.db.Batch;
import ponytrivia.db.Schema;
import ponytrivia.db.SimpleInsert;
import ponytrivia.db.SimpleQuery;
import ponytrivia.db.SimpleUpdate;

public class Importer {
	protected Schema schema;
	protected ListFileReader reader;

	public Importer(Schema schema) {
		this.schema = schema;
		this.reader = null;
	}

	public static class ImportStatus {
		public final String filename;
		public final long size;
		public final long position;
		
		public ImportStatus(String filename, long size, long position) {
			this.filename = filename;
			this.size = size;
			this.position = position;
		}
	}

	public ImportStatus getStatus() throws IOException
	{
		if (reader == null) {
			return null;
		}
		return new ImportStatus(reader.fileName, reader.getSize(), reader.getPosition());
	}

	public void import_all(File directory) throws IOException, SQLException 
	{
		reader = null;
		//System.out.println("movies");
		//import_movies(directory);
		//System.out.println("ratings");
		//import_ratings(directory);
		//System.out.println("genres");
		//import_genres(directory);
		import_actors(directory);
		//import_directors(directory);
		//import_biographies(directory);
		reader = null;
	}

	private void import_movies(File directory) throws IOException, SQLException 
	{
        reader = new ListFileReader(new File(directory, "movies.list"));
        reader.skipUntil("^MOVIES\\s+LIST\\s*$");
        reader.skipUntil("^=+\\s*$");
        
        final Pattern linePattern = Pattern.compile("^(.+?)\t\\s*(.+?)$");
        final Pattern tvshowPattern = Pattern.compile("\"(.+?)\"\\s.*?\\{(.+?)\\}");
        final Pattern filmPattern = Pattern.compile("(.+?)\\s\\(\\d+\\)");
        
        Batch batch = schema.createBatch(
        	"INSERT IGNORE INTO movies (imdb_name, type, name, episode, year) " +
        	"VALUES (?, ?, ?, ?, ?)");
        
        while (true) {
        	String line = reader.readLine();
            if (line == null) {
                break;
            }
            line = line.trim();
            if (line.isEmpty()) {
                continue;
            }
            Matcher m = linePattern.matcher(line);
            if (!m.matches()) {
            	continue;
            }
            String imdb_name = m.group(1);
            String name = null;
            String episode = null;
            boolean tvshow = false;
            int year;
            try {
            	year = Integer.parseInt(m.group(2));
            } catch(NumberFormatException ex) {
            	year = -1;
            }
            if (imdb_name.charAt(0) == '"') {
            	tvshow = true;
            	m = tvshowPattern.matcher(imdb_name);
            	if (!m.matches()) {
            		continue;
            	}
            	name = m.group(1);
            	episode = m.group(2);
            }
            else {
            	m = filmPattern.matcher(imdb_name);
            	if (!m.matches()) {
            		continue;
            	}
            	name = m.group(1);
            }
            batch.add(imdb_name, tvshow ? "tv" : "film", name, episode, (year > 1900) ? year : null);
        }
        batch.close();
	}

	private void import_ratings(File directory) throws IOException, SQLException 
	{
        reader = new ListFileReader(new File(directory, "ratings.list"));
        reader.skipUntil("New\\s+Distribution\\s+Votes\\s+Rank\\s+Title");
        
        final Pattern linePattern = Pattern.compile("^.+?\\s+(\\d+)\\s+(\\d\\.\\d)\\s+(.+)$");
        
        Batch batch = schema.createBatch("UPDATE movies SET rating = ?, votes = ? WHERE movie_id = ?");
        
        while (true) {
        	String line = reader.readLine();
            if (line == null) {
                break;
            }
            line = line.trim();
            if (line.isEmpty()) {
                continue;
            }
            Matcher m = linePattern.matcher(line);
            if (!m.matches()) {
            	continue;
            }
            int votes = -1;
            double rank = -1;
            String imdb_name = m.group(3);
            try {
            	votes = Integer.parseInt(m.group(1));
            } catch(NumberFormatException ex) {
            }
            try {
            	rank = Double.parseDouble(m.group(2));
            } catch(NumberFormatException ex) {
            }
            try {
            	int movie_id = schema.getMovieByName(imdb_name);
            	batch.add(rank, votes, movie_id);
            } catch (SQLException ex) {
            	//System.out.printf("ERR: %s\n", ex);
            }
        }
        batch.close();
	}

	private void import_genres(File directory) throws IOException, SQLException 
	{
        reader = new ListFileReader(new File(directory, "genres.list"));
        reader.skipUntil("^8: THE GENRES LIST\\s*$");
        
        final Pattern linePattern = Pattern.compile("^(.+?)\\t\\s*(.+)$");
        HashMap<String, Integer> genresMap = new HashMap<String, Integer>();
        
        Batch batch = schema.createBatch("INSERT IGNORE MovieGenres (movie, genre) VALUES (?, ?)");
    	SimpleInsert insertGenre = schema.createInsert("genres", true, "name");
    	SimpleQuery findGenre = schema.createQuery("genre_id", "genres", "name = ?");
    	//SimpleInsert insertMG = schema.createInsert("MovieGenres", true, "movie", "genre");
        
        String prev_imdb_name = "";
        int prev_movie_id = -1;
        while (true) {
        	String line = reader.readLine();
            if (line == null) {
                break;
            }
            line = line.trim();
            if (line.isEmpty()) {
                continue;
            }
            Matcher m = linePattern.matcher(line);
            if (!m.matches()) {
            	continue;
            }
            String imdb_name = m.group(1);
            String genre = m.group(2);
            int movie_id;
            if (imdb_name.equals(prev_imdb_name)) {
            	movie_id = prev_movie_id;
            }
            else {
            	try {
            		movie_id = schema.getMovieByName(imdb_name);
            	} catch (SQLException ex) {
            		continue;
            	}
            }
            int genre_id;
            genre = genre.toLowerCase();
            if (genresMap.containsKey(genre)) {
            	genre_id = genresMap.get(genre);
            }
            else {
            	genre_id = insertGenre.insert(genre);
            	insertGenre.commit();
            	if (genre_id < 0) {
            		genre_id = findGenre.queryGetKey(genre);
            	}
            	genresMap.put(genre, genre_id);
            }
            //System.out.printf("%s / %s\n", movie_id, genre_id);
            batch.add(movie_id, genre_id);
            //insertMG.insert(movie_id, genre_id);
            
            prev_imdb_name = imdb_name;
            prev_movie_id = movie_id;
        }
        insertGenre.close();
        findGenre.close();
        batch.close();
	}
	
	private abstract class ImporterHelper
	{
		protected SimpleInsert people;
		
		public ImporterHelper() throws SQLException {
			people = schema.createInsert("people", true, "imdb_name", "gender");
		}

		public void close() throws SQLException {
			people.close();
		}

		public void doImport(String gender) throws IOException, SQLException
		{
			while (true) {
		        List<String> lines = reader.readUntil("^\\s*$");
		        if (lines == null) {
		        	break;
		        }
		        String line = lines.remove(0);
		        String[] parts = line.split("\t\\s*");
		        if (parts.length != 2) {
		        	continue;
		        }
		        String person_name = parts[0];
		        //System.out.println("person: " + person_name);
		        String movie_info = parts[1];
		        lines.add(0, movie_info);
		        int person_id;
		        try {
			        person_id = people.insert(person_name, gender);
			        if (person_id < 0) {
			        	person_id = schema.getPersonByName(person_name);
			        }
			        for (String ln : lines) {
			        	addMovie(person_id, ln.trim());
			        }
		        } catch (SQLException ex) {
		        	continue;
		        }
			}
		}
		
		abstract protected void addMovie(int person_id, String movie_info) throws SQLException;
	}
	
	private class ActorsImporterHelper extends ImporterHelper
	{
		private final Pattern moviePattern = Pattern.compile(
				"(.+?)\\s+(?:\\[(.+?)\\])??\\s+(?:\\<(\\d+)\\>)??");
		Batch batch = null;
		
		public ActorsImporterHelper() throws SQLException {
			batch = schema.createBatch("INSERT IGNORE INTO roles " +
					"(actor, movie, char_name, credit_pos) VALUES (?, ?, ?, ?)");
		}
		
		@Override
		public void close() throws SQLException {
			batch.close();
			super.close();
		}

		@Override
		protected void addMovie(int person_id, String movie_info) throws SQLException {
			Matcher m = moviePattern.matcher(movie_info);
        	if (!m.matches()) {
        		return;
        	}
        	String imdb_name = m.group(1);
        	String character = m.group(2);
        	int pos = -1;
        	try {
        		pos = Integer.parseInt(m.group(3));
        	} catch (NumberFormatException ex) {
        	}
        	int movie_id = -1;
        	try {
        		movie_id = schema.getMovieByName(imdb_name);
        	} catch (SQLException ex) {
        		return;
        	}
        	//System.out.printf("%s / %s\n", character, pos);
        	batch.add(person_id, movie_id, character, (pos > 0) ? pos : null);
		}
	}

	private class DirectorsImporterHelper extends ImporterHelper
	{
		Batch batch = null;
		
		public DirectorsImporterHelper() throws SQLException {
			batch = schema.createBatch("INSERT IGNORE INTO directors (director, movie) " +
					"VALUES (?, ?, ?, ?)");
		}
		
		@Override
		public void close() throws SQLException {
			batch.close();
			super.close();
		}

		@Override
		protected void addMovie(int person_id, String imdb_name) throws SQLException {
			int movie_id = -1;
			try {
				movie_id = schema.getMovieByName(imdb_name);
			} catch (SQLException ex) {
				return;
			}
			batch.add(person_id, movie_id);
		}
	}
	
	private void import_actors(File directory) throws IOException, SQLException 
	{
        reader = new ListFileReader(new File(directory, "actors.list"));
        reader.skipUntil("^Name\\s+Titles\\s*$");
        reader.skipUntil("^-*\\s+-*\\s*$");
        ActorsImporterHelper helper = new ActorsImporterHelper();
        helper.doImport("m");
        helper.close();

        reader = new ListFileReader(new File(directory, "actresses.list"));
        reader.skipUntil("^Name\\s+Titles\\s*$");
        reader.skipUntil("^-*\\s+-*\\s*$");
        helper = new ActorsImporterHelper();
        helper.doImport("f");
        helper.close();
	}

	private void import_directors(File directory) throws IOException, SQLException 
	{
        reader = new ListFileReader(new File(directory, "directors.list"));
        reader.skipUntil("^\\s*Name\\s+Titles\\s*$");
        reader.skipUntil("^\\s*-+\\s+-+\\s*$");
        DirectorsImporterHelper helper = new DirectorsImporterHelper();
        helper.doImport(null);
        helper.close();
	}
	
    private static final Pattern datePattern = Pattern.compile("(\\d{1,2})??\\s+(\\w+)??\\s+(\\d{4})");
	
    @SuppressWarnings("deprecation")
	private static Date strToDate(String str) {
        Matcher m = datePattern.matcher(str);
		if (!m.matches()) {
			return null;
		}
		int day = 1;
		try {
			if (m.group(1) != null) {
				day = Integer.parseInt(m.group(1));
			}
		} catch (NumberFormatException ex) {
		}
		int month = 1;
		String monthStr = m.group(2);
		if (monthStr != null) {
			monthStr = monthStr.toLowerCase();
			if (monthStr.startsWith("jan")) {
				month = 1;
			} else if (monthStr.startsWith("feb")) {
				month = 2;
			} else if (monthStr.startsWith("mar")) {
				month = 3;
			} else if (monthStr.startsWith("apr")) {
				month = 4;
			} else if (monthStr.startsWith("may")) {
				month = 5;
			} else if (monthStr.startsWith("jun")) {
				month = 6;
			} else if (monthStr.startsWith("jul")) {
				month = 7;
			} else if (monthStr.startsWith("aug")) {
				month = 8;
			} else if (monthStr.startsWith("sep")) {
				month = 9;
			} else if (monthStr.startsWith("oct")) {
				month = 10;
			} else if (monthStr.startsWith("nov")) {
				month = 11;
			} else if (monthStr.startsWith("dec")) {
				month = 12;
			}
		}
		int year = -1;
		try {
			if (m.group(3) != null) {
				year = Integer.parseInt(m.group(3));
			}
		} catch (NumberFormatException ex) {
		}
		if (year < 0) {
			return null;
		}
		return new Date(year - 1900, month, day);
	}
	
	private void import_biographies(File directory) throws IOException, SQLException 
	{
        reader = new ListFileReader(new File(directory, "biographies.list"));
        reader.skipUntil("^---*$");
        final Pattern namePattern = Pattern.compile("(.+?)\\s*(:?,\\s+(.+?))??(?:\\s+(.+))??");
        
        SimpleUpdate people = schema.createUpdate("people", false, 
        		"first_name = ?, middle_name = ?, last_name = ?, real_name = ?, nick_name = ?, " +
        		"birthdate = ?, deathdate = ?", "imdb_name = ?");

        SimpleInsert newPeople = schema.createInsert("people", false, 
        		"imdb_name", "first_name", "middle_name", "last_name", "real_name", "nick_name", 
        		"birthdate", "deathdate");

        while (true) {
        	List<String> lines = reader.readUntil("^---*$");
        	if (lines == null) {
        		break;
        	}
    		int person_id = -1;
    		String imdb_name = null;
    		String first_name = null;
    		String last_name = null;
    		String nick_name = null;
    		String middle_name = null;
    		String real_name = null;
    		Date bdate = null;
    		Date ddate = null;
    		
        	for (String ln : lines) {
        		ln = ln.trim();
        		if (ln.isEmpty()) {
        			continue;
        		}
        		String[] parts = ln.split(":", 1);
        		if (parts.length != 2) {
        			continue;
        		}
        		
        		if (parts[0].equals("NM")) {
        			imdb_name = parts[1].trim();
        			try {
        				person_id = schema.getPersonByName(imdb_name);
        			} catch (SQLException ex) {
        			}
        			Matcher m = namePattern.matcher(imdb_name);
        			if (m.matches()) {
        				last_name = m.group(1);
        				first_name = m.group(2);
        				middle_name = m.group(3);
        			}
        		}
        		else if (parts[0].equals("RN")) {
        			real_name = parts[1].trim();
        		}        		
        		else if (parts[0].equals("NK")) {
        			nick_name = parts[1].trim();
        		}
        		else if (parts[0].equals("DB")) {
        			bdate = strToDate(parts[1].trim());
        		}
        		else if (parts[0].equals("DD")) {
        			ddate = strToDate(parts[1].trim());
        		}
        	}
    		if (imdb_name == null) {
    			continue;
    		}
    		if (person_id < 0) {
    			newPeople.insert(imdb_name, first_name, middle_name, last_name, real_name, 
    					nick_name, bdate, ddate, person_id);
    		}
    		else {
    			people.update(first_name, middle_name, last_name, real_name, nick_name, bdate, 
    					ddate, person_id);
    		}
        }
	}

	static public void main(String[] args) throws IOException, SQLException, ClassNotFoundException
	{
		Schema schema = new Schema("localhost:3306", "pony_imdb", "root", "root");
		
		Importer imp = new Importer(schema);
		imp.import_all(new File("C:\\Users\\sebulba\\Desktop\\db"));
		
	}
}











