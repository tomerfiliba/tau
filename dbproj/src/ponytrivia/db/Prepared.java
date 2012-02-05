package ponytrivia.db;

import java.sql.Date;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Types;

public abstract class Prepared {
	public PreparedStatement pstmt;
	protected boolean autoCommit;
	
	protected Prepared(PreparedStatement pstmt) {
		this.pstmt = pstmt;
		this.autoCommit = false;
	}
	
	public void close() throws SQLException {
		if (pstmt != null) {
			if (autoCommit) {
				pstmt.getConnection().commit();
			}
			pstmt.close();
			pstmt = null;
		}
	}
	
	public void commit() throws SQLException {
		pstmt.getConnection().commit();
	}
	
	public void setAutoCommit(boolean value) {
		autoCommit = value;
	}
	
	@Override
	protected void finalize() throws Throwable {
		close();
	}
	
	protected void fill(Object[] args) throws SQLException {
		fillPrepared(pstmt, args);
	}
	
	protected static void fillPrepared(PreparedStatement pstmt, Object[] args) throws SQLException {
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
