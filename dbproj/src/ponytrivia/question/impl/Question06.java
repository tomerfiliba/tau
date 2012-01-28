package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following actors played in the most movies?
 * Choose 4 actors so that no two actors have the same number of movies
 */
public class Question06 extends QuestionGenerator {
	public Question06(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema
				.executeQuery("select count(P.personid) from person as P, role as R "
						+ "where P.personid = R.personid "
						+ "order by RAND() limit 1 "
						+ "group by P.person "
						+ "having count(P.person) >= 30");

		rs.next();
		int person_id = rs.getInt(1);
		String person_name = schema.getPerson(person_id);
		rs.close();

		rs = schema
				.executeQuery("select count(P.personid) from person as P, role as R "
						+ "where P.personid = R.personid "
						+ "order by RAND() limit 3 "
						+ "group by P.person "
						+ "having count(P.person) < 30");

		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int i = 0; i < 3; i++) {
			rs.next();
			int pid = rs.getInt(1);
			wrongAnswers.add(schema.getPerson(pid));
		}
		rs.close();

		return new QuestionInfo(
				"Which of the following actors played in the most movies?",
				person_name, wrongAnswers);
	}

}
