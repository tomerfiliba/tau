package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * In which of the following movies did <actor X> play?
 * (Choose 3 movies in which X did not play)
 */
public class Question02 extends QuestionGenerator {
	public Question02(Schema schema) {
		super(schema);
	}
	
	private SimpleQuery chooseActor = null;
	
	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseActor == null) {
			chooseActor = schema.createQuery("P.person_id, M.movie_id", 
				"People as P, Roles as R, FilteredMovie as M", 
				"P.person_id = R.actor and R.movie = M.movie_id", "rand()", 1);
		}
		
		ResultSet rs = chooseActor.query();
		rs.next();
		int person_id = rs.getInt(1);
		int movie_id = rs.getInt(2);
		String right_answer = 
		
		String right_answer = schema.getMovieNameByID(movie_id);
		String person = schema.getPerson(person_id);

		rs = schema.executeQuery("select M.idmovie from filtered_movie as M " +
				"where M.idmovie not in ( " +
				"   select M.idmovie from movie as M, role as R, Person as P " +
				"    where M.idmovie = R.movie_id and R.person_id = " + person_id + ") " +
				"order by rand() " +
				"limit 3");
		ArrayList<String> wrong_answers = new ArrayList<String>();
		while (rs.next()) {
			movie_id = rs.getInt(1);
			wrong_answers.add(schema.getMovie(movie_id));
		}
		
		return new QuestionInfo("In which of the following movies did " + person + " play?", 
				right_answer, wrong_answers); 
	}

}
