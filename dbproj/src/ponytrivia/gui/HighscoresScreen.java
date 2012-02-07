package ponytrivia.gui;

import java.sql.ResultSet;
import java.sql.SQLException;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.wb.swt.SWTResourceManager;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleQuery;

public class HighscoresScreen extends Shell {
	private Table table;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void run(Display display, Schema schema) {
		try {
			HighscoresScreen shell = new HighscoresScreen(display, schema);
			shell.open();
			shell.layout();
			while (!shell.isDisposed()) {
				if (!display.readAndDispatch()) {
					display.sleep();
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	protected Schema schema;
	
	/**
	 * Create the shell.
	 * @param display
	 */
	public HighscoresScreen(Display display, Schema schema) {
		super(display, SWT.SHELL_TRIM);
		this.schema = schema;
		setLayout(new FormLayout());
		setText("Highscores Board");
		setSize(450, 400);
		
		Label lblGrail = new Label(this, SWT.NONE);
		lblGrail.setImage(SWTResourceManager.getImage(HighscoresScreen.class, "/ponytrivia/gui/res/grail.gif"));
		FormData fd_lblGrail = new FormData();
		fd_lblGrail.top = new FormAttachment(0, 10);
		fd_lblGrail.right = new FormAttachment(100, -10);
		lblGrail.setLayoutData(fd_lblGrail);
		
		table = new Table(this, SWT.BORDER | SWT.FULL_SELECTION);
		table.setHeaderVisible(true);
		FormData fd_table = new FormData();
		fd_table.bottom = new FormAttachment(100, -10);
		fd_table.right = new FormAttachment(lblGrail, -6);
		fd_table.top = new FormAttachment(lblGrail, 0, SWT.TOP);
		fd_table.left = new FormAttachment(0, 10);
		table.setLayoutData(fd_table);
		
		TableColumn tblclmnNum = new TableColumn(table, SWT.NONE);
		tblclmnNum.setWidth(37);
		tblclmnNum.setText("#");
		
		TableColumn tblclmnName = new TableColumn(table, SWT.NONE);
		tblclmnName.setWidth(100);
		tblclmnName.setText("Name");
		
		TableColumn tblclmnScore = new TableColumn(table, SWT.NONE);
		tblclmnScore.setWidth(100);
		tblclmnScore.setText("Score");
		
		try {
			createContents();
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	/**
	 * Create contents of the shell.
	 * @throws SQLException 
	 */
	protected void createContents() throws SQLException {
		ResultSet rs = null;
		SimpleQuery q = null;
		
		schema.commit(); // this will refresh the data (otherwise we get stale data)
		
		try {
			q = schema.createQuery("GP.username, HS.score", "GamePlayers AS GP, Highscores AS HS",
					"GP.user_id = HS.user", "HS.score DESC", 10);
			rs = q.query();
			Integer i = 1;
			while (rs.next()) {
				TableItem tableItem = new TableItem(table, SWT.NONE);
				tableItem.setText(0, i.toString());
				tableItem.setText(1, rs.getString(1));
				tableItem.setText(2, new Integer(rs.getInt(2)).toString());
				i++;
			}
		}
		finally {
			if (rs != null) {
				rs.close();
			}
			if (q != null) {
				q.close();
			}
		}
	}

	@Override
	protected void checkSubclass() {
		// Disable the check that prevents subclassing of SWT components
	}

}
