package ponytrivia.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;


public class Schema {
	protected Connection conn;

	public Schema(String host, String schema, String username, String password)
			throws SQLException, ClassNotFoundException {
		Class.forName("com.mysql.jdbc.Driver");

		conn = DriverManager.getConnection("jdbc:mysql://" + host + "/"
				+ schema, username, password);
		conn.setAutoCommit(false);
	}
	
	/**
	 * commits to the DB
	 * @throws SQLException
	 */
	public void commit() throws SQLException {
		conn.commit();
	}

	/**
	 * rollbacks to the DB
	 * @throws SQLException
	 */
	public void rollback() throws SQLException {
		conn.rollback();
	}
	
	/***
	 * creates a Statement object
	 * @return Statement
	 * @throws SQLException
	 */
	public Statement createStatement() throws SQLException {
		return conn.createStatement();
	}
	
	/***
	 * creates a PreparedStatement object
	 * @return PreparedStatement
	 * @throws SQLException
	 */
	public PreparedStatement prepareStatement(String sql) throws SQLException {
		return conn.prepareStatement(sql);
	}
	
	/**
	 * creates a batch object
	 * @param sql query passed to prepareStatement
	 * @return a Batch object; don't forget to close the batch when you're done
	 * @throws SQLException
	 */
	public Batch createBatch(String sql) throws SQLException {
		return new Batch(conn.prepareStatement(sql), 500);
	}

	/**
	 * creates a SimpleInsert object
	 * @param table - the table to insert into
	 * @param ignoreErrors - whether to INSERT or INSERT IGNORE
	 * @param columns - the table columns to insert
	 * @return A SimpleInsert object
	 * @throws SQLException
	 */
	public SimpleInsert createInsert(String table, boolean ignoreErrors, String... columns) throws SQLException {
		String cols = "";
		String temp = "";
		for (int i = 0; i < columns.length; i++) {
			cols += columns[i].toString();
			temp += "?";
			if (i != columns.length - 1) {
				cols += ", ";
				temp += ", ";
			}
		}
		return new SimpleInsert(conn.prepareStatement("INSERT " + (ignoreErrors ? "IGNORE" : "") + " INTO " + 
				table + " (" + cols + ") VALUES (" + temp +")", 
				Statement.RETURN_GENERATED_KEYS));
	}
	
	/**
	 * Creates a SimpleUpdate object
	 * @param table - the table to update
	 * @param ignoreErrors - whether to UPDATE or UPDATE IGNORE
	 * @param sets - the set clause (x = 5, y = 6)
	 * @param where - the where clause
	 * @return a SimpleUpdate object
	 * @throws SQLException
	 */
	public SimpleUpdate createUpdate(String table, boolean ignoreErrors, String sets, String where) throws SQLException {
		return new SimpleUpdate(conn.prepareStatement("UPDATE " + (ignoreErrors ? "IGNORE " : "") + 
				table + " SET " + sets + " WHERE " + where));
	}

	/**
	 * creates a SimpleQuery object (4 variants)
	 * @param columns - the columns to select
	 * @param tables - the tables to select from
	 * @param where - the where clause
	 * @return a SimpleQuery object
	 * @throws SQLException
	 */
	public SimpleQuery createQuery(String columns, String tables, String where) throws SQLException {
		return new SimpleQuery(conn.prepareStatement("SELECT " + columns + " FROM " + tables + 
				" WHERE " + where));
	}
	public SimpleQuery createQuery(String columns, String tables, String where, String orderby) throws SQLException {
		return new SimpleQuery(conn.prepareStatement("SELECT " + columns + " FROM " + tables + 
				" WHERE " + where + " ORDER BY " + orderby));
	}
	public SimpleQuery createQuery(String columns, String tables, String where, int limit) throws SQLException {
		return new SimpleQuery(conn.prepareStatement("SELECT " + columns + " FROM " + tables + 
				" WHERE " + where + " LIMIT " + limit));
	}
	public SimpleQuery createQuery(String columns, String tables, String where, String orderby, int limit) throws SQLException {
		return new SimpleQuery(conn.prepareStatement("SELECT " + columns + " FROM " + tables + 
				" WHERE " + where + " ORDER BY " + orderby + " LIMIT " + limit));
	}
	
	private SimpleQuery qGetMovieByName = null;
	
