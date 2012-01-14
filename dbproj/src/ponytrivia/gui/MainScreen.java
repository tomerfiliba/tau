package ponytrivia.gui;

import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.wb.swt.SWTResourceManager;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.custom.StackLayout;
import org.eclipse.swt.widgets.Composite;

public class MainScreen {

	protected Shell shlPonyTrivia;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			MainScreen window = new MainScreen();
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
		shlPonyTrivia.open();
		shlPonyTrivia.layout();
		while (!shlPonyTrivia.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}
	}

	/**
	 * Create contents of the window.
	 */
	protected void createContents() {
		shlPonyTrivia = new Shell();
		shlPonyTrivia.setImage(null);
		shlPonyTrivia.setSize(460, 240);
		shlPonyTrivia.setText("Pony Trivia");
		/*btnPlayGame.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				shlPonyTrivia.close();
				GameScreen gs = new GameScreen();
				gs.open();
			}
		});*/
		shlPonyTrivia.setLayout(new FormLayout());
		
		Button btnPlayGame = new Button(shlPonyTrivia, SWT.NONE);
		FormData fd_btnPlayGame = new FormData();
		fd_btnPlayGame.top = new FormAttachment(0, 10);
		fd_btnPlayGame.left = new FormAttachment(0, 10);
		btnPlayGame.setLayoutData(fd_btnPlayGame);
		btnPlayGame.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/kitty.gif"));
		btnPlayGame.setText("Play Game");
		
		Button btnImportImdbFiles = new Button(shlPonyTrivia, SWT.NONE);
		FormData fd_btnImportImdbFiles = new FormData();
		fd_btnImportImdbFiles.top = new FormAttachment(btnPlayGame, 7);
		fd_btnImportImdbFiles.left = new FormAttachment(btnPlayGame, 0, SWT.LEFT);
		btnImportImdbFiles.setLayoutData(fd_btnImportImdbFiles);
		btnImportImdbFiles.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/IMDB-logo.gif"));
		btnImportImdbFiles.setText("Import IMDB Files");
		
		Button btnEditDb = new Button(shlPonyTrivia, SWT.NONE);
		fd_btnImportImdbFiles.bottom = new FormAttachment(btnEditDb, 0, SWT.BOTTOM);
		fd_btnPlayGame.bottom = new FormAttachment(btnEditDb, -6);
		fd_btnPlayGame.right = new FormAttachment(btnEditDb, 0, SWT.RIGHT);
		FormData fd_btnEditDb = new FormData();
		fd_btnEditDb.left = new FormAttachment(0, 255);
		fd_btnEditDb.top = new FormAttachment(0, 112);
		btnEditDb.setLayoutData(fd_btnEditDb);
		btnEditDb.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/mysql-logo.gif"));
		btnEditDb.setText("Edit DB");

	}
}
