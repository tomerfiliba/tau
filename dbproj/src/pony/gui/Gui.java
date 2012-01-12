package pony.gui;

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

public class Gui {

	protected Shell shlPonyTrivia;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			Gui window = new Gui();
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
		shlPonyTrivia.setSize(647, 373);
		shlPonyTrivia.setText("Pony Trivia");
		shlPonyTrivia.setLayout(new GridLayout(1, false));
		new Label(shlPonyTrivia, SWT.NONE);
		
		Button btnPlayGame = new Button(shlPonyTrivia, SWT.NONE);
		btnPlayGame.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
			}
		});
		GridData gd_btnPlayGame = new GridData(SWT.LEFT, SWT.CENTER, false, false, 1, 1);
		gd_btnPlayGame.heightHint = 94;
		gd_btnPlayGame.widthHint = 248;
		btnPlayGame.setLayoutData(gd_btnPlayGame);
		btnPlayGame.setImage(SWTResourceManager.getImage(Gui.class, "/pony/gui/kitty.gif"));
		btnPlayGame.setText("Play Game");
		
		Button btnImportImdbFiles = new Button(shlPonyTrivia, SWT.NONE);
		GridData gd_btnImportImdbFiles = new GridData(SWT.LEFT, SWT.CENTER, false, false, 1, 1);
		gd_btnImportImdbFiles.heightHint = 71;
		gd_btnImportImdbFiles.widthHint = 247;
		btnImportImdbFiles.setLayoutData(gd_btnImportImdbFiles);
		btnImportImdbFiles.setImage(SWTResourceManager.getImage(Gui.class, "/pony/gui/IMDB-logo.gif"));
		btnImportImdbFiles.setText("Import IMDB Files");
		
		Button btnEditDb = new Button(shlPonyTrivia, SWT.NONE);
		GridData gd_btnEditDb = new GridData(SWT.LEFT, SWT.CENTER, false, false, 1, 1);
		gd_btnEditDb.widthHint = 248;
		btnEditDb.setLayoutData(gd_btnEditDb);
		btnEditDb.setImage(SWTResourceManager.getImage(Gui.class, "/pony/gui/mysql-logo.gif"));
		btnEditDb.setText("Edit DB");

	}
}
