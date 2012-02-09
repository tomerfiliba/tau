package ponytrivia.db;

import java.sql.PreparedStatement;
import java.sql.SQLException;

/**
 * represents a batch of operations that "flushes" (calls executeBatch) at a given interval
 */
public class Batch extends Prepared {
	protected int threshold;
	protected int count;
	
	/**
	 * @param pstmt - the prepared statement to use
	 * @param threshold - the threshold, e.g., how many add()s before automatically flushing
	 */
	public Batch(PreparedStatement pstmt, int threshold) {
		super(pstmt);
		this.threshold = threshold;
		this.count = 0;
		//setAutoCommit(false);
	}
	
	/**
	 * closes the batch and flushes everything that remains 
	 */
	@Override
	public void close() throws SQLException
	{
		if (pstmt != null) {
			flush();
		}
		super.close();
	}

	/**
	 * flushes the batch to the DB
	 * @throws SQLException
	 */
	public void flush() throws SQLException {
		if (count <= 0) {
			return;
		}
		int res[] = pstmt.executeBatch();
		for (int i = 0; i < res.length; i++) {
			if (res[i] == PreparedStatement.EXECUTE_FAILED) {
				//conn.rollback();
				throw new SQLException("Batch operation " + i
						+ " has failed with code " + res[i]);
			}
		}
		//pstmt.getConnection().commit();
		count = 0;
	}
	
	/**
	 * fills in the parameters and adds prepared statement to the batch 
	 * @param args - any arguments for the prepared statement
	 * @throws SQLException
	 */
	public void add(Object... args) throws SQLException {
		fill(args);
		pstmt.addBatch();
		count++;
		if (count > threshold) {
			flush();
		}
	}
	
	//public void add(String sql) throws SQLException {
	//	pstmt.addBatch(sql);
	//	count++;
	//	if (count > threshold) {
	//		flush();
	//	}
	//}

}









