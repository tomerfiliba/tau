package ponytrivia.question.impl;

import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
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

	private SimpleQuery chooseMovie = null;
	private SimpleQuery chooseWrongs = null;

	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseMovie == null) {
			chooseMovie = schema.createQuery("FM.movie_id, D.director", "FilteredMovies as FM, " +
					"MovieDirectors as D", "FM.movie_id = D.movie", "rand()", 1);
		}
		if (chooseWrongs == null) {
			chooseWrongs = schema.createQuery("director", "PopularDirectors as PD",
				"PD.director NOT IN (SELECT MD.director FROM MovieDirectors as MD WHERE MD.movie = ?)",
				"RAND()", 3);
		}
		
		int[] res = chooseMovie.queryGetIntsSingleRow(2);
		int movie_id = res[0];
		int director_id = res[1];
		String movie_name = schema.getMovieName(movie_id);
		String director_name = schema.getPersonName(director_id);
		
		int[][] wrongs = chooseWrongs.queryGetInts(3, 1, movie_id);
		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int[] r : wrongs){
			wrongAnswers.add(schema.getPersonName(r[0]));
		}
		
		return new QuestionInfo("Who is the director of " + movie_name + "?", 
				director_name, wrongAnswers); 
	}

}
