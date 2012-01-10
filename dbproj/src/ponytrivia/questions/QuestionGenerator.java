package ponytrivia.questions;

import ponytrivia.db.Schema;

public abstract class QuestionGenerator
{
	protected Schema schema;
	public QuestionGenerator(Schema schema)
	{
		this.schema = schema;
	}
	public abstract QuestionInfo generate();
}
