package ponytrivia.importer;

import java.io.File;
import java.io.IOException;
import java.sql.Date;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.HashMap;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import ponytrivia.db.Batch;
import ponytrivia.db.Schema;
import ponytrivia.db.SimpleInsert;
import ponytrivia.db.SimpleQuery;


/**
 * A utility class used to import IMDB list files into a MySQL DB, that will be used by the game.
 * The API of this class includes 
 *   - createSchema() - creates the DB schema
 *   - importLists()
 */
public class Importer {
	protected Schema schema;
	protected ListFileReader reader;

	public Importer(Schema schema) {
		this.schema = schema;
		this.reader = null;
	}

	/**
	 * holds the current filename, file size and file position.
	 * import progress = position / size.
	 * use getStatus() to retrieve an ImportStatus object
	 */
	public static class ImportStatus {
		public final String filename;
		public final long size;
		public final long position;
		
		protected ImportStatus(String filename, long size, long position) {
			this.filename = filename;
			this.size = size;
			this.position = position;
		}
	}

	/**
	 * gets the importer status. this is meant to be invoked by another thread to monitor
	 * the import process.
	 * @return An ImportStatus object or null, if not currently importing
	 * @throws IOException
	 */
	public ImportStatus getStatus() throws IOException
	{
		if (reader == null) {
			return null;
		}
		return new ImportStatus(reader.fileName, reader.getSize(), reader.getPosition());
	}

	public void importLists(String directory) throws IOException, SQLException {
		importLists(new File(directory));
	}

