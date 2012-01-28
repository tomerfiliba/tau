package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * In which of the following movies, the director is also an actor?
 * Choose 3 movies in which the director did not participate
 */
public class Question05 extends QuestionGenerator {
	public Question05(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema.executeQuery("select M.movie_id from filtered_movies as M, Movies_directors as D " +
				"where M.movie_id = D.movie and D.director in (select A.person from actors as A " +
											"where A.movie = M.movie_id) " +
				"order by rand() limit 1");

		rs.next();
		int movie_id =  rs.getInt(1);
		String movie_name = schema.getMovie(movie_id);
		rs.close();

		rs = schema.executeQuery("select M.movie_id from filtered_movies as M, Movies_directors as D " +
				"where M.movie_id = D.movie and D.director not in (select A.person from actors as A " +
											"where A.movie = M.movie_id) " +
				"order by rand() limit 3");

		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int i = 0; i < 3; i++) {
			rs.next();
			movie_id =  rs.getInt(1);
			wrongAnswers.add(schema.getMovie(movie_id));
		}
		rs.close();

		return new QuestionInfo("In which of the following movies, did the director also play?", 
				movie_name, wrongAnswers); 
	}

}
