package ponytrivia.question.impl;

import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following movies was produced in <year X>?
 * Choose 3 movies not produced in year X
 */
public class Question10 extends QuestionGenerator {
	public Question10(Schema schema) {
		super(schema);
	}

	private SimpleQuery chooseYear = null;
	private SimpleQuery chooseNotYear = null;

	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseYear == null) {
			chooseYear = schema.createQuery("FM.movie_id, M.year", "FilteredMovies as FM, Movies as M",
					"FM.movie_id = M.movie_id", "rand()", 1);
			
		}
		if (chooseNotYear == null) {
			chooseNotYear = schema.createQuery("FM.movie_id", "FilteredMovies as FM, Movies as M",
					"FM.movie_id = M.movie_id AND M.year != ? AND M.year BETWEEN ? AND ?", 
					"rand()", 3);
		}
		
		int[] res = chooseYear.queryGetIntsSingleRow(2);
		int movie_id = res[0];
		String movie_name = schema.getMovieName(movie_id);
		int year = res[1];
		
		int[][] wrongs = chooseNotYear.queryGetInts(3, 1, year, year - 3, year + 3);
		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int i = 0; i < 3; i++) {
			wrongAnswers.add(schema.getMovieName(wrongs[i][0]));
		}

		return new QuestionInfo("Which of the following movies was produced in " + year + "?",
				movie_name, wrongAnswers);
	}
}
