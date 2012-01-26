package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following actors is the oldest?
 * Choose 4 living actors
 * Living = no date of death, and age < 85
 */
public class Question12 extends QuestionGenerator {
	public Question12(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema
				.executeQuery("select X.person_id from ( "
						+ "	select A.person_id, P.birthdate from filtered_actors as A, person as P "
						+ "	where A.person_id = P.person_id and P.birthdate > 1930 and P.deathdate is null "
						+ "	order by rand() limit 4) as X "
						+ "order by X.birthdate asc");

		ArrayList<String> wrongAnswers = new ArrayList<String>();
		rs.next();
		int pid = rs.getInt(1);
		String person_name = schema.getPerson(pid);

		for (int i = 0; i < 3; i++) {
			rs.next();
			pid = rs.getInt(1);
			wrongAnswers.add(schema.getPerson(pid));
		}
		rs.close();

		return new QuestionInfo(
				"Which of the following actors is the oldest?",
				person_name, wrongAnswers);
	}
}
