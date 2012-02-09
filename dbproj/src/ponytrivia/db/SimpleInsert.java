package ponytrivia.db;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;


/**
 * a simplified insert abstraction
 */
public class SimpleInsert extends Prepared
{
	public SimpleInsert(PreparedStatement pstmt) throws SQLException {
		super(pstmt);
	}

	/**
	 * insert the prepared statement with the given parameters
	 * @param values - the parameters to the prepared statement
	 * @return the auto-generated key of the created row, or -1 if it cannot be inferred
	 * @throws SQLException
	 */
	public int insert(Object... values) throws SQLException {
		fill(values);
		pstmt.executeUpdate();
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
