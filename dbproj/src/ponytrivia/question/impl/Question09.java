package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following is the latest movie in which <actor X> played?
 * Choose 4 movies in which actor X has played, from different years
 */
public class Question09 extends QuestionGenerator {
	public Question09(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema
				.executeQuery("select A.person from filtered_actors as A "
						+ "order by rand() limit 1");
		rs.next();
		int person_id = rs.getInt(1);
		String person_name = schema.getPerson(person_id);
		rs.close();
		
		rs = schema.executeQuery("select X.idmovie from("
				+ "select M.idmovie, M.year from movie as M, role as R "
				+ "where M.idmovie = R.movie_id AND R.person_id = " + person_id
				+ " AND M.year > 1930 " + "order by rand() limit 4) as X "
				+ "order by X.year desc");

		ArrayList<String> wrongAnswers = new ArrayList<String>();
		rs.next();
		int mid = rs.getInt(1);
		String movie_name = schema.getMovie(mid);

		for (int i = 0; i < 3; i++) {
			rs.next();
			mid = rs.getInt(1);
			wrongAnswers.add(schema.getMovie(mid));
		}
		rs.close();

		return new QuestionInfo(
				"Which of the following is the latest movie in which " + person_name + " played?"
						+ person_name + "?", movie_name, wrongAnswers);
	}
}
