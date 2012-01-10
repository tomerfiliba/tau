package ponytrivia.questions;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

import ponytrivia.db.Schema;


public class QuestionRegistry {
	protected Schema schema;
	protected List<QuestionGenerator> questionRegistry;
	protected Random rand;
	protected Set<String> questionHistory;
	
	public QuestionRegistry(Schema schema) {
		questionRegistry = new ArrayList<QuestionGenerator>();
		rand = new Random();
		questionHistory = new HashSet<String>();
		
		questionRegistry.add(new Question1(schema));
	}
	
	public QuestionInfo getQuestion()
	{
		QuestionInfo qi = null;
		for (int i = 0; i < 5; i++) {
			int ind = rand.nextInt() % questionRegistry.size();
			QuestionGenerator qg = questionRegistry.get(ind);
			qi = qg.generate();
			if (!questionHistory.contains(qi.questionText)) {
				questionHistory.add(qi.questionText);
				break;
			}
		}
		return qi;
	}
	
}

