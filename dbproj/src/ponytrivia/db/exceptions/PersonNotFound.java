package ponytrivia.db.exceptions;

import java.sql.SQLException;

public class PersonNotFound extends SQLException {
	private static final long serialVersionUID = -1908977602846889049L;

	public PersonNotFound(String msg) {
		super(msg);
	}
}
