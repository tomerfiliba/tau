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


/**
 * implements a question register: uses the schema, possibly sets a filter on which movies,
 * actors and genres are to be fetched, and produces up to 10 questions in the background
 * (using a thread). 
 * 
 * use getQuestion() to pop a question from the pool, call close() when done
 *
 */
public class QuestionRegistry {
	protected Random rand;
	protected List<QuestionGenerator> registry;
	protected Set<String> questionHistory;
	protected Schema schema;
	protected ArrayBlockingQueue<QuestionInfo> questionsQueue;
	protected boolean disposed;
	
	public int numOfFilteredMovies = -1;
	public int numOfFilteredDirectors = -1;
	public int numOfFilteredActors = -1;
	
	public QuestionRegistry(Schema schema, boolean autoClear) throws SQLException {
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
		
		if (autoClear) {
			System.out.println("clearFilter begin");
			clearFilter();
			System.out.println("clearFilter done");
		}
		qgThread = new QuestionGeneratorThread();
		qgThread.setDaemon(true);
	}
	
	public void startBgThread() {
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
					//System.out.println("Added another question");
				} catch (InterruptedException e) {
					break;
				}
			}
        }
	}
	
	protected QuestionGeneratorThread qgThread;
	
	/**
	 * closes the background thread and waits for it to terminate
	 */
	@SuppressWarnings("deprecation")
	public void close() {
		if (disposed) {
			return;
		}
		disposed = true;
		questionsQueue.clear();
		//qgThread.interrupt();
		qgThread.stop();
		/*try {
			qgThread.join();
		} catch (InterruptedException e) {
		}*/
	}
	
	/**
	 * returns the next question from the pool. note that this might block if the pool is empty
	 * @return QuestionInfo object
	 */
	public QuestionInfo getQuestion() {
		try {
			return questionsQueue.take();
		} catch (InterruptedException e) {
			return null;
		}
	}

	/**
	 * called by the background thread to generate questions
	 * @return a QuestionInfo object
	 * @throws SQLException
	 */
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
	public synchronized void clearFilter() throws SQLException {
		setFilter(-1, -1, null);
	}
	
	/**
	 * sets the given year-range and genre-filter 
	 * @param minYear - the minimum year; use -1 for no lower bound
	 * @param maxYear - the maximum year; use -1 for no upper bound
	 * @param genre_ids - array of genre id's (integers, DB keys); use null for no genre constraint
	 * @return the number of films in the newly created FilteredMovies table
	 * @throws SQLException
	 */
	public synchronized void setFilter(int minYear, int maxYear, List<Integer> genre_ids) throws SQLException {
		Statement stmt = schema.createStatement();
		String genres = "";

		if (minYear < 0) {
			minYear = 0;
		}
		if (maxYear < 0) {
			maxYear = 3000;
		}
		if (genre_ids == null || genre_ids.size() == 0) {
			genres = "SELECT genre_id FROM Genres";
		}
		else {
			for (int i = 0; i < genre_ids.size() - 1; i++) {
				genres += genre_ids.get(i) + ", ";
			}
			genres += genre_ids.get(genre_ids.size() - 1);
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
			numOfFilteredMovies = stmt.getUpdateCount();
			System.out.println("FilteredMovies = " + numOfFilteredMovies);

			stmt.executeUpdate("DROP TABLE IF EXISTS FilteredDirectors");
			stmt.executeUpdate("CREATE TEMPORARY TABLE FilteredDirectors (fildir_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, " +
					"director INT UNIQUE KEY NOT NULL)");
			stmt.executeUpdate("INSERT INTO FilteredDirectors (director) SELECT DISTINCT PD.director " +
					"FROM PopularDirectors AS PD, FilteredMovies as FM, MovieDirectors AS MD " +
					"WHERE PD.director = MD.director AND MD.movie = FM.movie_id");
			numOfFilteredDirectors = stmt.getUpdateCount();
			System.out.println("FilteredDirectors = " + numOfFilteredDirectors);
			
			stmt.executeUpdate("DROP TABLE IF EXISTS FilteredActors");
			stmt.executeUpdate("CREATE TEMPORARY TABLE FilteredActors (filact_id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, " +
					"actor INT UNIQUE KEY NOT NULL)");
			stmt.executeUpdate("INSERT INTO FilteredActors (actor) SELECT DISTINCT PA.actor " +
					"FROM PopularActors AS PA INNER JOIN Roles AS R ON PA.actor = R.actor " +
					"INNER JOIN FilteredMovies AS FM ON R.movie = FM.movie_id");
			numOfFilteredActors = stmt.getUpdateCount();
			System.out.println("FilteredActors = " + numOfFilteredActors);
			
			stmt.close();
			questionsQueue.clear();
			questionHistory.clear();
			schema.commit();
		} catch (SQLException ex) {
			schema.rollback();
			throw ex;
		}
	}	
	
}

