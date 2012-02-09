package ponytrivia.db;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;


/**
 * a simplified abstraction over queries
 */
public class SimpleQuery extends Prepared {
	public SimpleQuery(PreparedStatement pstmt) throws SQLException {
		super(pstmt);
	}
	
	/**
	 * submit the given query with the given parameters
	 * @param values - the parameters for the prepared statement
	 * @return a ResultSet object, which must be close()d when finished with
	 * @throws SQLException
	 */
	public ResultSet query(Object... values) throws SQLException {
		fill(values);
		pstmt.executeQuery();
		return pstmt.getResultSet();
	}
	
	/**
	 * a simplified query, returning the first column of the first row (useful for getting back
	 * keys from the DB)
	 * @param values - the parameters for the prepared statement
	 * @return an integer (the key)
	 * @throws SQLException, NoResultsFound - if no results were found
	 */
	public int queryGetKey(Object... values) throws SQLException {
		ResultSet rs = query(values);
		try {
			if (!rs.next()) {
				throw new NoResultsFound("Empty ResultSet " + pstmt);
			}
			return rs.getInt(1);
		} finally {
			rs.close();
		}
	}
	
	/**
	 * returns a two-dimensional array of ints, the results of the query
	 * @param rows - the number of expected rows
	 * @param cols - the number of columns of this query
	 * @param values - the parameters for the prepared statement
	 * @return a two-dimensional array of ints with the results of the query
	 * @throws SQLException, NoResultsFound - if less than `rows` are returned
	 */
	public int[][] queryGetInts(int rows, int cols, Object... values) throws SQLException {
		ResultSet rs = query(values);
		int[][] results = new int[rows][cols];
		
		try {
			for (int i = 0; i < rows; i++) {
				if (!rs.next()) {
					throw new NoResultsFound("Not enough rows: " + pstmt);
				}
				for (int j = 0; j < cols; j++) {
					results[i][j] = rs.getInt(1 + j);
				}
			}
			return results;
		} finally {
			rs.close();
		}
	}

	/**
	 * like queryGetInts, but returns only the first row
	 * @param cols - the number of columns of this query
	 * @param values - the parameters for the prepared statement
	 * @return an array of ints with the results of the query
	 * @throws SQLException, NoResultsFound - if no rows are returned
	 */
	public int[] queryGetIntsSingleRow(int cols, Object... values) throws SQLException {
		return queryGetInts(1, cols, values)[0];
	}
	
}
