package ponytrivia.importer;

import java.io.EOFException;
import java.io.IOException;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import ponytrivia.db.Schema;

public class Importer {
	public Schema schema;

	public Importer(Schema schema) {
		this.schema = schema;
	}

	public void import_all(String directory) throws IOException {
		// import_movies("lists/movies.short");
		// import_actors("lists/actors.short");
		// import_actresses("C:\Documents and
		// Settings\Matan\Desktop\matan-tau\cpu\databases\lists")
	}

	private static void import_movies(String filename) throws IOException {
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^MOVIES\\s+LIST\\s*$");
		parser.skipUntil("^=+\\s*$");

		Pattern line_pat = Pattern
				.compile("(.+?)\\s+(?:\\{(.*?)\\})??(?:\t+(.*))??");
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
			} catch (NumberFormatException ex) {
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

			System.out.println(name + " | " + year);
		}
	}

	private static void import_actors(String filename) throws IOException {
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
				} else {
					String parts[] = line.split("\t+");
					role = parts[0];
				}
			}

		}
	}

	static void import_genres(String filename) throws IOException {
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^8:\\s+THE\\s+GENRES\\s+LIST\\s*$");
		parser.skipUntil("^=+\\s*$");
		parser.readLine();
		Pattern P = Pattern.compile("(.*\\d{4}\\))(.*$)");
		while (true) {
			try {
				List<String> lines = parser.readUntil("^\\s*$");
				if (lines == null)
					break;

				String movie_name = null;

				for (String line : lines) {
					String genre = null;
					String[] helper = null;
					Matcher M = P.matcher(line);

					if (M.matches()) {
						movie_name = M.group(1);
						genre = M.group(2);
						genre = genre.trim();
						if ((genre.startsWith("{"))
								|| ((genre.startsWith("-")))) {
							helper = genre.split("\\}");
							genre = helper[1];
							genre = genre.trim();
						}
					}
					System.out.println(movie_name + " " + genre);
				}
			} catch (EOFException e) {
				break;
			}
		}
	}

	public static void import_directors(String filename) throws IOException {
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^Name\\s+Titles\\s*$");
		parser.skipUntil("^-*\\s+-*\\s*$");

		// Pattern P = Pattern.compile("(.*\\t)(.*\\d.?)(.*$)");
		Pattern P = Pattern.compile("(.*\\t)(.*\\d{4}.?)(.*$)");
		while (true) {
			try {
				List<String> lines = parser.readUntil("^\\s*$");
				if (lines == null)
					break;

				String name = null;

				for (String line : lines) {
					String movie_name = null;
					Matcher M = P.matcher(line);
					if (M.matches()) {
						name = M.group(1);
						movie_name = M.group(2);
						name = name.trim();
						movie_name = movie_name.trim();
					}
					System.out.println(name + "   " + movie_name);
				}
			} catch (EOFException e) {
				break;
			}
		}
	}

	public static void import_countries(String filename) throws IOException {
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^COUNTRIES\\s+LIST\\s*$");
		parser.skipUntil("^=+\\s*$");

		Pattern P = Pattern.compile("(.*\\d{4}\\))(.*$)");
		while (true) {
			try {
				List<String> lines = parser.readUntil("^\\s*$");
				if (lines == null)
					break;

				String movie_name = null;

				for (String line : lines) {
					String country = null;
					String[] helper = null;
					Matcher M = P.matcher(line);

					if (M.matches()) {
						movie_name = M.group(1);
						country = M.group(2);
						country = country.trim();
						if ((country.startsWith("{"))
								|| ((country.startsWith("-")))) {
							helper = country.split("\\}");
							country = helper[1];
							country = country.trim();
						}
					}
					System.out.println(movie_name + " " + country);
				}
			} catch (EOFException e) {
				break;
			}
		}
	}

	public static void import_bios(String filename) throws IOException {
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^BIOGRAPHY\\s+LIST\\s*$");
		// parser.skipUntil("^-*\\s+-*\\s*$");
		parser.skipUntil("^=+\\s*$");
		parser.readLine();

		while (true) {
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

	private static int search_in_line(String line) {
		Pattern p = Pattern.compile("(.*\\w+)\\s(\\d{4})(.*$)");
		Matcher m = p.matcher(line);
		if (m.matches())
			return Integer.parseInt(m.group(2));
		return -1;
	}

	public static void import_ratings(String filename) throws IOException {
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^New\\s+Distribution\\s+Votes\\s+Rank\\s+Title*$");

		Pattern P = Pattern
				.compile("\\s+(\\d+)\\s+(\\d+)\\s+(.*\\d+\\s+)\\s+(\\b+.*$)");
		while (true) {
			try {
				List<String> lines = parser.readUntil("^\\s*$");
				if (lines == null)
					break;

				String movie_name = null;

				for (String line : lines) {
					long votes = 0;
					double rank = 0;

					Matcher M = P.matcher(line);

					if (M.matches()) {
						movie_name = M.group(4);
						votes = Long.parseLong(M.group(2));
						rank = Double.parseDouble(M.group(3));
					}

					System.out.println(movie_name + " " + votes + " " + rank);
				}
			} catch (EOFException e) {
				break;
			}
		}
	}

	public static void import_actresses(String filename) throws IOException {
		ListFileParser parser = new ListFileParser(filename);
		parser.skipUntil("^Name\\s+Titles\\s*$");
		parser.skipUntil("^-*\\s+-*\\s*$");
		parser.readLine();
		parser.readLine();
		Pattern P = Pattern.compile("(^\\s+|.*\\t)(.*$)");

		while (true) {
			try {
				List<String> lines = parser.readUntil("^\\s*$");
				if (lines == null) {
					break;
				}
				String actress_name = null;
				String[] AN;
				String[] MN;
				String copy_movie_name = null;
				for (String line : lines) {

					Matcher M = P.matcher(line);

					String movie_name = null;
					int movie_year = 0;
					String credits = null;
					String first_name = null;
					String last_name = null;
					String copy_first_name = null;
					String copy_last_name = null;
					int place = 0;

					if (M.matches()) {
						actress_name = M.group(1);
						// System.out.println(actress_name+"!");
						movie_name = M.group(2);
						// System.out.println(movie_name+"!");
						place = place_in_credits(movie_name);
						// System.out.println(place + movie_name);
						MN = movie_name.split("\\(");

						movie_name = MN[0];
						if (movie_name.equals(copy_movie_name)) {
							first_name = copy_first_name;
							last_name = copy_last_name;
							continue;
						}
						// System.out.println("C " + copy_movie_name + "M " +
						// movie_name);
						copy_movie_name = movie_name;
						movie_year = Integer.parseInt(MN[1].substring(0, 4));
						// System.out.println(movie_year + movie_name);
						if ((actress_name != null)) {
							if (actress_name.startsWith("  ")) {

								first_name = copy_first_name;
								last_name = copy_last_name;

							}
							if (!actress_name.contains(",")) {
								last_name = actress_name;
								copy_last_name = last_name;
							}

							try {
								AN = actress_name.split(",");
								first_name = AN[1].trim();
								last_name = AN[0].trim();
								copy_first_name = first_name;
								copy_last_name = last_name;
							} catch (ArrayIndexOutOfBoundsException ex) {

							}

						}

					}

					System.out.println("FIRST NAME: " + first_name + " "
							+ "LAST NAME: " + last_name + " movie_name: "
							+ movie_name + "year: " + movie_year
							+ " place_in_credits: " + place);
				}
			} catch (EOFException e) {
				break;
			}
		}

	}

	private static int place_in_credits(String credits) {
		int num = 0;
		try {
			if (credits != null) {

				int first = credits.indexOf("<");
				int last = credits.indexOf(">");
				num = Integer.parseInt(credits.substring(first + 1, last));
			}
		} catch (StringIndexOutOfBoundsException ex) {
			num = 0;
		}
		return num;
	}

}
