package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following movies was produced in <year X>?
 * Choose 4 movies produced in year X
 */
public class Question14 extends QuestionGenerator {
	public Question14(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema
				.executeQuery("select M.movie_id, M.year from filtered_movies as M " +
						"where M.year is not null " +
						"order by rand() limit 1");

		ArrayList<String> wrongAnswers = new ArrayList<String>();
		rs.next();
		int mid = rs.getInt(1);
		int movie_year = rs.getInt(2);
		String movie_name = schema.getMovie(mid);

		for (int i = 0; i < 3; i++) {
			rs.next();
			mid = rs.getInt(1);
			wrongAnswers.add(schema.getMovie(mid));
		}
		rs.close();

		return new QuestionInfo(
				"Which of the following movies was produced in " + movie_year,
				movie_name, wrongAnswers);
	}
}
