package ponytrivia.gui;

import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.wb.swt.SWTResourceManager;

public class Highscores {

	protected Shell shlHighscores;
	private Table table;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			Highscores window = new Highscores();
			window.open();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	/**
	 * Open the window.
	 */
	public void open() {
		Display display = Display.getDefault();
		createContents();
		shlHighscores.open();
		shlHighscores.layout();
		while (!shlHighscores.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}
	}

	/**
	 * Create contents of the window.
	 */
	protected void createContents() {
		shlHighscores = new Shell();
		shlHighscores.setSize(261, 378);
		shlHighscores.setText("Highscores");
		
		Label lblGameHighscores = new Label(shlHighscores, SWT.NONE);
		lblGameHighscores.setBounds(10, 10, 120, 18);
		lblGameHighscores.setText("Game Highscores");
		
		table = new Table(shlHighscores, SWT.FULL_SELECTION);
		table.setBounds(10, 75, 224, 248);
		table.setHeaderVisible(true);
		table.setLinesVisible(true);
		
		TableColumn tblclmnIndex = new TableColumn(table, SWT.NONE);
		tblclmnIndex.setWidth(20);
		
		TableColumn tblclmnUser = new TableColumn(table, SWT.NONE);
		tblclmnUser.setWidth(100);
		tblclmnUser.setText("User");
		
		TableColumn tblclmnScore = new TableColumn(table, SWT.NONE);
		tblclmnScore.setWidth(100);
		tblclmnScore.setText("Score");
		
		TableItem tableItem = new TableItem(table, SWT.NONE);
		tableItem.setText(0, "1");
		tableItem.setText(1, "Moshe");
		tableItem.setText(2, "283");
		
		Label label = new Label(shlHighscores, SWT.NONE);
		label.setImage(SWTResourceManager.getImage(Highscores.class, "/ponytrivia/gui/res/grail.gif"));
		label.setBounds(189, 10, 45, 59);
		
		//table.

	}
}
