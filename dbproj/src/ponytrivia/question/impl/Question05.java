package ponytrivia.question.impl;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Random;

import ponytrivia.db.NoResultsFound;
import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * In which of the following movies, the director is also an actor?
 * Choose 3 movies in which the director did not participate
 */
public class Question05 extends QuestionGenerator {
	Random rand = new Random();
	
	public Question05(Schema schema) {
		super(schema);
	}

	private SimpleQuery chooseDirActor = null;
	private SimpleQuery chooseDirNotActor = null;
	private SimpleQuery maxMovieID = null;
	
	@Override
	public QuestionInfo generate() throws SQLException {
		if (maxMovieID == null) {
			maxMovieID = schema.createQuery("MAX(FM.filmov_id)", "FilteredMovies as FM", "true");
		}
		if (chooseDirActor == null) {
			chooseDirActor = schema.createQuery("FM.movie_id", "FilteredMovies as FM, MovieDirectors as D", 
					"FM.filmov_id > ? AND FM.movie_id = D.movie AND D.director IN " +
							"(SELECT R.actor FROM Roles AS R WHERE R.movie = FM.movie_id)", 1);
		}
		if (chooseDirNotActor == null) {
			chooseDirNotActor = schema.createQuery("FM.movie_id", "FilteredMovies as FM, MovieDirectors as D",
					"FM.filmov_id > ? AND FM.movie_id = D.movie AND D.director NOT IN " +
							"(SELECT R.actor FROM Roles AS R WHERE R.movie = FM.movie_id)", 3);
		}
		
		int movie_id = -1;
		int begin_at = 0;
		for (int i = 0; i < 10; i++) {
			begin_at = Math.abs(rand.nextInt()) % maxMovieID.queryGetKey();
			System.out.println("begin_at = " + begin_at);
			try {
				movie_id = chooseDirActor.queryGetKey(begin_at);
			} catch (NoResultsFound ex) {
				continue;
			}
			break;
		}
		if (movie_id < 0) {
			throw new SQLException("failed too many timed");
		}

		String movie_name = schema.getMovieName(movie_id);
		int[][] wrongs = chooseDirNotActor.queryGetInts(3, 1, begin_at);
		ArrayList<String> wrongAnswers = new ArrayList<String>();
		
		for (int[] r : wrongs) {
			wrongAnswers.add(schema.getMovieName(r[0]));
		}

		return new QuestionInfo("In which of the following movies, did the director also play?", 
				movie_name, wrongAnswers); 
	}

}
