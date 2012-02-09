package ponytrivia.db;

import java.sql.Date;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Types;


/**
 * a base class for working with prepare statements
 */
public abstract class Prepared {
	protected PreparedStatement pstmt;
	
	protected Prepared(PreparedStatement pstmt) {
		this.pstmt = pstmt;
	}
	
	/**
	 * closes the prepared statement
	 * @throws SQLException
	 */
	public void close() throws SQLException {
		if (pstmt != null) {
			pstmt.close();
			pstmt = null;
		}
	}
	
	@Override
	protected void finalize() throws Throwable {
		close();
	}

	/**
	 * fills in the arguments for the prepare statement
	 * @param args
	 * @throws SQLException
	 */
	protected void fill(Object[] args) throws SQLException {
		int i = 1;
		for (Object arg : args) {
			if (arg == null) {
				pstmt.setNull(i, Types.VARCHAR);
			}
			else if (arg instanceof Date) {
				pstmt.setDate(i, ((Date)arg));
			}
			else if (arg instanceof Integer) {
				pstmt.setInt(i, ((Integer)arg));
			}
			else if (arg instanceof Long) {
				pstmt.setLong(i, ((Long)arg));
			}
			else if (arg instanceof Float) {
				pstmt.setFloat(i, ((Float)arg));
			}
			else if (arg instanceof Double) {
				pstmt.setDouble(i, ((Double)arg));
			}
			else if (arg instanceof Boolean) {
				pstmt.setBoolean(i, ((Boolean)arg));
			}
			else if (arg instanceof String) {
				pstmt.setString(i, ((String)arg));
			} else {
				throw new IllegalArgumentException("can't add " + arg);
			}
			i++;
		}		
	}
}
