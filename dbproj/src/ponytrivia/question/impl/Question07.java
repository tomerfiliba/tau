package ponytrivia.question.impl;

import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
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

	private SimpleQuery chooseMost = null;
	private SimpleQuery chooseWrongs = null;

	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseMost == null) {
			chooseMost = schema.createQuery("FM.movie_id", "FilteredMovies AS FM, Movies AS M",
					"FM.movie_id = M.movie_id AND M.rating >= 8.5", "rand()", 1);
		}
		if (chooseWrongs == null) {
			chooseWrongs = schema.createQuery("FM.movie_id", "FilteredMovies AS FM, Movies AS M",
					"FM.movie_id = M.movie_id AND M.rating < 8.5", "rand()", 3);
		}

		int movie_id = chooseMost.queryGetKey();
		String movie_name = schema.getMovieName(movie_id);
		
		int[][] wrongs = chooseWrongs.queryGetInts(3, 1);
		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int i = 0; i < 3; i++) {
			wrongAnswers.add(schema.getMovieName(wrongs[i][0]));
		}

		return new QuestionInfo("Which of the following movies is the most popular on IMDB?",
				movie_name, wrongAnswers);
	}

}
