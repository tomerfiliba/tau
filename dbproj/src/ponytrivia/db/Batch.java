package ponytrivia.db;

import java.sql.PreparedStatement;
import java.sql.SQLException;


public class Batch extends Prepared {
	protected int threshold;
	protected int count;
	
	public Batch(PreparedStatement pstmt, int threshold) {
		super(pstmt);
		this.threshold = threshold;
		this.count = 0;
		setAutoCommit(false);
	}
	
	@Override
	public void close() throws SQLException
	{
		if (pstmt != null) {
			flush();
		}
		super.close();
	}

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
		pstmt.getConnection().commit();
		count = 0;
	}
	
	public void add(Object... args) throws SQLException {
		fill(args);
		pstmt.addBatch();
		count++;
		if (count > threshold) {
			flush();
		}
	}
	
	public void add(String sql) throws SQLException {
		pstmt.addBatch(sql);
		count++;
		if (count > threshold) {
			flush();
		}
	}

}









