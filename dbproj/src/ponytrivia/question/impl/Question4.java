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
 * Who of the following is the director of <movie X>?
 * Choose 3 directors of movies from the same year as X
 */
/*public class Question4 extends QuestionGenerator {
	protected Random rand = new Random();

	public Question4(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema.executeQuery("select M.idmovie, M.year from filtered_movie as M " +
			"WHERE M.year is not null" +
			"ORDER BY RAND() LIMIT 1");

		rs.next();
		Integer year = rs.getInt(1);
		int movie_id =  rs.getInt(2);
		
		rs = schema.executeQuery("select M.idmovie, M.year from filtered_movie as M " +
				"WHERE M.year != null");


		rs = schema.executeQuery("select M.idmovie, M.year from filtered_movie as M " +
				"WHERE M.year != null" +
				"ORDER BY RAND() LIMIT 1");

		String movie_name = schema.getMovie(movie_id);
		rs = schema.executeQuery("select M.idmovie from filtered_movie as M " +
				"where M.idmovie not in ( " +
				"   select M.idmovie from movie as M, director as D, Person as P " +
				"    where M.idmovie = D.movie_id and D.person_id = " + person_id + ") " +
				"order by rand() " +
				"limit 3");
		ArrayList<String> wrong_answers = new ArrayList<String>();
		while (rs.next()) {
			movie_id = rs.getInt(1);
			wrong_answers.add(schema.getMovie(movie_id));
		}		
		return new QuestionInfo("Who is the director of " + movie_name + "?", 
				year.toString(), wrong_answers); 
	}

}*/
