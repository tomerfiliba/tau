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
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.Combo;

public class MainScreen {

	protected Shell shlPonyTrivia;
	private Text text;

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
		shlPonyTrivia.setSize(623, 269);
		shlPonyTrivia.setText("Pony Trivia");
		shlPonyTrivia.setLayout(null);
		
		Button btnImportImdbFiles = new Button(shlPonyTrivia, SWT.NONE);
		btnImportImdbFiles.setBounds(10, 150, 239, 66);
		btnImportImdbFiles.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/IMDB-logo.gif"));
		btnImportImdbFiles.setText("Import IMDB Files");
		
		Button btnEditDb = new Button(shlPonyTrivia, SWT.NONE);
		btnEditDb.setBounds(255, 150, 189, 67);
		btnEditDb.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/mysql-logo.gif"));
		btnEditDb.setText("Edit DB");
		
		Group group = new Group(shlPonyTrivia, SWT.NONE);
		group.setBounds(10, 10, 582, 134);
		
		Button btnPlayGame = new Button(group, SWT.NONE);
		btnPlayGame.setBounds(321, 20, 251, 96);
		btnPlayGame.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/kitty1.gif"));
		btnPlayGame.setText("Play Game");
		
		text = new Text(group, SWT.BORDER);
		text.setBounds(110, 20, 126, 24);
		
		Label lblNewLabel = new Label(group, SWT.NONE);
		lblNewLabel.setBounds(10, 23, 94, 18);
		lblNewLabel.setText("Username:");
		
		Label lblGenre = new Label(group, SWT.NONE);
		lblGenre.setText("Genre:");
		lblGenre.setBounds(10, 55, 94, 18);
		
		Combo combo = new Combo(group, SWT.NONE);
		combo.setBounds(110, 55, 126, 26);
		
		Label lblDecade = new Label(group, SWT.NONE);
		lblDecade.setText("Decade:");
		lblDecade.setBounds(10, 90, 94, 18);
		
		Combo combo_1 = new Combo(group, SWT.NONE);
		combo_1.setItems(new String[] {"1950's", "1960's", "1970's", "1980's", "1990's", "2000's"});
		combo_1.setBounds(110, 87, 126, 26);
		combo_1.select(5);
		
		Button btnHighscores = new Button(shlPonyTrivia, SWT.NONE);
		btnHighscores.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				shlPonyTrivia.setEnabled(false);
				Highscores hs = new Highscores();
				hs.open();
				
			}
		});
		btnHighscores.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/grail.gif"));
		btnHighscores.setBounds(450, 150, 142, 67);
		btnHighscores.setText("Highscores");

	}
}
