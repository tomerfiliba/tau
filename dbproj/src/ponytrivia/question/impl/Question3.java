package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * What year was <movie X> produced?
 */
public class Question3 extends QuestionGenerator {
	protected Random rand = new Random();

	public Question3(Schema schema) {
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
	
	@Override
	public QuestionInfo generate() throws SQLException {
		ResultSet rs = schema.executeQuery("select M.idmovie, M.year from filtered_movie as M " +
			"WHERE M.year is not null and M.year > 1900 " +
			"ORDER BY RAND() LIMIT 1");

		rs.next();
		int movie_id =  rs.getInt(1);
		Integer year = rs.getInt(2);
		
		String movie_name = schema.getMovie(movie_id);
		ArrayList<String> wrong_answers = new ArrayList<String>();
		for (Integer r : getRands(year, -6, 6)) {
			wrong_answers.add(r.toString());
		}
		
		return new QuestionInfo("What year was " + movie_name + " released?", 
				year.toString(), wrong_answers); 
	}

}
