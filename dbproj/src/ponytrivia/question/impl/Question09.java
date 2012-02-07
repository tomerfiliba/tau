package ponytrivia.question.impl;

import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following actors is the youngest?
 * Choose 4 living actors, ordered by age (desc)
 * Living = no date of death
 */
public class Question09 extends QuestionGenerator {
	public Question09(Schema schema) {
		super(schema);
	}

	private SimpleQuery chooseActors = null;

	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseActors == null) {
			chooseActors = schema.createQuery("X.actor", "(SELECT FA.actor, P.birth_date FROM " +
					"FilteredActors AS FA, People AS P WHERE FA.actor = P.person_id AND " +
					"P.birth_date IS NOT NULL AND P.death_date IS NULL ORDER BY rand() LIMIT 4) AS X",
					"true", "X.birth_date DESC");
		}
		
		int[][] res = chooseActors.queryGetInts(4, 1);
		ArrayList<String> wrongAnswers = new ArrayList<String>();
		String person_name = schema.getPersonName(res[0][0]);

		for (int i = 1; i < 4; i++) {
			wrongAnswers.add(schema.getPersonName(res[i][0]));
		}

		return new QuestionInfo("Which of the following actors is the youngest?",
				person_name, wrongAnswers);
	}
}
