package ponytrivia.db;

import java.sql.PreparedStatement;
import java.sql.SQLException;


public class SimpleUpdate extends Prepared
{
	public SimpleUpdate(PreparedStatement pstmt) throws SQLException {
		super(pstmt);
	}
	
	public void update(Object... values) throws SQLException {
		fill(values);
		pstmt.executeUpdate();
		//conn.commit();
	}
}
