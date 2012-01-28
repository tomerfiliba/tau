package ponytrivia.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

//import javax.naming.InitialContext;
//import javax.naming.NamingException;
import javax.sql.DataSource;


public class Schema {
	private Connection conn;
	
	public Schema(String host, String username, String password, String schemaName) throws SQLException, ClassNotFoundException
	{
		Class.forName("com.mysql.jdbc.Driver");

		conn = DriverManager.getConnection("jdbc:mysql://" + host + "/", username, password);
		Statement stmt = conn.createStatement();
		stmt.executeUpdate("CREATE SCHEMA IF NOT EXISTS " + schemaName);
		stmt.close();
		conn.close();

		conn = DriverManager.getConnection("jdbc:mysql://" + host + "/" + schemaName, username, password);
		stmt = conn.createStatement();
		
		stmt.executeUpdate("CREATE OR REPLACE VIEW filtered_movie AS SELECT * FROM movie");
		
		stmt.close();
	}
	
	public enum MovieType
	{
		TV("tv"),
		FILM("film");
		
		public String id;
		private MovieType(String id)
		{
			this.id = id;
		}
	}
	
	public String getMovie(int movie_id) throws SQLException 
	{
		ResultSet rs = executeQuery("select M.name from movie as M where m.idmovie = " + movie_id);
		rs.next();
		return rs.getString(1);
	}

	public String getPerson(int person_id) throws SQLException 
	{
		ResultSet rs = executeQuery("select P.first_name, P.last_name from person as P where p.idperson = " + person_id);
		rs.next();
		return rs.getString(1) + " " + rs.getString(2);
	}

	/*public void addMovie(String full_name, String name, MovieType type, Integer year, 
			String country, String language) throws SQLException
	{
		Statement stmt = conn.createStatement();
		stmt.executeUpdate("INSERT (" + full_name +"," + name + "," + type.id + "," + year + 
				"," + country + "," + language + ") INTO `" + SCHEMA + "`.`movies`");
		stmt.close();
	}*/
	
	

	public Statement createStatement() throws SQLException
	{
		return conn.createStatement();
	}
	
	public int getForeignKey(String sql) throws SQLException
	{
		Statement stmt = createStatement();
		ResultSet rs = stmt.executeQuery(sql);
		if (!rs.next()) {
			throw new SQLException("expected one match");
		}
		int id = rs.getInt(1);
		rs.close();
		stmt.close();
		return id;
	}
	
	public ResultSet executeQuery(String sql) throws SQLException
	{
		Statement stmt = conn.createStatement();
		ResultSet res = stmt.executeQuery(sql);
		//stmt.close();
		return res;
	}

	public Batch createBatch() throws SQLException
	{
		return createBatch(5000);
	}

	public Batch createBatch(int threshold) throws SQLException
	{
		return new Batch(createStatement(), threshold);
	}
}
