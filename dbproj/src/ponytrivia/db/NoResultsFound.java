package ponytrivia.db;

import java.sql.SQLException;

public class NoResultsFound extends SQLException {
	private static final long serialVersionUID = -1908977602846889049L;

	public NoResultsFound(String msg) {
		super(msg);
	}
}
