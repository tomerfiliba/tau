package ponytrivia.question.impl;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * What year was <movie X> produced?
 */
public class Question03 extends QuestionGenerator {
	protected Random rand = new Random();

	public Question03(Schema schema) {
		super(schema);
	}
	
	protected Set<Integer> getRands(int year, int min, int max)
	{
		HashSet<Integer> s = new HashSet<Integer>();
		int currYear = Calendar.getInstance().get(Calendar.YEAR);
		while (s.size() < 3) {
			int r = year + (Math.abs(rand.nextInt()) % (max - min)) + min;
			if (r != year && r <= currYear) {
				s.add(r);
			}
		}
		return s;
	}

	private SimpleQuery chooseMovie = null;

	@Override
	public QuestionInfo generate() throws SQLException {
		if (chooseMovie == null) {
			chooseMovie = schema.createQuery("FM.movie_id, M.year", "FilteredMovies as FM, Movies as M",
				"FM.movie_id = M.movie_id", "rand()", 1);
		}
		int[] res = chooseMovie.queryGetIntsSingleRow(2);
		int movie_id =  res[0];
		Integer year = res[1];
		
		String movie_name = schema.getMovieName(movie_id);
		ArrayList<String> wrong_answers = new ArrayList<String>();
		for (Integer r : getRands(year, -6, 6)) {
			wrong_answers.add(r.toString());
		}
		
		return new QuestionInfo("What year was " + movie_name + " released?", 
				year.toString(), wrong_answers); 
	}

}
