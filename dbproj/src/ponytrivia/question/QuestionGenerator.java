package ponytrivia.question;

import java.sql.SQLException;

import ponytrivia.db.Schema;

public abstract class QuestionGenerator
{
	protected Schema schema;
	//protected Random rand = new Random();
	
	public QuestionGenerator(Schema schema)
	{
		this.schema = schema;
	}
	
	public abstract QuestionInfo generate() throws SQLException;
}