	/**
	 * returns the key of the given movie (by imdb_name)
	 * @param movieName
	 * @return DB key
	 * @throws SQLException, NoResultsFound
	 */
	public int getMovieByName(String movieName) throws SQLException {
		if (qGetMovieByName == null) {
			qGetMovieByName = createQuery("movie_id", "movies", "imdb_name = ?", 1);
		}
		return qGetMovieByName.queryGetKey(movieName);
	}

	private SimpleQuery qGetPersonByName = null;
	
	/**
	 * returns the key of the given person (by imdb_name)
	 * @param personName
	 * @return DB key
	 * @throws SQLException, NoResultsFound
	 */
	public int getPersonByName(String personName) throws SQLException {
		if (qGetPersonByName == null) {
			qGetPersonByName = createQuery("person_id", "people", "imdb_name = ?", 1);
		}
		return qGetPersonByName.queryGetKey(personName);
	}

	private SimpleQuery qGetMovieName = null;
	
	/**
	 * returns the movie name of the given movie id
	 * @param movie_id
	 * @return the movie name or null
	 * @throws SQLException 
	 */
	public String getMovieName(int movie_id) throws SQLException {
		if (qGetMovieName == null) {
			qGetMovieName = createQuery("name", "movies", "movie_id = ?", 1);
		}
		ResultSet rs = qGetMovieName.query(movie_id);
		try {
			if (!rs.next()) {
				throw new NoResultsFound("No movie with id " + movie_id); 
			}
			return rs.getString(1);
		} finally {
			rs.close();
		}
	}
	
	private SimpleQuery qGetPersonName = null;
	
	/**
	 * returns the movie name of the given movie id
	 * @param movie_id
	 * @return the movie name or null
	 * @throws SQLException 
	 */
	public String getPersonName(int person_id) throws SQLException {
		if (qGetPersonName == null) {
			qGetPersonName = createQuery("first_name, middle_name, last_name", "people", "person_id = ?", 1);
		}
		ResultSet rs = qGetPersonName.query(person_id);
		try {
			if (!rs.next()) {
				throw new NoResultsFound("No person with id " + person_id); 
			}
			String name = "";
			for (int i = 1; i <= 3; i++) {
				String n = rs.getString(i);
				if (n != null) {
					name += n + " ";
				}
			}
			return name;
		} finally {
			rs.close();
		}
	}

	private void debug(Object obj) {
		System.out.println(new java.util.Date() + " >> " + obj);
	}

	public void createPopularTables(boolean force) throws SQLException {
		Statement stmt = createStatement();

		if (force) {
			stmt.executeUpdate("DROP TABLE IF EXISTS PopularMovies");
			stmt.executeUpdate("DROP TABLE IF EXISTS PopularDirectors");
			stmt.executeUpdate("DROP TABLE IF EXISTS PopularActors");
		}

		debug("creating PopularMovies");
		try {
			stmt.executeUpdate("CREATE TABLE PopularMovies (popmov_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, " +
					"movie_id INT UNIQUE KEY NOT NULL)");
		} catch (SQLException ex) {
			debug("tables already exist, skipping");
			rollback();
			return; // table already exists
		}
		stmt.executeUpdate("INSERT INTO PopularMovies (movie_id) SELECT movie_id FROM Movies WHERE " +
				"rating >= 6.9 AND votes >= 800 AND year >= 1950 AND is_film = 1");
		debug(stmt.getUpdateCount());
		
		debug("creating PopularDirectors");
		stmt.executeUpdate("CREATE TABLE PopularDirectors (popdir_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT," +
				"director INT UNIQUE KEY NOT NULL)");
		stmt.executeUpdate("INSERT INTO PopularDirectors (director) SELECT DISTINCT D.director " +
				"FROM MovieDirectors as D, PopularMovies as PM WHERE D.movie = PM.movie_id");
		debug(stmt.getUpdateCount());

		debug("creating PopularActors");
		stmt.executeUpdate("CREATE TABLE PopularActors (popact_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT," +
				"actor INT UNIQUE KEY NOT NULL) ");
		stmt.executeUpdate("INSERT INTO PopularActors (actor) SELECT DISTINCT X.actor FROM (" +
				"SELECT R.actor, COUNT(R.movie) AS cnt FROM Roles as R WHERE R.credit_pos <= 18 " +
				"AND R.movie IN (SELECT movie_id FROM PopularMovies) GROUP BY R.actor HAVING cnt >= 3) AS X");
		debug(stmt.getUpdateCount());
		stmt.close();

		commit();
		debug("done");
	}	
}

















