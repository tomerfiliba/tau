package ponytrivia.db;

import java.sql.PreparedStatement;
import java.sql.SQLException;


/**
 * a simple abstraction for database updates
 */
public class SimpleUpdate extends Prepared
{
	public SimpleUpdate(PreparedStatement pstmt) throws SQLException {
		super(pstmt);
	}

	/**
	 * sends the update to the DB
	 * @param values - the parameters for the prepared statement
	 * @throws SQLException
	 */
	public void update(Object... values) throws SQLException {
		fill(values);
		pstmt.executeUpdate();
	}
}
