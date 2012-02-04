package ponytrivia.db.exceptions;

import java.sql.SQLException;

public class MovieNotFound extends SQLException {
	private static final long serialVersionUID = 1812344649856888937L;
	
	public MovieNotFound(String msg) {
		super(msg);
	}
}
