package ponytrivia.question;

import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Random;
import java.util.Set;
import java.util.concurrent.ArrayBlockingQueue;

import ponytrivia.db.Schema;
import ponytrivia.question.impl.Question01;
import ponytrivia.question.impl.Question02;
import ponytrivia.question.impl.Question03;
import ponytrivia.question.impl.Question04;
import ponytrivia.question.impl.Question05;
import ponytrivia.question.impl.Question06;
import ponytrivia.question.impl.Question07;
import ponytrivia.question.impl.Question08;
import ponytrivia.question.impl.Question09;
import ponytrivia.question.impl.Question10;


public class QuestionRegistry {
	protected Random rand;
	protected List<QuestionGenerator> registry;
	protected Set<String> questionHistory;
	protected Schema schema;
	protected ArrayBlockingQueue<QuestionInfo> questionsQueue;
	protected boolean disposed;
	
	public QuestionRegistry(Schema schema) throws SQLException {
		registry = new ArrayList<QuestionGenerator>();
		rand = new Random();
		questionHistory = new HashSet<String>();
		questionsQueue = new ArrayBlockingQueue<QuestionInfo>(10); 
		this.schema = schema;
		disposed = false;
		
		registry.add(new Question01(schema));
		registry.add(new Question02(schema));
		registry.add(new Question03(schema));
		registry.add(new Question04(schema));
		registry.add(new Question05(schema));
		registry.add(new Question06(schema));
		registry.add(new Question07(schema));
		registry.add(new Question08(schema));
		registry.add(new Question09(schema));
		registry.add(new Question10(schema));
		
		System.out.println("clearFilter begin");
		clearFilter();
		System.out.println("clearFilter done");
		qgThread = new QuestionGeneratorThread();
		qgThread.setDaemon(true);
		qgThread.start();
	}
	
	protected class QuestionGeneratorThread extends Thread {
		@Override
		public void run() {
			System.out.println("QuestionGeneratorThread started");
			while (!disposed) {
				QuestionInfo qi;
				try {
					qi = generateQuestion();
				} catch (SQLException e) {
					// just try again
					//e.printStackTrace();
					continue;
					//break;
				}
				if (qi == null || qi.answers.size() != 4) {
					// something went wrong, try again 
					continue;
				}
				try {
					questionsQueue.put(qi);
					System.out.println("Added another question");
				} catch (InterruptedException e) {
					// shouldn't ever happen
					System.out.println("Error adding question");
					break;
				}
			}
        }
	}
	
	protected QuestionGeneratorThread qgThread;
	
	public void close() {
		if (disposed) {
			return;
		}
		disposed = true;
		questionsQueue.clear();
		try {
			qgThread.join();
		} catch (InterruptedException e) {
		}
	}
	
	public QuestionInfo getQuestion() {
		try {
			System.out.println("In getQuestion...");
			return questionsQueue.take();
		} catch (InterruptedException e) {
			return null;
		} finally {
			System.out.println("Outta getQuestion");
		}
	}

	protected QuestionInfo generateQuestion() throws SQLException
	{
		QuestionInfo qi = null;
		int ind = Math.abs(rand.nextInt()) % registry.size();
		QuestionGenerator qg = registry.get(ind);
		qi = qg.generate();
		String unique = qi.questionText + qi.answers.get(qi.correctAnswerIndex);
		if (!questionHistory.contains(unique)) {
			questionHistory.add(unique);
			return qi;
		}
		return null;
	}
	
	/**
	 * clears (or initializes) the FilteredMovies table without any constraints
	 * @return the films of movies in the newly created FilteredMovies table
	 * @throws SQLException
	 */
	public synchronized int clearFilter() throws SQLException {
		return setFilter(-1, -1, null);
	}
	
	/**
	 * sets the given year-range and genre-filter 
	 * @param minYear - the minimum year; use -1 for no lower bound
	 * @param maxYear - the maximum year; use -1 for no upper bound
	 * @param genre_ids - array of genre id's (integers, DB keys); use null for no genre constraint
	 * @return the number of films in the newly created FilteredMovies table
	 * @throws SQLException
	 */
	public synchronized int setFilter(int minYear, int maxYear, int[] genre_ids) throws SQLException {
		Statement stmt = schema.createStatement();
		String genres = "";

		if (minYear < 0) {
			minYear = 0;
		}
		if (maxYear < 0) {
			maxYear = 3000;
		}
		if (genre_ids == null || genre_ids.length == 0) {
			genres = "SELECT genre_id FROM genres";
		}
		else {
			for (int i = 0; i < genre_ids.length - 1; i++) {
				genres = genre_ids[i] + ", ";
			}
			genres += genre_ids[genre_ids.length - 1];
		}

		try {
			// since we can't create temporary views -- we use temporary tables
			stmt.executeUpdate("DROP TABLE IF EXISTS FilteredMovies");
			stmt.executeUpdate("CREATE TEMPORARY TABLE FilteredMovies (filmov_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, " +
					"movie_id INT UNIQUE KEY NOT NULL)");
			stmt.executeUpdate("INSERT INTO FilteredMovies (movie_id) SELECT DISTINCT PM.movie_id " +
					"FROM PopularMovies as PM, Movies as M, MovieGenres as MG WHERE " +
					"PM.movie_id = M.movie_id AND M.year BETWEEN " + minYear + " AND " + maxYear + 
					" AND MG.movie = PM.movie_id AND MG.genre IN (" + genres + ")");
			int count = stmt.getUpdateCount();
			System.out.println("FilteredMovies = " + count);

			stmt.executeUpdate("DROP TABLE IF EXISTS FilteredDirectors");
			stmt.executeUpdate("CREATE TEMPORARY TABLE FilteredDirectors (fildir_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, " +
					"director INT UNIQUE KEY NOT NULL)");
			stmt.executeUpdate("INSERT INTO FilteredDirectors (director) SELECT DISTINCT PD.director " +
					"FROM PopularDirectors AS PD, FilteredMovies as FM, MovieDirectors AS MD " +
					"WHERE PD.director = MD.director AND MD.movie = FM.movie_id");
			count = stmt.getUpdateCount();
			System.out.println("FilteredDirectors = " + count);
			
			stmt.executeUpdate("DROP TABLE IF EXISTS FilteredActors");
			stmt.executeUpdate("CREATE TEMPORARY TABLE FilteredActors (filact_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, " +
					"actor INT UNIQUE KEY NOT NULL)");
			stmt.executeUpdate("INSERT INTO FilteredActors (actor) SELECT DISTINCT PA.actor " +
					"FROM PopularActors AS PA INNER JOIN Roles AS R ON PA.actor = R.actor " +
					"INNER JOIN FilteredMovies AS FM ON R.movie = FM.movie_id");
			count = stmt.getUpdateCount();
			System.out.println("FilteredActors = " + count);
			
			stmt.close();
			questionsQueue.clear();
			questionHistory.clear();
			schema.commit();
			return count;
		} catch (SQLException ex) {
			schema.rollback();
			throw ex;
		}
	}	
	
}

