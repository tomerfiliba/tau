package ponytrivia.db;

import java.sql.SQLException;
import java.sql.Statement;

public class Batch {
	protected Statement stmt;
	protected int count;
	protected int threshold;

	public Batch(Statement stmt, int threshold) {
		this.stmt = stmt;
		this.threshold = threshold;
		this.count = 0;
	}

	protected void flush() throws SQLException {
		if (count <= 0) {
			return;
		}
		int res[] = stmt.executeBatch();
		for (int i = 0; i < res.length; i++) {
			if (res[i] != 0) {
				throw new SQLException("Batch operation " + i
						+ " has failed with code " + res[i]);
			}
		}
		count = 0;
	}
	
	public void add(String sql) throws SQLException {
		count++;
		stmt.addBatch(sql);
		if (count > threshold) {
			flush();
		}
	}

	public void close() throws SQLException {
		flush();
		stmt.close();
	}
}