package ponytrivia.question.impl;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionGenerator;
import ponytrivia.question.QuestionInfo;

/*
 * Which of the following actors played in <movie X> and <movie Y>?
 * Choose 3 actors, some of which played in X and some in Y, but not in both
 */
public class Question01 extends QuestionGenerator {
	public Question01(Schema schema) {
		super(schema);
	}
	
	@Override
	public QuestionInfo generate() throws SQLException {
		/*
		 * mid = select M.movie_id from filtered_movie as M order by rand() limit 1
		 * 
		 * pid = select A.person_id from famous_actors as A, roles as R
		 *       where R.movie_id = $$mid$$ and R.person_id = P.person_id
		 *       order by rand() limit 1 
		 * 
		 * mid2 = select M.movie_id from filtered_movie as M, roles as R
		 *        where M.movie_id <> $$mid$$ and R.movie_id = M.movie_id and R.person_id = $$pid$$
		 *        order by rand() limit 1
		 * 
		 * wrong = select A.person_id from famous_actors as A
		 *         where A.person_id not in (select R.person_id from roles as R 
		 *                                   where R.movie_id = $$mid$$ or R.movie_id = $$mid2$$)
		 *         order by rand() limit 3
		 */
		
		ResultSet rs = schema.executeQuery("select P.idperson, M.idmovie from person as P, role as R, filtered_movie as M " +
			"WHERE P.idperson = R.person_id and R.movie_id = M.idmovie " +
			"ORDER BY RAND() LIMIT 1");

		rs.next();
		int person_id = rs.getInt(1);
		int movie_id =  rs.getInt(2);
		
		String right_answer = schema.getMovie(movie_id);
		String person = schema.getPerson(person_id);

		rs = schema.executeQuery("select M.idmovie from filtered_movie as M " +
				"where M.idmovie not in ( " +
				"   select M.idmovie from movie as M, role as R, Person as P " +
				"    where M.idmovie = R.movie_id and R.person_id = " + person_id + ") " +
				"order by rand() " +
				"limit 3");
		ArrayList<String> wrong_answers = new ArrayList<String>();
		while (rs.next()) {
			movie_id = rs.getInt(1);
			wrong_answers.add(schema.getMovie(movie_id));
		}
		
		return new QuestionInfo("In which of the following movies did " + person + " play?", 
				right_answer, wrong_answers); 
	}

}
