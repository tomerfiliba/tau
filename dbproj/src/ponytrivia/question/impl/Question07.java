package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following movies has the highest rating on IMDB?
 * Choose 4 movies so that no two movies have the same rating in IMDB
 */
public class Question07 extends QuestionGenerator {
	public Question07(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema
				.executeQuery("select M.movie_id from filtered_movies as M "
						+ "where M.rating >= 8.0 order by rand() limit 1");

		rs.next();
		int mid = rs.getInt(1);
		String movie_name = schema.getPerson(mid);
		rs.close();

		rs = schema
				.executeQuery("select M.movie_id from filtered_movies as M "
						+ "where M.rating < 8.0 order by rand() limit 3");

		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int i = 0; i < 3; i++) {
			rs.next();
			mid = rs.getInt(1);
			wrongAnswers.add(schema.getPerson(mid));
		}
		rs.close();

		return new QuestionInfo(
				"Which of the following movies has the highest rating on IMDB?",
				movie_name, wrongAnswers);
	}

}
