package ponytrivia.question;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;

import ponytrivia.db.Schema;
import ponytrivia.question.impl.Question02;
import ponytrivia.question.impl.Question03;
import ponytrivia.question.impl.Question04;
import ponytrivia.question.impl.Question05;
import ponytrivia.question.impl.Question06;
import ponytrivia.question.impl.Question07;
import ponytrivia.question.impl.Question08;
import ponytrivia.question.impl.Question09;
import ponytrivia.question.impl.Question12;
import ponytrivia.question.impl.Question13;
import ponytrivia.question.impl.Question14;


public class QuestionRegistry {
	protected Random rand;
	protected List<QuestionGenerator> questionRegistry;
	protected Set<String> questionHistory;
	
	public QuestionRegistry(Schema schema) {
		questionRegistry = new ArrayList<QuestionGenerator>();
		rand = new Random();
		questionHistory = new HashSet<String>();
		
		questionRegistry.add(new Question02(schema));
		questionRegistry.add(new Question03(schema));
		/*questionRegistry.add(new Question04(schema));
		questionRegistry.add(new Question05(schema));
		questionRegistry.add(new Question06(schema));
		questionRegistry.add(new Question07(schema));
		questionRegistry.add(new Question08(schema));
		questionRegistry.add(new Question09(schema));
		questionRegistry.add(new Question12(schema));
		questionRegistry.add(new Question13(schema));
		questionRegistry.add(new Question14(schema));*/
	}
	
	public QuestionInfo getQuestion() throws SQLException
	{
		final int retries = 50;
		QuestionInfo qi = null;
		
		for (int i = 0; i < retries; i++) {
			int ind = Math.abs(rand.nextInt()) % questionRegistry.size();
			QuestionGenerator qg = questionRegistry.get(ind);
			try {
				qi = qg.generate();
			} catch (SQLException ex) {
				if (i >= retries - 1) {
					throw ex;
				}
				continue;
			}
			if (!questionHistory.contains(qi.questionText)) {
				questionHistory.add(qi.questionText);
				break;
			}
		}
		return qi;
	}
	
}

