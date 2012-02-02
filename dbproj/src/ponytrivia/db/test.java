package ponytrivia.db;

import java.io.IOException;
import java.sql.SQLException;


import ponytrivia.importer.*;
public class test {

	/**
	 * @param args
	 * @throws ClassNotFoundException
	 * @throws SQLException
	 * @throws IOException 
	 */
	public static void main(String[] args) throws SQLException, ClassNotFoundException, IOException {
		// TODO Auto-generated method stub
		final String SCHEMA = "test_imdb";
		Schema s=new Schema("localhost:3306","root","mnmnmn");
		
		Importer import1=new Importer(s);
		String directory="C:"+ 
		"\\"+
		"matan-tau\\" +
		"cpu\\" +
		"databases\\" +
		"lists\\";
		String directory1="C:"+ 
		"\\"+ "Documents and Settings\\" +
		"matan\\" +
		"workspace\\" +
		"dbproj\\" +
		"lists\\";
		import1.import_all(directory1);
//		s.executeUpdate(sql);
//		s.executeUpdate(wish);
	}
}
