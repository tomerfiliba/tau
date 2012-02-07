package ponytrivia.question.impl;

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
	private SimpleQuery chooseWrong = null;
	
	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseActor == null) {
			chooseActor = schema.createQuery("P.person_id, M.movie_id", 
				"People as P, Roles as R, FilteredMovies as M", 
				"P.person_id = R.actor and R.movie = M.movie_id", "rand()", 1);
		}
		if (chooseWrong == null) {
			chooseWrong = schema.createQuery("FM.movie_id", "FilteredMovies as FM", 
					"FM.movie_id NOT IN (SELECT M.movie_id FROM Movies AS M, Roles AS R, People as P " +
							"WHERE M.movie_id = R.movie AND R.actor = ?)", 
					"RAND()", 3);
		}
		
		int res[] = chooseActor.queryGetIntsSingleRow(2);
		int person_id = res[0];
		int movie_id = res[1];
		String person_name = schema.getPersonName(person_id);
		String right_answer = schema.getMovieName(movie_id);

		ArrayList<String> wrong_answers = new ArrayList<String>();

		int wrongs[][] = chooseWrong.queryGetInts(3, 1, person_id);
		for (int[] mid : wrongs) {
			wrong_answers.add(schema.getMovieName(mid[0]));
		}
		
		return new QuestionInfo("In which of the following movies did " + person_name + " play?", 
				right_answer, wrong_answers); 
	}

}
