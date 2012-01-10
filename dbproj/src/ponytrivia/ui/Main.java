package ponytrivia.ui;

import ponytrivia.db.Schema;
import ponytrivia.importer.Importer;
import ponytrivia.questions.QuestionRegistry;

public class Main {

	/**
	 * @param args
	 * @throws Exception 
	 */
	public static void main(String[] args) throws Exception {
		// TODO Auto-generated method stub
		Schema db = new Schema("localhost:3306", "root", "root");
		//Importer importer = new Importer(db);
		
		//importer.import_all();
		/*QuestionRegistry qr = new QuestionRegistry(db);
		System.out.println(qr.getQuestion());*/
	}

}
