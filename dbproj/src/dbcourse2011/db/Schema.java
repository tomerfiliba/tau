package dbcourse2011.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;


public class Schema {
	private Connection conn;
	private final String SCHEMA = "imdb";
	
	public Schema(String host, String username, String password) throws SQLException, ClassNotFoundException
	{
		Class.forName("com.mysql.jdbc.Driver");

		conn = DriverManager.getConnection("jdbc:mysql://" + host + "/", username, password);
		Statement stmt = conn.createStatement();
		stmt.executeUpdate("CREATE SCHEMA IF NOT EXISTS imdb");
		stmt.close();
		conn.close();

		conn = DriverManager.getConnection("jdbc:mysql://" + host + "/" + SCHEMA, username, password);
		stmt = conn.createStatement();
		
		stmt.executeUpdate("CREATE TABLE IF NOT EXISTS `" + SCHEMA + "`.`movies` (" +
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
		
		stmt.close();
	}
	
	public ResultSet executeQuery(String sql) throws SQLException
	{
		Statement stmt = conn.createStatement();
		ResultSet res = stmt.executeQuery(sql);
		stmt.close();
		return res;
	}
	
	public void executeUpdate(String sql) throws SQLException
	{
		Statement stmt = conn.createStatement();
		stmt.executeUpdate(sql);
		stmt.close();
	}
	
}
