package ponytrivia.question;

import java.sql.SQLException;

import ponytrivia.db.Schema;

/**
 * an abstract quesiton generator -- needs to implement generate()
 */
public abstract class QuestionGenerator
{
	protected Schema schema;
	
	public QuestionGenerator(Schema schema)
	{
		this.schema = schema;
	}
	
	public abstract QuestionInfo generate() throws SQLException;
}
