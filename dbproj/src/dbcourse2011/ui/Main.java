package dbcourse2011.ui;

import dbcourse2011.db.Schema;
import dbcourse2011.importer.Importer;

public class Main {

	/**
	 * @param args
	 * @throws Exception 
	 */
	public static void main(String[] args) throws Exception {
		// TODO Auto-generated method stub
		Schema db = new Schema("localhost:3306", "root", "root");
		Importer importer = new Importer(db);
		
		//importer.import_all();
		
		
	}

}
