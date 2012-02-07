package ponytrivia.question.impl;

import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following actors played in the most movies?
 * Choose an actor with over 10 movies, and 3 with less than 10 movies
 */
public class Question06 extends QuestionGenerator {
	public Question06(Schema schema) {
		super(schema);
	}

	private SimpleQuery chooseActor = null;
	private SimpleQuery chooseWrongs = null;

	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseActor == null) {
			chooseActor = schema.createQuery("PA.actor, COUNT(R.movie) AS cnt", "FilteredActors AS PA, Roles as R",
					"R.actor = PA.actor GROUP BY PA.actor HAVING cnt >= 10 ORDER BY rand() LIMIT 1");
		}
		if (chooseWrongs == null) {
			chooseWrongs = schema.createQuery("PA.actor, COUNT(R.movie) AS cnt", "FilteredActors AS PA, Roles as R",
					"R.actor = PA.actor GROUP BY PA.actor HAVING cnt < 10 ORDER BY rand() LIMIT 3");
		}
		int person_id = chooseActor.queryGetKey();
		String actor = schema.getPersonName(person_id);
		
		int[][] wrongs = chooseWrongs.queryGetInts(3, 1);
		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int i = 0; i < 3; i++) {
			wrongAnswers.add(schema.getPersonName(wrongs[i][0]));
		}

		return new QuestionInfo("Which of the following actors played in the most movies?",
				actor, wrongAnswers);
	}

}
