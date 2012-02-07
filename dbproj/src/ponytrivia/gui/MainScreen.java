package ponytrivia.gui;

import java.io.IOException;
import java.sql.SQLException;
import java.util.Properties;

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
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.Combo;

import ponytrivia.db.Schema;

public class MainScreen {
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

	protected Display display;
	protected Shell shlMain;
	protected Schema schema; 

	protected void errorMsgbox(String title, String message) {
		MessageBox mb = new MessageBox(shlMain, SWT.ICON_ERROR | SWT.OK);
        mb.setText(title);
        mb.setMessage(message);
        mb.open();
	}

	/**
	 * Open the window.
	 */
	public void open() {
		display = Display.getDefault();
		shlMain = new Shell();
		createContents();
		shlMain.open();
		shlMain.layout();
		
		Properties config = new Properties();
		try {
			config.load(MainScreen.class.getResourceAsStream("/config.properties"));
		} catch (IOException e) {
			e.printStackTrace();
			errorMsgbox("Config Error", "Could not load config file!");
			shlMain.dispose();
			return;
		}
		
		try {
			schema = new Schema(config.getProperty("dbhost"), config.getProperty("schema"), 
					config.getProperty("dbuser"), config.getProperty("dbpass"));
		} catch (Exception e) {
			e.printStackTrace();
			errorMsgbox("DB Error", "Could not connect to DB!");
			shlMain.dispose();
			return;
		}
		
		while (!shlMain.isDisposed()) {
			if (!display.readAndDispatch()) {
				display.sleep();
			}
		}
	}
	
	/**
	 * Create contents of the window.
	 */
	protected void createContents() {
		shlMain.setSize(623, 269);
		shlMain.setText("Pony Trivia");
		shlMain.setLayout(null);
		
		Button btnImportImdbFiles = new Button(shlMain, SWT.NONE);
		btnImportImdbFiles.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				shlMain.setEnabled(false);
				ImportScreen.run(display, schema);
				shlMain.setEnabled(true);
			}
		});
		btnImportImdbFiles.setBounds(10, 150, 239, 66);
		btnImportImdbFiles.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/IMDB-logo.gif"));
		btnImportImdbFiles.setText("Import IMDB Files");
		
		Button btnEditDb = new Button(shlMain, SWT.NONE);
		btnEditDb.setBounds(255, 150, 189, 67);
		btnEditDb.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/mysql-logo.gif"));
		btnEditDb.setText("Edit DB");
		
		Group group = new Group(shlMain, SWT.NONE);
		group.setBounds(10, 10, 582, 134);
		
		Button btnPlayGame = new Button(group, SWT.NONE);
		btnPlayGame.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				shlMain.setEnabled(false);
				GameScreen.run(display, schema);
				shlMain.setEnabled(true);
			}
		});
		btnPlayGame.setBounds(321, 20, 251, 96);
		btnPlayGame.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/kitty1.gif"));
		btnPlayGame.setText("Play Game");
		
		Label lblNewLabel = new Label(group, SWT.NONE);
		lblNewLabel.setBounds(10, 23, 94, 18);
		lblNewLabel.setText("Username:");

		Text playerName = new Text(group, SWT.BORDER);
		playerName.setBounds(110, 20, 126, 24);

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
		
		Button btnHighscores = new Button(shlMain, SWT.NONE);
		btnHighscores.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				shlMain.setEnabled(false);
				HighscoresScreen.run(display, schema);
				shlMain.setEnabled(true);
			}
		});
		btnHighscores.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/grail.gif"));
		btnHighscores.setBounds(450, 150, 142, 67);
		btnHighscores.setText("Highscores");

	}

}
