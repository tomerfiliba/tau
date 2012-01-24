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
 * In which of the following movies, the director is also an actor?
 * Choose 3 movies in which the director did not participate
 */
public class Question5 extends QuestionGenerator {
	protected Random rand = new Random();

	public Question5(Schema schema) {
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

		rs.next();
		movie_id =  rs.getInt(1);
		rs.close();

		
		return new QuestionInfo("Who is the director of " + movie_name + "?", 
				schema.getPerson(director_id), wrongAnswers); 
	}

}
