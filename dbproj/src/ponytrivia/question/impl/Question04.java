package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Who of the following is the director of <movie X>?
 * Choose 3 directors of movies from the same year as X
 */
public class Question04 extends QuestionGenerator {
	public Question04(Schema schema) {
		super(schema);
	}

	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema.executeQuery("select M.idmovie from filtered_movie as M " +
			"ORDER BY RAND() LIMIT 1");

		rs.next();
		int movie_id =  rs.getInt(1);
		String movie_name = schema.getMovie(movie_id);
		rs.close();
		
		rs = schema.executeQuery("select D.director from Movies_directors as D " +
				"WHERE D.movie = " + movie_id + ' ' +
				"ORDER BY RAND() LIMIT 1");
		rs.next();
		int director_id = rs.getInt(1);
		rs.close();
		
		rs = schema.executeQuery("SELECT PD.director from Filtered_directors as PD " +
				"WHERE PD.director NOT IN (SELECT D.director FROM Movies_directors as D " +
				"                          WHERE D.movie = " + movie_id + ") " +
				"ORDER BY RAND() LIMIT 3");
		
		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int i=0; i<3; i++){
			rs.next();
			int pid = rs.getInt(1);
			wrongAnswers.add(schema.getPerson(pid));
		}
		
		return new QuestionInfo("Who is the director of " + movie_name + "?", 
				schema.getPerson(director_id), wrongAnswers); 
	}

}
