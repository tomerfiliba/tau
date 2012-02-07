package ponytrivia.question.impl;

import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following actors played in <movie X> and <movie Y>?
 * Choose 3 actors which did not play in movie X
 */
public class Question01 extends QuestionGenerator {
	public Question01(Schema schema) {
		super(schema);
	}
	
	private SimpleQuery chooseActor = null;
	private SimpleQuery chooseMovies = null;
	private SimpleQuery chooseWrongs = null;
	
	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseActor == null) {
			chooseActor = schema.createQuery("FA.actor", "FilteredActors AS FA", "true", "rand()", 1);
		}
		if (chooseMovies == null) {
			chooseMovies = schema.createQuery("FM.movie_id", "FilteredMovies AS FM, Roles AS R", 
					"FM.movie_id = R.movie AND R.actor = ?", "rand()", 2);
		}
		if (chooseWrongs == null) {
			chooseWrongs = schema.createQuery("FA.actor", "FilteredActors AS FA", 
					"FA.actor NOT IN (SELECT DISTINCT R.actor FROM Roles AS R " +
							"WHERE R.movie = ?)", 
					"rand()", 3);
		}
		
		int person_id = chooseActor.queryGetKey();
		String actor = schema.getPersonName(person_id);
		
		int movies[][] = chooseMovies.queryGetInts(2, 1, person_id);
		String movie1 = schema.getMovieName(movies[0][0]);
		String movie2 = schema.getMovieName(movies[1][0]);

		int wrongs[][] = chooseWrongs.queryGetInts(3, 1, movies[0][0]);
		ArrayList<String> wrongAnswers = new ArrayList<String>();
		for (int[] r : wrongs) {
			wrongAnswers.add(schema.getPersonName(r[0]));
		}

		return new QuestionInfo("Which of the following actors played in " + movie1 + " and " + movie2 + "?",
				actor, wrongAnswers);
	}

}
