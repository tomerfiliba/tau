package dbcourse2011.importer;

import java.sql.Connection;
import java.sql.DriverManager;



public class Importer
{

	/**
	 * @param args
	 */
	public static void main(String[] args) throws Exception
	{
		ActorsImporter importer = new ActorsImporter("/home/tomer/Desktop/actors.list");
		ActorInfo inf = importer.getNext();
		

		Class.forName("com.mysql.jdbc.Driver");
		Connection conn = DriverManager.getConnection("jdbc:mysql://host_addr:port/schema_name",
			     "myLogin", "myPassword");
		
		
	}

}
