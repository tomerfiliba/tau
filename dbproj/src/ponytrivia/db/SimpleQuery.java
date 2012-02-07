package ponytrivia.db;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;


public class SimpleQuery extends Prepared {
	public SimpleQuery(PreparedStatement pstmt) throws SQLException {
		super(pstmt);
	}

	public ResultSet query(Object... values) throws SQLException {
		fill(values);
		pstmt.executeQuery();
		return pstmt.getResultSet();
	}
	
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

	public int[] queryGetIntsSingleRow(int cols, Object... values) throws SQLException {
		return queryGetInts(1, cols, values)[0];
	}
	
}
