package ponytrivia.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
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

	public Statement createStatement() throws SQLException {
		return conn.createStatement();
	}
	public PreparedStatement prepareStatement(String sql) throws SQLException {
		return conn.prepareStatement(sql);
	}
	
	public Batch createBatch(String sql) throws SQLException {
		return new Batch(conn.prepareStatement(sql), 2000);
	}

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
	public SimpleUpdate createUpdate(String table, boolean ignoreErrors, String sets, String where) throws SQLException {
		return new SimpleUpdate(conn.prepareStatement("UPDATE " + (ignoreErrors ? "IGNORE " : "") + 
				table + " SET " + sets + " WHERE " + where));
	}

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
	public int getMovieByName(String movieName) throws SQLException {
		if (qGetMovieByName == null) {
			qGetMovieByName = createQuery("movie_id", "movies", "imdb_name = ?");
		}
		return qGetMovieByName.queryGetKey(movieName);
	}

	private SimpleQuery qGetPersonByName = null;
	public int getPersonByName(String personName) throws SQLException {
		if (qGetPersonByName == null) {
			qGetPersonByName = createQuery("person_id", "people", "imdb_name = ?");
		}
		return qGetPersonByName.queryGetKey(personName);
	}
}

















