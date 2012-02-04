package ponytrivia.db;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.SQLException;


public class Batch extends Prepared {
	protected Connection conn;
	protected int threshold;
	protected int count;
	
	public Batch(Connection conn, PreparedStatement pstmt, int threshold) {
		super(pstmt);
		this.threshold = threshold;
		this.count = 0;
	}
	
	@Override
	public void close() throws SQLException
	{
		if (pstmt != null) {
			flush();
		}
		super.close();
	}

	protected void flush() throws SQLException {
		if (count <= 0) {
			return;
		}
		int res[] = pstmt.executeBatch();
		for (int i = 0; i < res.length; i++) {
			if (res[i] != 0) {
				conn.rollback();
				throw new SQLException("Batch operation " + i
						+ " has failed with code " + res[i]);
			}
		}
		conn.commit();
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









