package ponytrivia.Importer;

import java.io.EOFException;
import java.io.IOException;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.sun.xml.internal.fastinfoset.tools.PrintTable;

import ponytrivia.db.Batch;
import ponytrivia.db.Schema;

public class Importer {
	public Schema schema;

	public Importer(Schema schema) {
		this.schema = schema;
	}

	public void import_all(String directory) throws IOException, SQLException {
<<<<<<< HEAD
		// import_movies(directory);
		import_roles(directory);
=======
		import_movies(directory);
	//	import_roles(directory);
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
	}

	protected void import_movies(String directory) throws IOException,
			SQLException {
<<<<<<< HEAD
		// final Pattern line_pat = Pattern
		// .compile("(.+?)\\s+\\((\\d+)\\)\\s+(?:\\{(.*?)\\})??");
		// ("(.+?)\\s+\\((\\d+)\\)\\s+(\\{(.*?)\\})??(.*)??");
		final Pattern line_pat = Pattern
				.compile("(.+?)\\s+\\((\\d+)\\)\\s+(\\{.*\\})??\\s+(.*)?");
		ListFileParser parser = new ListFileParser(directory + "/movies.short");

=======
		//final Pattern line_pat = Pattern
			//	.compile("(.+?)\\s+\\((\\d+)\\)\\s+(?:\\{(.*?)\\})??");
		final Pattern line_pat = Pattern
		.compile("(.+?)\\s+\\((\\d+)\\)\\s+(\\{(.*?)\\})??(.*)??");
		ListFileParser parser = new ListFileParser(directory + "/movies_short.txt");
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
		parser.skipUntil("^MOVIES\\s+LIST\\s*$");
		parser.skipUntil("^=+\\s*$");

		Batch batch = schema.createBatch();
		for (int i = 0; i < 500; i++) {

			// while (true) {
			String line = parser.readLine();
			if (line == null) {
				break;
			}
			line=line.replace("'"," ");
			if (line.trim().isEmpty()) {
				continue;
			}
			Matcher m = line_pat.matcher(line);
			if (!m.matches()) {
				continue;
			}

			String name1 = "'" + m.group(1) + "'";
			String name = m.group(1);
			String year_s = m.group(2);
			String episode = m.group(3);
			boolean tvshow = name.startsWith("\"");
			int year = -1;

			try {
				year = Integer.parseInt(year_s);
			} catch (NumberFormatException ex) {
			}
<<<<<<< HEAD
			String full_name = "'" + name + "&" + year + "&" + episode + "'";
			try {
				// System.out.println(full_name + " " + " " + name1 + " " +
				// year);
				batch.add("INSERT IGNORE INTO Movies (imdb_name, type, name, year) "
						+ "VALUES("
						+ full_name
						+ ", "
						+ (tvshow ? "true" : "false")
						+ ", "
						+ name1
						+ ", "
						+ (year > 1900 ? year : "NULL") + ")");

			} catch (SQLException e) {

			}
=======

			batch.add("INSERT IGNORE INTO Movies (imdb_name, type, name, year) "
					+ "VALUES ("
					+ full_name
					+ ", "
					+ (tvshow ? "true" : "false")
					+ ", "
					+ name
					+ ", "
					+ (year > 1900 ? year : "NULL") + ")");
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
		}
		batch.close();
	}

<<<<<<< HEAD
	// gender: 0 - f, 1 - m
	protected void _import_actors(ListFileParser parser, int gender)
			throws IOException, SQLException {
		Pattern title_pat = Pattern
				.compile("(.+)\\s+\\((\\d+)\\)(\\s+)?(\\(.+\\))?(\\s+)?(\\{.+\\})?(\\s+)?(\\[(.+?)\\])??(\\s+)?(\\<(\\d+?)\\>)??");
=======
	//gender: 0 - f, 1 - m 
	protected void _import_actors(ListFileParser parser,int gender) throws IOException {
		Pattern title_pat = Pattern
				.compile("(.+)\\s+\\((\\d+)\\)\\s+(\\[(.+)\\])??\\s*(\\<(\\d+)\\>)??");

		Batch batch = schema.createBatch();
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8

		Batch batch = schema.createBatch();
		for (int i = 0; i < 500; i++) {
			// while (true) {
			List<String> lines = parser.readUntil("^\\s*$");
			if (lines == null) {
				break;
			}
			String imdb_name = null;
			int pid = 0;
<<<<<<< HEAD
			String first_name = null;
=======
			String first_name;
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
			String last_name;
			for (String line : lines) {
				line=line.replace("'"," ");
				String title;
				String character = null;
<<<<<<< HEAD
				String episode;
				int credits = 0;
				String parts[] = line.split("\t+");
=======
				int credits = 0;
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
				if (imdb_name == null) {
					imdb_name = parts[0];
					//imdb_name= imdb_name.replace("'"," ");
					title = parts[1];
<<<<<<< HEAD
					if (imdb_name.contains(",")) {
						String names[] = imdb_name.split(",");
						first_name = names[1];
						last_name = names[0];
						
						if (last_name.startsWith("-")) {
							last_name = null;
						}
					} else
						last_name = imdb_name;
					try {
					//	System.out.println(imdb_name);
						schema.executeUpdate("INSERT IGNORE INTO People (imdb_name,first_name,last_name,gender)"
								+ " VALUES ("
								+ "'"
								+ imdb_name
								+ "'"
								+ ", "
								+ "'"
								+ first_name
								+ "'"
								+ ", "
								+ "'"
								+ last_name
								+ "'" + "," + gender + ")");
						
						pid = schema
							.getForeignKey("SELECT P.idperson FROM People as P where P.imdb_name = "
										+ "'" + imdb_name + "'");
					} catch (SQLException e) {
						// TODO Auto-generated catch block
						System.out.println(e.toString());
					//	System.out.println(imdb_name);
					//	return;
=======
					String names[] = imdb_name.split(",");
					first_name = names[1];
					last_name = names[0];
					try {
						schema.executeUpdate("INSERT IGNORE INTO People (imdb_name,first_name,last_name,gender)"
								+ " VALUES ("
								+ imdb_name
								+ ", "
								+ first_name
								+ ", " + last_name + ")");

						pid = schema
								.getForeignKey("SELECT P.id FROM People as P where P.imdb_name = "
										+ imdb_name);
					} catch (SQLException e) {
						// TODO Auto-generated catch block
						continue;
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
					}

				} else {
					title = parts[1];
				}
				
				Matcher m = title_pat.matcher(title);
				if (!m.matches())
					continue;
				int year = -1;
				String movie_name = m.group(1);
<<<<<<< HEAD
				String year_s = m.group(2);
				try {
					year = Integer.parseInt(year_s);
				} catch (NumberFormatException ex) {
				}
				episode = m.group(6);
				character = m.group(9);
				if (m.group(12) != null) {
					credits = Integer.parseInt(m.group(12));
				}
				movie_name= movie_name + "&" + year + "&" + episode;
				int mid = 0;
				try {
					movie_name="\"$#*! My Dad Says\"&2010&{Easy, Writer (#1.6)}";
					mid = schema
							.getForeignKey("SELECT M.idmovie from movies as M where "
=======
				String year = m.group(2);
				try {

					character = m.group(4);
					credits = Integer.parseInt(m.group(6));
				} catch (IndexOutOfBoundsException e) {
					// if (character==null)
				}

				try {
					int mid = schema
							.getForeignKey("SELECT M.movie_id from movies as M where "
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
									+ "M.imdb_name = '" + movie_name + "'");

					batch.add("INSERT IGNORE INTO Roles (actor, movie,character,credit_pos) VALUES ("
							+ pid
							+ ", "
							+ mid
							+ ", "
<<<<<<< HEAD
							+ "'"
							+ character
							+ "'"
							+ ", "
							+ credits);
				} catch (SQLException e) {
					System.out.println(e.toString());
					// TODO Auto-generated catch block
					System.out.println(mid);
					return;
				}

=======
							+ character
							+ ", "
							+ credits);
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					continue;
				}
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
			}
		}
		batch.close();
	}
<<<<<<< HEAD

	protected void import_roles(String directory) throws IOException,
			SQLException {
		ListFileParser parser = new ListFileParser(directory + "/actors.short");
		parser.skipUntil("^Name\\s+Titles\\s*$");
		parser.skipUntil("^-*\\s+-*\\s*$");
		_import_actors(parser, 1);

		// parser = new ListFileParser(directory + "/actresses.list");
		// parser.skipUntil("^Name\\s+Titles\\s*$");
		// parser.skipUntil("^-*\\s+-*\\s*$");
		// _import_actors(parser, 0);
=======

	protected void import_roles(String directory) throws IOException {
		ListFileParser parser = new ListFileParser(directory + "/actor.list");
		parser.skipUntil("^Name\\s+Titles\\s*$");
		parser.skipUntil("^-*\\s+-*\\s*$");
		_import_actors(parser,1);

		parser = new ListFileParser(directory + "/actresses.list");
		parser.skipUntil("^Name\\s+Titles\\s*$");
		parser.skipUntil("^-*\\s+-*\\s*$");
		_import_actors(parser,0);
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
	}

	protected void import_genres(String directory) throws IOException,
			SQLException {
		ListFileParser parser = new ListFileParser(directory + "/genres.list");
		parser.skipUntil("^8:\\s+THE\\s+GENRES\\s+LIST\\s*$");
		parser.skipUntil("^=+\\s*$");
		parser.readLine();
		Batch batch = schema.createBatch();
		while (true) {

			String line = parser.readLine();
			if (line == null) {
				break;
			}
			int gid;

			String[] parts = line.split("\\t");
			String genre = parts[1];
			String imdb_name = parts[0];
			try {
				schema.executeUpdate("INSERT IGNORE INTO genre (genre)"
						+ " VALUES (" + genre + ")");

				gid = schema
						.getForeignKey("SELECT genre.id FROM genre as G where G.genre = "
								+ genre);

				int mid = schema
						.getForeignKey("SELECT M.movie_id from movies as M where "
								+ "M.imdb_name = '" + imdb_name + "'");

				batch.add("INSERT IGNORE INTO movie_genre (movie_id,genre_id) VALUES ("
						+ mid + ", " + gid + ")");
			} catch (SQLException e) {
				// TODO Auto-generated catch block
				continue;
			}
		}

		batch.close();
	}

<<<<<<< HEAD
	protected void import_directors(String directory) throws IOException,
			SQLException {
=======
	protected void import_directors(String directory) throws IOException, SQLException {
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
		ListFileParser parser = new ListFileParser(directory
				+ "/directors.list");
		Pattern title_pat = Pattern
				.compile("(.+)\\s+\\((\\d+)\\)\\s+(\\[(.+)\\])??\\s*(\\<(\\d+)\\>)??");
<<<<<<< HEAD

		Batch batch = schema.createBatch();

		while (true) {
			List<String> lines = parser.readUntil("^\\s*$");
			if (lines == null) {
				break;
			}
			String imdb_name = null;
			int pid = 0;
			String first_name;
			String last_name;
			for (String line : lines) {
				String title;

				if (imdb_name == null) {
					String parts[] = line.split("\t+");
					imdb_name = parts[0];
					title = parts[1];
					String names[] = imdb_name.split(",");
					first_name = names[1];
					last_name = names[0];
					try {
						schema.executeUpdate("INSERT IGNORE INTO People (imdb_name,first_name,last_name)"
								+ " VALUES ("
								+ imdb_name
								+ ", "
								+ first_name
								+ ", " + last_name + ")");

						pid = schema
								.getForeignKey("SELECT P.id FROM People as P where P.imdb_name = "
										+ imdb_name);
					} catch (SQLException e) {
						// TODO Auto-generated catch block
						continue;
					}

				} else {
					String parts[] = line.split("\t+");
					title = parts[0];
				}
				Matcher m = title_pat.matcher(title);
				String movie_name = m.group(1);

=======

		Batch batch = schema.createBatch();

		while (true) {
			List<String> lines = parser.readUntil("^\\s*$");
			if (lines == null) {
				break;
			}
			String imdb_name = null;
			int pid = 0;
			String first_name;
			String last_name;
			for (String line : lines) {
				String title;
				
				if (imdb_name == null) {
					String parts[] = line.split("\t+");
					imdb_name = parts[0];
					title = parts[1];
					String names[] = imdb_name.split(",");
					first_name = names[1];
					last_name = names[0];
					try {
						schema.executeUpdate("INSERT IGNORE INTO People (imdb_name,first_name,last_name)"
								+ " VALUES ("
								+ imdb_name
								+ ", "
								+ first_name
								+ ", " + last_name + ")");

						pid = schema
								.getForeignKey("SELECT P.id FROM People as P where P.imdb_name = "
										+ imdb_name);
					} catch (SQLException e) {
						// TODO Auto-generated catch block
						continue;
					}

				} else {
					String parts[] = line.split("\t+");
					title = parts[0];
				}
				Matcher m = title_pat.matcher(title);
				String movie_name = m.group(1);

>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
				try {
					int mid = schema
							.getForeignKey("SELECT M.movie_id from movies as M where "
									+ "M.imdb_name = '" + movie_name + "'");

					batch.add("INSERT IGNORE INTO directors (preson_id,movie_id) VALUES ("
<<<<<<< HEAD
							+ pid + ", " + mid);
=======
							+ pid
							+ ", "
							+ mid
							);
>>>>>>> e50c4f3af7dcb5caf5ee1dc99d9dd725885dade8
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					continue;
				}
			}
		}
		batch.close();
	}

	public void import_bios(String filename) throws IOException, SQLException {
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^BIOGRAPHY\\s+LIST\\s*$");
		// parser.skipUntil("^-*\\s+-*\\s*$");
		parser.skipUntil("^=+\\s*$");
		parser.readLine();

		while (true) {
			int pid;
			try {
				List<String> lines = parser.readUntil("^\\s*$");
				;
				// List<String> lines =
				// parser.readUntil("---------------------------");
				// parser.readUntil("^\\s*$");
				if (lines == null)
					break;
				int i = 0;
				// System.out.println(lines.get(0));
				// System.out.println(lines);
				// String paragraph =
				for (String line : lines) {
					int b_year = 0;
					int d_year = 0;
					String NM = null;
					String RN = null;
					String DD = null;
					String DB = null;
					String NK = null;

					if (line.startsWith("\\s\\s") || (line.startsWith("--")))
						continue;

					if (line.startsWith("NM")) {
						NM = line.substring(3);
						pid = schema
								.getForeignKey("SELECT P.id FROM People as P where P.imdb_name = "
										+ NM);
					}
					if (line.startsWith("RN")) {
						RN = line.substring(3);

					}
					if (line.startsWith("DB")) {
						DB = line.substring(3);
						b_year = search_in_line(DB);

					}
					if (line.startsWith("NK")) {
						NK = line.substring(3);

					}
					if (line.startsWith("DD")) {
						DD = line.substring(3);
						d_year = search_in_line(DD);
					}

					System.out.println(NM + " " + RN + " " + NK + " " + b_year
							+ " " + d_year);
					// System.out.println(year);
				}

			} catch (EOFException e) {
				break;
			}
		}
	}

	private int search_in_line(String line) {
		Pattern p = Pattern.compile("(.*\\w+)\\s(\\d{4})(.*$)");
		Matcher m = p.matcher(line);
		if (m.matches())
			return Integer.parseInt(m.group(2));
		return -1;
	}

	public void import_ratings(String filename) throws IOException,
			SQLException {
		final Pattern line_pat = Pattern
				.compile("\\s+(\\d+)\\s+(\\d+)\\s+(.*\\d+\\s+)\\s+(.*)$");
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^New\\s+Distribution\\s+Votes\\s+Rank\\s+Title*$");
		Batch batch = schema.createBatch();

		while (true) {
			String line = parser.readLine();
			if (line == null) {
				break;
			}
			Matcher m = line_pat.matcher(line);
			if (!m.matches()) {
				continue;
			}
			String full_name = m.group(4);
			int votes = Integer.parseInt(m.group(2));
			double rating = Double.parseDouble(m.group(3));
			int mid;
			try {
				mid = schema
						.getForeignKey("select M.movie_id from Movies as M "
								+ "where M.imdb_name = '" + full_name + "'");
			} catch (SQLException e) {
				continue;
			}
			batch.add("UPDATE Movies SET rating = " + rating + ", votes = "
					+ votes + " WHERE movie_id = " + mid);
		}
		batch.close();
	}
}