	/**
	 * Imports all lists files from a given directory into the DB
	 * @param directory A File object representing the directory that contains all of the list 
	 *        files. Their names are expected to match those of the IMDB site
	 * @throws IOException
	 * @throws SQLException
	 */
	public void importLists(File directory) throws IOException, SQLException 
	{
		reader = null;
		System.out.printf("%s >> importing movies\n", (new java.util.Date()));
		//import_movies(directory);
		
		System.out.printf("\n%s >> importing ratings\n", (new java.util.Date()));
		//import_ratings(directory);
		
		System.out.printf("\n%s >> importing genres\n", (new java.util.Date()));
		// import_genres(directory);
		
		System.out.printf("\n%s >> importing actors\n", (new java.util.Date()));
		//import_actors(directory);
		
		System.out.printf("\n%s >> importing directors\n", (new java.util.Date()));
		//import_directors(directory);
		
		System.out.printf("\n%s >> importing bios\n", (new java.util.Date()));
		import_biographies(directory);
		reader = null;
		
		System.out.printf("\n%s >> done\n", (new java.util.Date()));
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
        
        //int i = 0;
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
            /*i++;
            if (i % 5000 == 0) {
        		System.out.print(i + ", ");
        		if (i % 50000 == 0) {
        			System.out.println();
        		}
	        }*/
            batch.add(imdb_name, tvshow ? "tv" : "film", name, episode, (year > 1900) ? year : null);
        }
        batch.close();
        reader.close();
	}

	private void import_ratings(File directory) throws IOException, SQLException 
	{
        reader = new ListFileReader(new File(directory, "ratings.list"));
        reader.skipUntil("New\\s+Distribution\\s+Votes\\s+Rank\\s+Title");
        
        final Pattern linePattern = Pattern.compile("^.+?\\s+(\\d+)\\s+(\\d\\.\\d)\\s+(.+)$");
        
        Batch batch = schema.createBatch("UPDATE movies SET rating = ?, votes = ? " +
        		"WHERE imdb_name = ?");
        
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
        	batch.add(rank, votes, imdb_name);
        }
        batch.close();
        reader.close();
	}

	private void import_genres(File directory) throws IOException, SQLException 
	{
        reader = new ListFileReader(new File(directory, "genres.list"));
        reader.skipUntil("^8: THE GENRES LIST\\s*$");
        
        final Pattern linePattern = Pattern.compile("^(.+?)\\t\\s*(.+)$");
        HashMap<String, Integer> genresMap = new HashMap<String, Integer>();
        
        Batch batch = schema.createBatch("INSERT IGNORE MovieGenres (movie, genre) VALUES " +
        		"((SELECT movie_id FROM Movies WHERE imdb_name = ? LIMIT 1), ?)");
        
    	SimpleInsert insertGenre = schema.createInsert("genres", true, "name");
    	SimpleQuery findGenre = schema.createQuery("genre_id", "genres", "name = ?");
        
    	Statement stmt = schema.createStatement();
    	try {
    		stmt.executeUpdate("ALTER TABLE MovieGenres DROP FOREIGN KEY `mg_genre`");
    	} catch (SQLException ex) {
    	}
    	try {
    		stmt.executeUpdate("ALTER TABLE MovieGenres DROP FOREIGN KEY `mg_movie`");
		} catch (SQLException ex) {
		}
    	stmt.getConnection().commit();
    	stmt.close();
        
    	//int i = 0;
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
            /*i++;
            if (i % 5000 == 0) {
            	insertGenre.commit();
        		System.out.print(i + ", ");
        		if (i % 50000 == 0) {
        			System.out.println();
        		}
	        }*/
            batch.add(imdb_name, genre_id);
        }
        insertGenre.close();
        findGenre.close();
        batch.close();
        
    	stmt = schema.createStatement();
    	stmt.executeUpdate("DELETE FROM MovieGenres WHERE movie = 0 OR genre = 0");
    	stmt.executeUpdate("ALTER TABLE MovieGenres ADD CONSTRAINT `mg_movie` "+
		"FOREIGN KEY (`movie`) REFERENCES `movies` (`movie_id`) " +
		"ON DELETE CASCADE ON UPDATE NO ACTION, " +
		"ADD CONSTRAINT `mg_genre` " +
		"FOREIGN KEY (`genre`) REFERENCES `genres` (`genre_id`) " +
		"ON DELETE CASCADE ON UPDATE NO ACTION");
    	stmt.getConnection().commit();        
    	stmt.close();
        
        reader.close();
	}
	
	private abstract class ImporterHelper
	{
		protected SimpleInsert people;
		protected Batch batch = null;
		protected int minEntries = 0;
		
		public ImporterHelper() throws SQLException {
			people = schema.createInsert("people", true, "imdb_name", "gender");
		}

		public void close() throws SQLException {
			if (batch != null) {
				batch.close();
			}
			people.close();
		}

		public void doImport(String gender) throws IOException, SQLException
		{
			int i = 0;
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
		        i++;
		        if (i % 5000 == 0) {
		        	System.out.print(i + ", ");
	        		if (i % 50000 == 0) {
	        			System.out.println();
	        		}
		        }
		        String person_name = parts[0];
		        String movie_info = parts[1];
		        lines.add(0, movie_info);
		        int person_id;
				for (int j = lines.size() - 1; j >=0; j--) {
					if (lines.get(j).charAt(0) == '"') {
						// skip tv shows
						lines.remove(j);
					}
				}
				if (lines.size() < minEntries) {
					// skips people with less than minEntries
					continue;
				}
		        try {
			        person_id = people.insert(person_name, gender);
			        if (person_id < 0) {
			        	person_id = schema.getPersonByName(person_name);
			        }
			        for (String ln : lines) {
			        	addMovie(person_id, ln);
			        }
		        } catch (SQLException ex) {
		        	throw ex;
		        }
			}
		}
		
		abstract protected void addMovie(int person_id, String line) throws SQLException;
	}
	
	private class ActorsImporterHelper extends ImporterHelper
	{
		private final Pattern moviePattern = Pattern.compile(
				"(.+?)\\s+(?:\\[(.+?)\\])??\\s+(?:\\<(\\d+)\\>)??");
		
		public ActorsImporterHelper() throws SQLException {
			minEntries = 6;
			batch = schema.createBatch("INSERT IGNORE INTO roles (actor, movie, char_name, credit_pos) " +
					"VALUES (?, (SELECT movie_id FROM Movies WHERE imdb_name = ? LIMIT 1), ?, ?)");
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
        	batch.add(person_id, imdb_name, character, (pos > 0) ? pos : null);
		}
	}

	private class DirectorsImporterHelper extends ImporterHelper
	{
		public DirectorsImporterHelper() throws SQLException {
			minEntries = 3;
			batch = schema.createBatch("INSERT IGNORE INTO MovieDirectors (director, movie) " +
					"VALUES (?, (SELECT movie_id FROM Movies WHERE imdb_name = ? LIMIT 1))");
		}

		@Override
		protected void addMovie(int person_id, String imdb_name) throws SQLException {
			batch.add(person_id, imdb_name);
		}
	}
	
	private void import_actors(File directory) throws IOException, SQLException 
	{
    	Statement stmt = schema.createStatement();
    	try {
    		stmt.executeUpdate("ALTER TABLE Roles DROP FOREIGN KEY `roles_person`");
    	} catch (SQLException ex) {
    	}
    	try {
    		stmt.executeUpdate("ALTER TABLE Roles DROP FOREIGN KEY `roles_movie`");
		} catch (SQLException ex) {
		}
    	stmt.getConnection().commit();
    	stmt.close();

		reader = new ListFileReader(new File(directory, "actors.list"));
        reader.skipUntil("^Name\\s+Titles\\s*$");
        reader.skipUntil("^-*\\s+-*\\s*$");
        ActorsImporterHelper helper = new ActorsImporterHelper();
        helper.doImport("m");
        helper.close();
        reader.close();

        reader = new ListFileReader(new File(directory, "actresses.list"));
        reader.skipUntil("^Name\\s+Titles\\s*$");
        reader.skipUntil("^-*\\s+-*\\s*$");
        ActorsImporterHelper helper2 = new ActorsImporterHelper();
        helper2.doImport("f");
        helper2.close();
        reader.close();

    	stmt = schema.createStatement();
    	stmt.executeUpdate("DELETE FROM Roles WHERE movie = 0 OR actor = 0");
    	stmt.executeUpdate("ALTER TABLE Roles ADD CONSTRAINT `roles_movie` " +
    	"FOREIGN KEY (`movie`) REFERENCES `movies` (`movie_id`) ON DELETE CASCADE ON UPDATE NO ACTION, " +
    	"ADD CONSTRAINT `roles_person` FOREIGN KEY (`actor`) REFERENCES `people` " +
    	"(`person_id`) ON DELETE CASCADE ON UPDATE NO ACTION");
    	stmt.getConnection().commit();        
    	stmt.close();
	}

	private void import_directors(File directory) throws IOException, SQLException 
	{
    	Statement stmt = schema.createStatement();
    	try {
    		stmt.executeUpdate("ALTER TABLE MovieDirectors DROP FOREIGN KEY `md_movie`");
    	} catch (SQLException ex) {
    	}
    	try {
    		stmt.executeUpdate("ALTER TABLE MovieDirectors DROP FOREIGN KEY `md_people`");
		} catch (SQLException ex) {
		}
    	stmt.getConnection().commit();
    	stmt.close();

		reader = new ListFileReader(new File(directory, "directors.list"));
        reader.skipUntil("^\\s*Name\\s+Titles\\s*$");
        reader.skipUntil("^\\s*-+\\s+-+\\s*$");
        DirectorsImporterHelper helper = new DirectorsImporterHelper();
        helper.doImport(null);
        helper.close();
        reader.close();
        
        stmt = schema.createStatement();
    	stmt.executeUpdate("DELETE FROM MovieDirectors WHERE movie = 0 OR director = 0");
    	stmt.executeUpdate("ALTER TABLE MovieDirectors ADD CONSTRAINT `md_movie` " +
    	"FOREIGN KEY (`movie`) REFERENCES `movies` (`movie_id`) ON DELETE CASCADE ON UPDATE NO ACTION, " +
    	"ADD CONSTRAINT `md_people` FOREIGN KEY (`director`) REFERENCES `people` " +
    	"(`person_id`) ON DELETE CASCADE ON UPDATE NO ACTION");
	}
	
    private static final Pattern datePattern = Pattern.compile("(?:(\\d{1,2})\\s+)??(?:(\\w+)\\s+)??(\\d{4}).*");
	
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
				month = 0;
			} else if (monthStr.startsWith("feb")) {
				month = 1;
			} else if (monthStr.startsWith("mar")) {
				month = 2;
			} else if (monthStr.startsWith("apr")) {
				month = 3;
			} else if (monthStr.startsWith("may")) {
				month = 4;
			} else if (monthStr.startsWith("jun")) {
				month = 5;
			} else if (monthStr.startsWith("jul")) {
				month = 6;
			} else if (monthStr.startsWith("aug")) {
				month = 7;
			} else if (monthStr.startsWith("sep")) {
				month = 8;
			} else if (monthStr.startsWith("oct")) {
				month = 9;
			} else if (monthStr.startsWith("nov")) {
				month = 10;
			} else if (monthStr.startsWith("dec")) {
				month = 11;
			}
			else {
				try {
					month = Integer.parseInt(monthStr);
				} catch (NumberFormatException ex) {
				}
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
        final Pattern namePattern = Pattern.compile("(.+?)\\s*(?:,\\s+(.+?))??(?:\\s+(.+))??");
        
        Batch batch = schema.createBatch("UPDATE people SET first_name = ?, middle_name = ?, " +
        		"last_name = ?, real_name = ?, nick_name = ?, birth_date = ?, death_date = ? " +
        		"WHERE imdb_name = ?");

        SimpleInsert newPeople = schema.createInsert("people", false, 
        		"imdb_name", "first_name", "middle_name", "last_name", "real_name", "nick_name", 
        		"birth_date", "death_date");

		int i = 0;
        while (true) {
        	List<String> lines = reader.readUntil("^---*$", false, false);
        	if (lines == null) {
        		break;
        	}
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
        		if (ln.charAt(2) != ':') {
        			continue;
        		}
        		String key = ln.substring(0,2);
        		String value = ln.substring(3).trim();
        		
        		if (key.equals("NM")) {
        			imdb_name = value;
        			Matcher m = namePattern.matcher(imdb_name);
        			if (m.matches()) {
        				last_name = m.group(1);
        				first_name = m.group(2);
        				middle_name = m.group(3);
        			}
        			if (last_name != null && last_name.length() > 70) {
        				last_name = last_name.substring(0, 70);
        			}
        			if (first_name != null && first_name.length() > 70) {
        				first_name = first_name.substring(0, 70);
        			}
        			if (middle_name != null && middle_name.length() > 70) {
        				middle_name = middle_name.substring(0, 70);
        			}
        		}
        		else if (key.equals("RN")) {
        			if (value.length() > 90) {
        				real_name = value.substring(0, 90);
        			}
        			else {
        				real_name = value;
        			}
        		}
        		else if (key.equals("NK")) {
        			if (value.length() > 90) {
            			nick_name = value.substring(0, 90);
        			}
        			else {
            			nick_name = value;
        			}
        		}
        		else if (key.equals("DB")) {
        			bdate = strToDate(value);
        		}
        		else if (key.equals("DD")) {
        			ddate = strToDate(value);
        		}
        	}
    		if (imdb_name != null) {
	        	i++;
	        	if (i % 5000 == 0) {
	        		System.out.print(i + ", ");
	        		if (i % 50000 == 0) {
	        			System.out.println();
	        		}
	        	}
	        	batch.add(first_name, middle_name, last_name, real_name, nick_name, 
					bdate, ddate, imdb_name);
    		}
        }
        
        newPeople.close();
        batch.close();
        reader.close();
	}
	
	public void createViews() throws SQLException {
		Statement stmt = schema.createStatement();
		try {
			stmt.executeUpdate("CREATE ALGORITHM=TEMPTABLE VIEW PopularMovies AS SELECT movie_id " +
					"FROM Movies WHERE rating >= 7.1 AND year >= 1920 AND votes >= 800");
		} catch (SQLException ex) {
			// the view already exists
		}
	}

	static public void main(String[] args) throws IOException, SQLException, ClassNotFoundException
	{
		Schema schema = new Schema("localhost:3306", "pony_imdb", "root", "root");
		
		Importer imp = new Importer(schema);
		imp.importLists("d:\\imdb-files");
		
	}
}











