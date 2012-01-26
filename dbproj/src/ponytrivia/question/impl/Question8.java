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
 * Which of the following is the latest movie directed by <director X>?
 * Choose 4 movies by director X
 */
public class Question8 extends QuestionGenerator {
	protected Random rand = new Random();

	public Question8(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema
				.executeQuery("select * from (select distinct D.person_id, M.movie_id, M.year "
						+ "from filtered_directors as D, filtered_movies as M "
						+ "where D.movie_id = M.movie_id and M.year > 1930 "
						+ "group by D.person_id "
						+ "having count(M.movie_id) >= 4 "
						+ "order by rand() limit 4) as X" +
						" order by X.year desc");

		ArrayList<String> wrongAnswers = new ArrayList<String>();
		rs.next();
		int pid = rs.getInt(1);
		int mid = rs.getInt(2);
		String person_name = schema.getPerson(pid);
		String movie_name = schema.getPerson(mid);
		rs.close();

		for (int i = 0; i < 3; i++) {
			rs.next();
			mid = rs.getInt(2);
			wrongAnswers.add(schema.getMovie(mid));
		}
		rs.close();

		return new QuestionInfo(
				"Which of the following is the latest movie directed by " + person_name +"?",
				movie_name, wrongAnswers);
	}
}
