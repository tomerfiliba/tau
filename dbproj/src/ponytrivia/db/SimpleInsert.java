package ponytrivia.db;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;


public class SimpleInsert extends Prepared
{
	public SimpleInsert(PreparedStatement pstmt) throws SQLException {
		super(pstmt);
		setAutoCommit(true);
	}
	
	public int insert(Object... values) throws SQLException {
		fill(values);
		pstmt.executeUpdate();
		//conn.commit();
		ResultSet rs = pstmt.getGeneratedKeys();
		try {
			if (!rs.next()) {
				return -1;
			}
			return rs.getInt(1);
		} finally {
			rs.close();
		}
	}
}
