package ponytrivia.gui;

import java.io.IOException;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Properties;

import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.wb.swt.SWTResourceManager;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.Combo;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleInsert;
import ponytrivia.db.SimpleQuery;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;

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
	
	public static boolean rebuildPopularTables = false;

	protected Display display;
	protected Shell shlMain;
	protected Schema schema; 
	private Table table;
	protected GameScreen.GameConfig gameConfig = new GameScreen.GameConfig();
	
	protected SimpleQuery findPlayer;
	protected SimpleInsert insertPlayer;

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
		
		Properties appConfig = new Properties();
		try {
			appConfig.load(MainScreen.class.getResourceAsStream("/config.properties"));
		} catch (IOException e) {
			e.printStackTrace();
			errorMsgbox("Config Error", "Could not load config file!");
			shlMain.dispose();
			return;
		}
		
		try {
			schema = new Schema(appConfig.getProperty("dbhost"), appConfig.getProperty("schema"), 
					appConfig.getProperty("dbuser"), appConfig.getProperty("dbpass"));
		} catch (Exception e) {
			e.printStackTrace();
			errorMsgbox("DB Error", "Could not connect to DB!");
			shlMain.dispose();
			return;
		}
		
		try {
			String s = appConfig.getProperty("questionTime");
			if (s != null) {
				gameConfig.alotted_time = Integer.parseInt(s);
			}
		} catch (NumberFormatException ex) {
			ex.printStackTrace();
		}

		try {
			String s = appConfig.getProperty("questionsToWin");
			if (s != null) {
				gameConfig.questions_to_win = Integer.parseInt(s);
			}
		} catch (NumberFormatException ex) {
			ex.printStackTrace();
		}

		try {
			String s = appConfig.getProperty("turnsBeforeReenableFiftyFifty");
			if (s != null) {
				gameConfig.initalTurnsForFiftyFifty = Integer.parseInt(s);
			}
		} catch (NumberFormatException ex) {
			ex.printStackTrace();
		}
		
		createContents();
		shlMain.open();
		shlMain.layout();

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
		shlMain.setSize(623, 416);
		shlMain.setText("Welcome to Pony Trivia");
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
		btnImportImdbFiles.setBounds(10, 294, 239, 66);
		btnImportImdbFiles.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/IMDB-logo.gif"));
		btnImportImdbFiles.setText("Import IMDB Files");
		
		Button btnEditDb = new Button(shlMain, SWT.NONE);
		btnEditDb.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				shlMain.setEnabled(false);
				EditScreen.run(display, schema);
				shlMain.setEnabled(true);
			}
		});
		btnEditDb.setBounds(255, 294, 189, 67);
		btnEditDb.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/mysql-logo.gif"));
		btnEditDb.setText("Edit DB");
		
		Group group = new Group(shlMain, SWT.NONE);
		group.setBounds(10, 10, 582, 278);
		
		Button btnPlayGame = new Button(group, SWT.NONE);
		btnPlayGame.setBounds(10, 172, 251, 96);
		btnPlayGame.setImage(SWTResourceManager.getImage(MainScreen.class, "/ponytrivia/gui/res/kitty1.gif"));
		btnPlayGame.setText("Play Game");
		
		Label lblNewLabel = new Label(group, SWT.NONE);
		lblNewLabel.setBounds(10, 23, 94, 18);
		lblNewLabel.setText("Your Name");

		final Text playerName = new Text(group, SWT.BORDER);
		playerName.setBounds(130, 20, 126, 24);

		Label lblGenre = new Label(group, SWT.NONE);
		lblGenre.setText("Movie Genres");
		lblGenre.setBounds(294, 23, 103, 18);
		
		Label lblDecade = new Label(group, SWT.NONE);
		lblDecade.setText("From Decade");
		lblDecade.setBounds(10, 59, 114, 18);
		
		final Combo comboMinYear = new Combo(group, SWT.NONE);
		comboMinYear.setItems(new String[] {"Any", "1950", "1960", "1970", "1980", "1990", "2000"});
		comboMinYear.setBounds(130, 56, 126, 26);
		comboMinYear.select(0);
		
		Label lblToDecade = new Label(group, SWT.NONE);
		lblToDecade.setText("To Decade");
		lblToDecade.setBounds(10, 98, 114, 18);
		
		final Combo comboMaxYear = new Combo(group, SWT.NONE);
		comboMaxYear.setItems(new String[] {"1950", "1960", "1970", "1980", "1990", "2000", "Any"});
		comboMaxYear.setBounds(130, 95, 126, 28);
		comboMaxYear.select(6);
		
		table = new Table(group, SWT.BORDER | SWT.CHECK | SWT.FULL_SELECTION);
		table.setBounds(294, 47, 278, 221);
		table.setHeaderVisible(true);
		table.setLinesVisible(true);
		
		TableColumn tblclmnGenre = new TableColumn(table, SWT.NONE);
		tblclmnGenre.setWidth(239);
		tblclmnGenre.setText("Genre");
		
		try {
			findPlayer = schema.createQuery("user_id", "GamePlayers", "username = ?");
			insertPlayer = schema.createInsert("GamePlayers", false, "username");

			SimpleQuery q = schema.createQuery("genre_id, name", "genres", "true", "name ASC");
			ResultSet rs;
			rs = q.query();
			while (rs.next()) {
				TableItem tableItem = new TableItem(table, SWT.NONE);
				tableItem.setData(rs.getInt(1));
				tableItem.setText(0, rs.getString(2));
			}
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
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
		btnHighscores.setBounds(450, 294, 142, 67);
		btnHighscores.setText("Highscores");
		
		btnPlayGame.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				String player = playerName.getText().trim();
				if (player.isEmpty()) {
					errorMsgbox("Error", "Please enter your name first");
					return;
				}
				gameConfig.playerId = -1;
				try {
					ResultSet rs = findPlayer.query(player);
					if (!rs.next()) {
						gameConfig.playerId = insertPlayer.insert(player);
						schema.commit();
					} else {
						gameConfig.playerId = rs.getInt(1);
					}
					rs.close();
				} catch (SQLException e) {
					e.printStackTrace();
					errorMsgbox("Error", "Could not find/add player");
					return;
				}
				
				shlMain.setEnabled(false);
				int minYear = -1;
				int maxYear = -1;
				switch (comboMinYear.getSelectionIndex()) {
				case 1:
					minYear = 1950;
					break;
				case 2:
					minYear = 1960;
					break;
				case 3:
					minYear = 1970;
					break;
				case 4:
					minYear = 1980;
					break;
				case 5:
					minYear = 1990;
					break;
				case 6:
					minYear = 2000;
					break;
				}
				switch (comboMaxYear.getSelectionIndex()) {
				case 0:
					maxYear = 1950;
					break;
				case 1:
					maxYear = 1960;
					break;
				case 2:
					maxYear = 1970;
					break;
				case 3:
					maxYear = 1980;
					break;
				case 4:
					maxYear = 1990;
					break;
				case 5:
					maxYear = 2000;
					break;
				}
				
				ArrayList<Integer> genre_ids = new ArrayList<Integer>();
				for (TableItem ti : table.getItems()) {
					if (ti.getChecked()) {
						genre_ids.add((Integer)ti.getData());
					}
				}
				
				ApplyFilterScreen.run(display, schema, gameConfig, minYear, maxYear, genre_ids);
				shlMain.setEnabled(true);
			}
		});

	}
}
