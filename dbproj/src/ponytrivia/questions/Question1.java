package ponytrivia.questions;

import ponytrivia.db.Schema;

public class Question1 extends QuestionGenerator {

	public Question1(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() {
		return new QuestionInfo("what is your name", 
				new String[]{"sir lancelot", "galahad", "king arthur", "i don't know"}, 
				2);
	}

}
