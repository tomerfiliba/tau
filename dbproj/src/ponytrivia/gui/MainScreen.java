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
		shlPonyTrivia.setSize(386, 331);
		shlPonyTrivia.setText("Pony Trivia");
		FillLayout fl_shlPonyTrivia = new FillLayout(SWT.VERTICAL);
		fl_shlPonyTrivia.spacing = 10;
		fl_shlPonyTrivia.marginHeight = 5;
		fl_shlPonyTrivia.marginWidth = 5;
		shlPonyTrivia.setLayout(fl_shlPonyTrivia);
		
		Button btnPlayGame = new Button(shlPonyTrivia, SWT.NONE);
		btnPlayGame.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
			}
		});
		btnPlayGame.setImage(SWTResourceManager.getImage(MainScreen.class, "/pony/gui/kitty.gif"));
		btnPlayGame.setText("Play Game");
		
		Button btnImportImdbFiles = new Button(shlPonyTrivia, SWT.NONE);
		btnImportImdbFiles.setImage(SWTResourceManager.getImage(MainScreen.class, "/pony/gui/IMDB-logo.gif"));
		btnImportImdbFiles.setText("Import IMDB Files");
		
		Button btnEditDb = new Button(shlPonyTrivia, SWT.NONE);
		btnEditDb.setImage(SWTResourceManager.getImage(MainScreen.class, "/pony/gui/mysql-logo.gif"));
		btnEditDb.setText("Edit DB");

	}
}
