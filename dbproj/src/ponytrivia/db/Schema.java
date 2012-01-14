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
	//private DataSource dataSource;
	private final String SCHEMA = "test_imdb";
	
	/*public Connection getConnection(String host, String username, String password) 
			throws NamingException, SQLException
	{
		if (conn == null) {
			InitialContext ctx = new InitialContext();
			dataSource = (DataSource)ctx.lookup("java:comp/env/jdbc/MySQLDB");
			conn = dataSource.getConnection("jdbc:mysql://" + host + "/", username, password);
		}
		return conn;
	}*/
	
	public Schema(String host, String username, String password) throws SQLException, ClassNotFoundException
	{
		Class.forName("com.mysql.jdbc.Driver");

		conn = DriverManager.getConnection("jdbc:mysql://" + host + "/", username, password);
		Statement stmt = conn.createStatement();
		stmt.executeUpdate("CREATE SCHEMA IF NOT EXISTS " + SCHEMA);
		stmt.close();
		conn.close();

		conn = DriverManager.getConnection("jdbc:mysql://" + host + "/" + SCHEMA, username, password);
		stmt = conn.createStatement();
		
		stmt.executeUpdate("CREATE OR REPLACE VIEW `" + SCHEMA + "`.`filtered_movie` AS SELECT * FROM movie");
		
		/*stmt.executeUpdate("CREATE TABLE IF NOT EXISTS `" + SCHEMA + "`.`movies` (" +
			"`idmovie` INT NOT NULL AUTO_INCREMENT ," +
			"`full_name` VARCHAR(200) NOT NULL ," +
			"`name` VARCHAR(200) NULL ," +
			"`type` ENUM('tv', 'film') NULL ," +
			"`year` INT NULL ," +
			"`country` VARCHAR(45) NULL ," +
			"`language` VARCHAR(45) NULL ," +
			"PRIMARY KEY (`idmovie`) ," +
			"UNIQUE INDEX `name_UNIQUE` (`full_name` ASC) )" +
			"ENGINE = InnoDB"
		 );
		
		stmt.executeUpdate("CREATE TABLE IF NOT EXISTS `" + SCHEMA + "`.`persons` (" +
			"`idperson` INT NOT NULL AUTO_INCREMENT," +
			"`full_name` VARCHAR(200) NOT NULL ," +
			"`real_name` VARCHAR(200) NULL ," +
			"`nick_name` VARCHAR(200) NULL ," +
			"`sex` ENUM('male', 'female') NULL ," +
			"`height` INT NULL ," +
			"`background` TEXT NULL ," +
			"`birthdate` DATE NULL ," +
			"`deathdate` DATE NULL ," +
			"PRIMARY KEY (`idperson`) ," +
			"UNIQUE INDEX `full_name_UNIQUE` (`full_name` ASC) )" +
			"ENGINE = InnoDB"
		);

		stmt.executeUpdate("CREATE TABLE IF NOT EXISTS `" + SCHEMA + "`.`roles` (" +
			"`idrole` INT NOT NULL AUTO_INCREMENT," +
			"`character` VARCHAR(100) NULL ," +
			"`movieid` INT NULL ," +
			"`personid` INT NULL ," +
			"`credit_position` INT NULL ," +
			"PRIMARY KEY (`idrole`) ," +
			"INDEX `personid` (`personid` ASC) ," +
			"INDEX `movieid` (`movieid` ASC) ," +
			"CONSTRAINT `roles_movieid`" +
			"	FOREIGN KEY (`movieid` )" +
			"	REFERENCES `" + SCHEMA + "`.`movies` (`idmovie` )" +
			"	ON DELETE CASCADE" +
			"	ON UPDATE NO ACTION," +
			"CONSTRAINT `roles_personid`" +
			"	FOREIGN KEY (`personid` )" +
			"	REFERENCES `" + SCHEMA + "`.`persons` (`idperson` )" +
			"	ON DELETE CASCADE" +
			"	ON UPDATE NO ACTION)" +
			"ENGINE = InnoDB"
		);

		stmt.executeUpdate("CREATE TABLE IF NOT EXISTS `" + SCHEMA + "`.`quotes` (" +
			"`idquote` INT NOT NULL AUTO_INCREMENT," +
			"`text` TEXT NULL ," +
			"`speaker` INT NULL ," +
			"PRIMARY KEY (`idquote`) ," +
			"INDEX `speaker` (`speaker` ASC) ," +
			"CONSTRAINT `quotes_speaker`" +
			"	FOREIGN KEY (`speaker` )" +
			"	REFERENCES `" + SCHEMA + "`.`persons` (`idperson` )" +
			"	ON DELETE CASCADE" +
			"	ON UPDATE NO ACTION)" +
			"ENGINE = InnoDB"
		);

		stmt.executeUpdate("CREATE TABLE IF NOT EXISTS `" + SCHEMA + "`.`directors` (" +
			"`iddirector` INT NOT NULL AUTO_INCREMENT," +
			"`movieid` INT NULL ," +
			"`director` INT NULL ," +
			"PRIMARY KEY (`iddirector`) ," +
			"INDEX `movieid` (`movieid` ASC) ," +
			"INDEX `director` (`director` ASC), " +
			"CONSTRAINT `directors_movieid`" +
			"	FOREIGN KEY (`movieid` )" +
			"	REFERENCES `" + SCHEMA + "`.`movies` (`idmovie` )" +
			"	ON DELETE CASCADE" +
			"	ON UPDATE NO ACTION," +
			"CONSTRAINT `directors_director`" +
			"	FOREIGN KEY (`director` )" +
			"	REFERENCES `" + SCHEMA + "`.`persons` (`idperson` )" +
			"	ON DELETE CASCADE" +
			"	ON UPDATE NO ACTION)" +
			"ENGINE = InnoDB"
		);
		
		stmt.close();*/
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

	public void addMovie(String full_name, String name, MovieType type, Integer year, 
			String country, String language) throws SQLException
	{
		Statement stmt = conn.createStatement();
		stmt.executeUpdate("INSERT (" + full_name +"," + name + "," + type.id + "," + year + 
				"," + country + "," + language + ") INTO `" + SCHEMA + "`.`movies`");
		stmt.close();
	}
	
	
	public ResultSet executeQuery(String sql) throws SQLException
	{
		Statement stmt = conn.createStatement();
		ResultSet res = stmt.executeQuery(sql);
		//stmt.close();
		return res;
	}
	
	public void executeUpdate(String sql) throws SQLException
	{
		Statement stmt = conn.createStatement();
		stmt.executeUpdate(sql);
		stmt.close();
	}
	
}
