package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * In which of the following movies did <actor X> play?
 * (Choose 3 movies in which X did not play)
 */
public class Question2 extends QuestionGenerator {

	public Question2(Schema schema) {
		super(schema);
	}
	
	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema.executeQuery("select P.idperson, M.idmovie from person as P, role as R, filtered_movie as M " +
			"WHERE P.idperson = R.person_id and R.movie_id = M.idmovie " +
			"ORDER BY RAND() LIMIT 1");

		rs.next();
		int person_id = rs.getInt(1);
		int movie_id =  rs.getInt(2);
		
		String right_answer = schema.getMovie(movie_id);
		String person = schema.getPerson(person_id);

		rs = schema.executeQuery("select distinct(M.idmovie) from filtered_movie as M, role as R " + 
				"where R.movie_id = M.idmovie and R.person_id != " + movie_id + " " +
				"ORDER BY RAND() LIMIT 3");
		ArrayList<String> wrong_answers = new ArrayList<String>();
		while (rs.next()) {
			movie_id = rs.getInt(1);
			wrong_answers.add(schema.getMovie(movie_id));
		}
		
		return new QuestionInfo("In which of the following movies did " + person + " play?", 
				right_answer, wrong_answers); 
	}

}
