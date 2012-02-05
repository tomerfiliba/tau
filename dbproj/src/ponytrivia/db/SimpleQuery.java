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
}
