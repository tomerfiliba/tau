package ponytrivia.gui;

import java.sql.Date;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.HashSet;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.TabFolder;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.widgets.TabItem;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.Spinner;

import ponytrivia.db.Schema;
import ponytrivia.db.SimpleInsert;
import ponytrivia.db.SimpleQuery;
import ponytrivia.db.SimpleUpdate;

import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.widgets.DateTime;

public class EditScreen extends Shell {
	private Text text;
	private Table table;
	private Text textName;
	private Spinner textYear;
	private Spinner textRating;
	private Spinner textVotes;
	
	protected Schema schema;
	protected SimpleQuery findMovie;
	protected SimpleQuery getMovieDetails;
	protected SimpleQuery getMovieGenres;
	protected SimpleQuery getAllGenres;
	protected SimpleUpdate updateMovieDetails;
	protected SimpleInsert insertMovieDetails;
	protected SimpleInsert insertMovieDetailsToPopular;
	protected SimpleInsert insertMovieGenre;
	protected PreparedStatement discardMovieGenres;
	
	protected SimpleQuery findPerson1;
	protected SimpleQuery findPerson2;
	protected SimpleQuery getPersonDetails;
	protected SimpleUpdate updatePersonDetails;
	protected SimpleInsert insertPersonDetails;

	private Text textPersonNameSearch;
	private Text textPersonFN;
	private Text textPersonMN;
	private Text textPersonLN;
	private Text textPersonNN;
	private Text textPersonRN;
	private Text text_1;
	private Spinner text_2;
	private Text text_3;
	private Text text_4;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void run(Display display, Schema schema) {
		try {
			EditScreen shell = new EditScreen(display, schema);
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

	protected void errorMsgBox(String title, String message) {
		MessageBox mb = new MessageBox(this, SWT.ICON_ERROR | SWT.OK);
        mb.setText(title);
        mb.setMessage(message);
        mb.open();
	}

	
	/**
	 * Create the shell.
	 * @param display
	 */
	public EditScreen(Display display, final Schema schema) {
		super(display, SWT.SHELL_TRIM);
		this.schema = schema;
		setMinimumSize(new Point(500, 300));
		setLayout(new FormLayout());
		setText("Edit DB");
		setSize(809, 582);
		
		TabFolder tabFolder = new TabFolder(this, SWT.NONE);
		FormData fd_tabFolder = new FormData();
		fd_tabFolder.top = new FormAttachment(0, 10);
		fd_tabFolder.left = new FormAttachment(0, 10);
		fd_tabFolder.bottom = new FormAttachment(100, -10);
		fd_tabFolder.right = new FormAttachment(100, -10);
		tabFolder.setLayoutData(fd_tabFolder);
		
		TabItem tbtmMovies = new TabItem(tabFolder, SWT.NONE);
		tbtmMovies.setText("Movies");
		
		Composite composite = new Composite(tabFolder, SWT.NONE);
		tbtmMovies.setControl(composite);
		FillLayout fl_composite = new FillLayout(SWT.HORIZONTAL);
		fl_composite.spacing = 6;
		fl_composite.marginHeight = 5;
		fl_composite.marginWidth = 5;
		composite.setLayout(fl_composite);
		
		Group grpLocateMovie = new Group(composite, SWT.NONE);
		grpLocateMovie.setText("Locate Movie");
		grpLocateMovie.setLayout(new FormLayout());
		
		Label lblMovieName = new Label(grpLocateMovie, SWT.NONE);
		FormData fd_lblMovieName = new FormData();
		fd_lblMovieName.top = new FormAttachment(0, 10);
		fd_lblMovieName.left = new FormAttachment(0, 10);
		lblMovieName.setLayoutData(fd_lblMovieName);
		lblMovieName.setText("Movie name:");
		
		text = new Text(grpLocateMovie, SWT.BORDER);
		FormData fd_text = new FormData();
		fd_text.top = new FormAttachment(lblMovieName, -3, SWT.TOP);
		fd_text.left = new FormAttachment(lblMovieName, 18);
		fd_text.right = new FormAttachment(100, -10);
		text.setLayoutData(fd_text);
		
		final List list = new List(grpLocateMovie, SWT.BORDER | SWT.V_SCROLL);
		FormData fd_list = new FormData();
		fd_list.left = new FormAttachment(0, 10);
		fd_list.right = new FormAttachment(100, -10);
		fd_list.top = new FormAttachment(text, 23);
		fd_list.bottom = new FormAttachment(100, -10);
		list.setLayoutData(fd_list);
		
		final HashMap<Integer, Integer> movieIdMapping = new HashMap<Integer, Integer>();
		final HashMap<Integer, Integer> personIdMapping = new HashMap<Integer, Integer>();
		final HashMap<Integer, String> allGenres = new HashMap<Integer, String>();
		
		text.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent arg0) {
				if (text.getText().length() < 3) {
					return;
				}
				String movie_name = "%"+ text.getText() + "%";
				list.removeAll();
				movieIdMapping.clear();
				try {
					ResultSet rs = findMovie.query(movie_name);
					int i = 0;
					while (rs.next()) {
						list.add(rs.getString(1));
						movieIdMapping.put(i, rs.getInt(2));
						i++;
					}
				} catch (SQLException e) {
					e.printStackTrace();
				}
			}
		});
		
		Group grpEditDetails = new Group(composite, SWT.NONE);
		grpEditDetails.setText("Edit Details");
		grpEditDetails.setLayout(new FormLayout());
		
		Label lblName = new Label(grpEditDetails, SWT.NONE);
		FormData fd_lblName = new FormData();
		fd_lblName.top = new FormAttachment(0, 10);
		fd_lblName.left = new FormAttachment(0, 10);
		lblName.setLayoutData(fd_lblName);
		lblName.setText("Name:");
		
		Label lblYear = new Label(grpEditDetails, SWT.NONE);
		FormData fd_lblYear = new FormData();
		fd_lblYear.top = new FormAttachment(lblName, 16);
		fd_lblYear.left = new FormAttachment(0, 10);
		lblYear.setLayoutData(fd_lblYear);
		lblYear.setText("Year:");
		
		Label lblRating = new Label(grpEditDetails, SWT.NONE);
		FormData fd_lblRating = new FormData();
		fd_lblRating.top = new FormAttachment(lblYear, 17);
		fd_lblRating.left = new FormAttachment(0, 10);
		lblRating.setLayoutData(fd_lblRating);
		lblRating.setText("Rating:");
		
		Label lblVotes = new Label(grpEditDetails, SWT.NONE);
		FormData fd_lblVotes = new FormData();
		fd_lblVotes.top = new FormAttachment(lblRating, 16);
		fd_lblVotes.left = new FormAttachment(0, 10);
		lblVotes.setLayoutData(fd_lblVotes);
		lblVotes.setText("Votes:");
		
		Label lblType = new Label(grpEditDetails, SWT.NONE);
		FormData fd_lblType = new FormData();
		fd_lblType.top = new FormAttachment(lblVotes, 20);
		fd_lblType.left = new FormAttachment(lblName, 0, SWT.LEFT);
		lblType.setLayoutData(fd_lblType);
		lblType.setText("Type:");
		
		final Button btnFilm = new Button(grpEditDetails, SWT.RADIO);
		FormData fd_btnFilm = new FormData();
		fd_btnFilm.top = new FormAttachment(lblType, 0, SWT.TOP);
		btnFilm.setLayoutData(fd_btnFilm);
		btnFilm.setText("Film");
		
		final Button btnTvShow = new Button(grpEditDetails, SWT.RADIO);
		fd_btnFilm.right = new FormAttachment(btnTvShow, -6);
		FormData fd_btnTvShow = new FormData();
		fd_btnTvShow.left = new FormAttachment(0, 194);
		fd_btnTvShow.top = new FormAttachment(lblType, 0, SWT.TOP);
		btnTvShow.setLayoutData(fd_btnTvShow);
		btnTvShow.setText("TV Show");
		
		table = new Table(grpEditDetails, SWT.BORDER | SWT.CHECK | SWT.FULL_SELECTION);
		FormData fd_table = new FormData();
		fd_table.top = new FormAttachment(lblType, 19);
		fd_table.left = new FormAttachment(lblName, 0, SWT.LEFT);
		fd_table.right = new FormAttachment(100, -10);
		table.setLayoutData(fd_table);
		table.setHeaderVisible(true);
		table.setLinesVisible(true);
		
		TableColumn tblclmnNewColumn = new TableColumn(table, SWT.NONE);
		tblclmnNewColumn.setWidth(310);
		tblclmnNewColumn.setText("Genres");
		
		final Button btnUpdate = new Button(grpEditDetails, SWT.NONE);
		btnUpdate.setEnabled(false);
		fd_table.bottom = new FormAttachment(btnUpdate, -10);
		FormData fd_btnUpdate = new FormData();
		fd_btnUpdate.right = new FormAttachment(100, -10);
		fd_btnUpdate.bottom = new FormAttachment(100, -10);
		
		btnUpdate.setLayoutData(fd_btnUpdate);
		btnUpdate.setText("Update Existing");
		
		textName = new Text(grpEditDetails, SWT.BORDER);
		FormData fd_textName = new FormData();
		fd_textName.bottom = new FormAttachment(lblName, 0, SWT.BOTTOM);
		fd_textName.left = new FormAttachment(btnFilm, 0, SWT.LEFT);
		fd_textName.right = new FormAttachment(100, -10);
		textName.setLayoutData(fd_textName);
		
		textYear = new Spinner(grpEditDetails, SWT.BORDER);
		textYear.setMaximum(2020);
		textYear.setMinimum(1900);
		textYear.setSelection(1990);
		FormData fd_textYear = new FormData();
		fd_textYear.bottom = new FormAttachment(lblYear, 0, SWT.BOTTOM);
		fd_textYear.left = new FormAttachment(btnFilm, 0, SWT.LEFT);
		fd_textYear.right = new FormAttachment(100, -10);
		textYear.setLayoutData(fd_textYear);
		
		textRating = new Spinner(grpEditDetails, SWT.BORDER);
		textRating.setDigits(1);
		textRating.setSelection(60);
		FormData fd_textRating = new FormData();
		fd_textRating.bottom = new FormAttachment(lblRating, 0, SWT.BOTTOM);
		fd_textRating.left = new FormAttachment(btnFilm, 0, SWT.LEFT);
		fd_textRating.right = new FormAttachment(100, -10);
		textRating.setLayoutData(fd_textRating);
		
		textVotes = new Spinner(grpEditDetails, SWT.BORDER);
		textVotes.setMaximum(5000000);
		FormData fd_textVotes = new FormData();
		fd_textVotes.bottom = new FormAttachment(lblVotes, 0, SWT.BOTTOM);
		fd_textVotes.left = new FormAttachment(btnFilm, 0, SWT.LEFT);
		fd_textVotes.right = new FormAttachment(100, -10);
		textVotes.setLayoutData(fd_textVotes);
		
		Button btnAddNew = new Button(grpEditDetails, SWT.NONE);
		FormData fd_btnAddNew = new FormData();
		fd_btnAddNew.bottom = new FormAttachment(btnUpdate, 0, SWT.BOTTOM);
		fd_btnAddNew.right = new FormAttachment(btnUpdate, -6);
		btnAddNew.setLayoutData(fd_btnAddNew);
		btnAddNew.setText("Add New");
		
		TabItem tbtmPeople = new TabItem(tabFolder, SWT.NONE);
		tbtmPeople.setText("People");
		
		Composite composite_1 = new Composite(tabFolder, SWT.NONE);
		tbtmPeople.setControl(composite_1);
		FillLayout fl_composite_1 = new FillLayout(SWT.HORIZONTAL);
		fl_composite_1.spacing = 5;
		fl_composite_1.marginHeight = 5;
		fl_composite_1.marginWidth = 5;
		composite_1.setLayout(fl_composite_1);
		
		Group grpFindPerson = new Group(composite_1, SWT.NONE);
		grpFindPerson.setText("Locate Person");
		grpFindPerson.setLayout(new FormLayout());
		
		Label lblName_1 = new Label(grpFindPerson, SWT.NONE);
		FormData fd_lblName_1 = new FormData();
		fd_lblName_1.top = new FormAttachment(0, 10);
		fd_lblName_1.left = new FormAttachment(0, 10);
		lblName_1.setLayoutData(fd_lblName_1);
		lblName_1.setText("Name:");
		
		textPersonNameSearch = new Text(grpFindPerson, SWT.BORDER);
		FormData fd_textPersonNameSearch = new FormData();
		fd_textPersonNameSearch.bottom = new FormAttachment(lblName_1, 0, SWT.BOTTOM);
		fd_textPersonNameSearch.left = new FormAttachment(lblName_1, 13);
		fd_textPersonNameSearch.right = new FormAttachment(100, -10);
		textPersonNameSearch.setLayoutData(fd_textPersonNameSearch);
		
		
		final List list_1 = new List(grpFindPerson, SWT.BORDER);
		FormData fd_list_1 = new FormData();
		fd_list_1.top = new FormAttachment(lblName_1, 6);
		fd_list_1.left = new FormAttachment(lblName_1, 0, SWT.LEFT);
		fd_list_1.right = new FormAttachment(100, -10);
		fd_list_1.bottom = new FormAttachment(100, -10);
		list_1.setLayoutData(fd_list_1);
		
		Group grpEditDetails_1 = new Group(composite_1, SWT.NONE);
		grpEditDetails_1.setText("Edit Details");
		grpEditDetails_1.setLayout(new FormLayout());
		
		Label lblFirstName = new Label(grpEditDetails_1, SWT.NONE);
		FormData fd_lblFirstName = new FormData();
		fd_lblFirstName.top = new FormAttachment(0, 10);
		fd_lblFirstName.left = new FormAttachment(0, 10);
		lblFirstName.setLayoutData(fd_lblFirstName);
		lblFirstName.setText("First name:");
		
		Label lblMiddleName = new Label(grpEditDetails_1, SWT.NONE);
		FormData fd_lblMiddleName = new FormData();
		fd_lblMiddleName.top = new FormAttachment(lblFirstName, 18);
		fd_lblMiddleName.left = new FormAttachment(lblFirstName, 0, SWT.LEFT);
		lblMiddleName.setLayoutData(fd_lblMiddleName);
		lblMiddleName.setText("Middle name:");
		
		Label lblLastName = new Label(grpEditDetails_1, SWT.NONE);
		FormData fd_lblLastName = new FormData();
		fd_lblLastName.top = new FormAttachment(lblMiddleName, 15);
		fd_lblLastName.left = new FormAttachment(lblFirstName, 0, SWT.LEFT);
		lblLastName.setLayoutData(fd_lblLastName);
		lblLastName.setText("Last name:");
		
		Label lblNickName = new Label(grpEditDetails_1, SWT.NONE);
		FormData fd_lblNickName = new FormData();
		fd_lblNickName.top = new FormAttachment(lblLastName, 16);
		fd_lblNickName.left = new FormAttachment(lblFirstName, 0, SWT.LEFT);
		lblNickName.setLayoutData(fd_lblNickName);
		lblNickName.setText("Nick name:");
		
		Label lblRealName = new Label(grpEditDetails_1, SWT.NONE);
		FormData fd_lblRealName = new FormData();
		fd_lblRealName.top = new FormAttachment(lblNickName, 19);
		fd_lblRealName.left = new FormAttachment(lblFirstName, 0, SWT.LEFT);
		lblRealName.setLayoutData(fd_lblRealName);
		lblRealName.setText("Real name:");
		
		Label lblBirthDate = new Label(grpEditDetails_1, SWT.NONE);
		FormData fd_lblBirthDate = new FormData();
		fd_lblBirthDate.top = new FormAttachment(lblRealName, 20);
		fd_lblBirthDate.left = new FormAttachment(lblFirstName, 0, SWT.LEFT);
		lblBirthDate.setLayoutData(fd_lblBirthDate);
		lblBirthDate.setText("Birth date:");
		
		Label lblDeathDate = new Label(grpEditDetails_1, SWT.NONE);
		FormData fd_lblDeathDate = new FormData();
		fd_lblDeathDate.top = new FormAttachment(lblBirthDate, 17);
		fd_lblDeathDate.left = new FormAttachment(lblFirstName, 0, SWT.LEFT);
		lblDeathDate.setLayoutData(fd_lblDeathDate);
		lblDeathDate.setText("Death date:");
		
		final DateTime birthDate = new DateTime(grpEditDetails_1, SWT.BORDER);
		FormData fd_birthDate = new FormData();
		fd_birthDate.bottom = new FormAttachment(lblBirthDate, 0, SWT.BOTTOM);
		birthDate.setLayoutData(fd_birthDate);
		
		textPersonFN = new Text(grpEditDetails_1, SWT.BORDER);
		fd_birthDate.left = new FormAttachment(textPersonFN, 0, SWT.LEFT);
		FormData fd_textPersonFN = new FormData();
		fd_textPersonFN.bottom = new FormAttachment(lblFirstName, 0, SWT.BOTTOM);
		fd_textPersonFN.left = new FormAttachment(lblFirstName, 32);
		fd_textPersonFN.right = new FormAttachment(100, -10);
		textPersonFN.setLayoutData(fd_textPersonFN);
		
		textPersonMN = new Text(grpEditDetails_1, SWT.BORDER);
		FormData fd_textPersonMN = new FormData();
		fd_textPersonMN.bottom = new FormAttachment(lblMiddleName, 0, SWT.BOTTOM);
		fd_textPersonMN.left = new FormAttachment(textPersonFN, 0, SWT.LEFT);
		fd_textPersonMN.right = new FormAttachment(100, -10);
		textPersonMN.setLayoutData(fd_textPersonMN);
		
		textPersonLN = new Text(grpEditDetails_1, SWT.BORDER);
		FormData fd_textPersonLN = new FormData();
		fd_textPersonLN.bottom = new FormAttachment(lblLastName, 0, SWT.BOTTOM);
		fd_textPersonLN.left = new FormAttachment(textPersonFN, 0, SWT.LEFT);
		fd_textPersonLN.right = new FormAttachment(100, -10);
		textPersonLN.setLayoutData(fd_textPersonLN);
		
		textPersonNN = new Text(grpEditDetails_1, SWT.BORDER);
		FormData fd_textPersonNN = new FormData();
		fd_textPersonNN.bottom = new FormAttachment(lblNickName, 0, SWT.BOTTOM);
		fd_textPersonNN.left = new FormAttachment(textPersonFN, 0, SWT.LEFT);
		fd_textPersonNN.right = new FormAttachment(100, -10);
		textPersonNN.setLayoutData(fd_textPersonNN);
		
		textPersonRN = new Text(grpEditDetails_1, SWT.BORDER);
		FormData fd_textPersonRN = new FormData();
		fd_textPersonRN.bottom = new FormAttachment(lblRealName, 0, SWT.BOTTOM);
		fd_textPersonRN.left = new FormAttachment(textPersonFN, 0, SWT.LEFT);
		fd_textPersonRN.right = new FormAttachment(100, -10);
		textPersonRN.setLayoutData(fd_textPersonRN);
		
		final DateTime deathDate = new DateTime(grpEditDetails_1, SWT.BORDER);
		deathDate.setEnabled(false);
		FormData fd_deathDate = new FormData();
		fd_deathDate.bottom = new FormAttachment(lblDeathDate, 0, SWT.BOTTOM);
		fd_deathDate.left = new FormAttachment(birthDate, 0, SWT.LEFT);
		deathDate.setLayoutData(fd_deathDate);
		
		final Button btnAlive = new Button(grpEditDetails_1, SWT.CHECK);
		btnAlive.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				deathDate.setEnabled(!btnAlive.getSelection());
			}
		});
		btnAlive.setSelection(true);
		FormData fd_btnAlive = new FormData();
		fd_btnAlive.top = new FormAttachment(lblDeathDate, 0, SWT.TOP);
		fd_btnAlive.left = new FormAttachment(deathDate, 6);
		btnAlive.setLayoutData(fd_btnAlive);
		btnAlive.setText("Alive");
		
		Label lblGender = new Label(grpEditDetails_1, SWT.NONE);
		FormData fd_lblGender = new FormData();
		fd_lblGender.top = new FormAttachment(lblDeathDate, 16);
		fd_lblGender.left = new FormAttachment(lblFirstName, 0, SWT.LEFT);
		lblGender.setLayoutData(fd_lblGender);
		lblGender.setText("Gender:");
		
		final Button btnMale = new Button(grpEditDetails_1, SWT.RADIO);
		FormData fd_btnMale = new FormData();
		fd_btnMale.bottom = new FormAttachment(lblGender, 0, SWT.BOTTOM);
		fd_btnMale.left = new FormAttachment(birthDate, 0, SWT.LEFT);
		btnMale.setLayoutData(fd_btnMale);
		btnMale.setText("Male");
		
		final Button btnFemale = new Button(grpEditDetails_1, SWT.RADIO);
		FormData fd_btnFemale = new FormData();
		fd_btnFemale.bottom = new FormAttachment(lblGender, 0, SWT.BOTTOM);
		fd_btnFemale.left = new FormAttachment(btnMale, 19);
		btnFemale.setLayoutData(fd_btnFemale);
		btnFemale.setText("Female");
		
		final Button btnUpdate2 = new Button(grpEditDetails_1, SWT.NONE);
		btnUpdate2.setEnabled(false);
		FormData fd_btnUpdate2 = new FormData();
		fd_btnUpdate2.right = new FormAttachment(100, -10);
		fd_btnUpdate2.bottom = new FormAttachment(100, -10);
		btnUpdate2.setLayoutData(fd_btnUpdate2);
		btnUpdate2.setText("Update Existing");
		
		Button btnAddNew2 = new Button(grpEditDetails_1, SWT.NONE);
		fd_btnUpdate2.top = new FormAttachment(btnAddNew2, 0, SWT.TOP);
		FormData fd_btnAddNew2 = new FormData();
		fd_btnAddNew2.bottom = new FormAttachment(100, -10);
		fd_btnAddNew2.right = new FormAttachment(btnUpdate2, -6);
		btnAddNew2.setLayoutData(fd_btnAddNew2);
		btnAddNew2.setText("Add New");
		
		TabItem tbtmRoles = new TabItem(tabFolder, SWT.NONE);
		tbtmRoles.setText("Actors");
		
		Composite composite_2 = new Composite(tabFolder, SWT.NONE);
		tbtmRoles.setControl(composite_2);
		composite_2.setLayout(new FormLayout());
		
		Group grpDetails = new Group(composite_2, SWT.NONE);
		grpDetails.setText("Details");
		grpDetails.setLayout(new FormLayout());
		FormData fd_grpDetails = new FormData();
		fd_grpDetails.bottom = new FormAttachment(100, -10);
		fd_grpDetails.left = new FormAttachment(0, 10);
		fd_grpDetails.top = new FormAttachment(100, -130);
		fd_grpDetails.right = new FormAttachment(100, -10);
		grpDetails.setLayoutData(fd_grpDetails);
		
		Label lblCharacter = new Label(grpDetails, SWT.NONE);
		FormData fd_lblCharacter = new FormData();
		fd_lblCharacter.top = new FormAttachment(0, 10);
		fd_lblCharacter.left = new FormAttachment(0, 10);
		lblCharacter.setLayoutData(fd_lblCharacter);
		lblCharacter.setText("Character:");
		
		text_1 = new Text(grpDetails, SWT.BORDER);
		FormData fd_text_1 = new FormData();
		fd_text_1.top = new FormAttachment(lblCharacter, -3, SWT.TOP);
		fd_text_1.right = new FormAttachment(lblCharacter, 179, SWT.RIGHT);
		fd_text_1.left = new FormAttachment(lblCharacter, 47);
		text_1.setLayoutData(fd_text_1);
		
		Label lblCreditPosition = new Label(grpDetails, SWT.NONE);
		FormData fd_lblCreditPosition = new FormData();
		fd_lblCreditPosition.top = new FormAttachment(lblCharacter, 18);
		fd_lblCreditPosition.left = new FormAttachment(0, 10);
		lblCreditPosition.setLayoutData(fd_lblCreditPosition);
		lblCreditPosition.setText("Credit position:");
		
		text_2 = new Spinner(grpDetails, SWT.BORDER);
		FormData fd_text_2 = new FormData();
		fd_text_2.top = new FormAttachment(lblCreditPosition, -3, SWT.TOP);
		fd_text_2.left = new FormAttachment(text_1, 0, SWT.LEFT);
		fd_text_2.right = new FormAttachment(text_1, 0, SWT.RIGHT);
		text_2.setLayoutData(fd_text_2);
		
		Button btnAddRole = new Button(grpDetails, SWT.NONE);
		FormData fd_btnAddRole = new FormData();
		fd_btnAddRole.bottom = new FormAttachment(100, -10);
		fd_btnAddRole.right = new FormAttachment(100, -10);
		btnAddRole.setLayoutData(fd_btnAddRole);
		btnAddRole.setText("Add/Update Role");
		
		Button btnRemoveRole = new Button(grpDetails, SWT.NONE);
		FormData fd_btnRemoveRole = new FormData();
		fd_btnRemoveRole.bottom = new FormAttachment(100, -10);
		fd_btnRemoveRole.right = new FormAttachment(btnAddRole, -15);
		btnRemoveRole.setLayoutData(fd_btnRemoveRole);
		btnRemoveRole.setText("Remove Role");
		
		Composite composite_4 = new Composite(composite_2, SWT.NONE);
		FillLayout fl_composite_4 = new FillLayout(SWT.HORIZONTAL);
		fl_composite_4.marginWidth = 5;
		fl_composite_4.marginHeight = 5;
		fl_composite_4.spacing = 5;
		composite_4.setLayout(fl_composite_4);
		FormData fd_composite_4 = new FormData();
		fd_composite_4.top = new FormAttachment(0, 10);
		fd_composite_4.left = new FormAttachment(0, 10);
		fd_composite_4.right = new FormAttachment(100, -10);
		fd_composite_4.bottom = new FormAttachment(grpDetails, -10);
		//fd_composite_4.left = new FormAttachment(grpDetails, 0, SWT.LEFT);
		composite_4.setLayoutData(fd_composite_4);
		
		Group grpLocateMovie_1 = new Group(composite_4, SWT.NONE);
		grpLocateMovie_1.setText("Locate Movie");
		grpLocateMovie_1.setLayout(new FormLayout());
		
		Label lblMovieName_1 = new Label(grpLocateMovie_1, SWT.NONE);
		FormData fd_lblMovieName_1 = new FormData();
		fd_lblMovieName_1.top = new FormAttachment(0, 10);
		fd_lblMovieName_1.left = new FormAttachment(0, 10);
		lblMovieName_1.setLayoutData(fd_lblMovieName_1);
		lblMovieName_1.setText("Movie name:");
		
		text_3 = new Text(grpLocateMovie_1, SWT.BORDER);
		FormData fd_text_3 = new FormData();
		fd_text_3.top = new FormAttachment(lblMovieName_1, -3, SWT.TOP);
		fd_text_3.left = new FormAttachment(lblMovieName_1, 6);
		fd_text_3.right = new FormAttachment(100, -10);
		text_3.setLayoutData(fd_text_3);
		
		List list_2 = new List(grpLocateMovie_1, SWT.BORDER);
		FormData fd_list_2 = new FormData();
		fd_list_2.top = new FormAttachment(lblMovieName_1, 6);
		fd_list_2.left = new FormAttachment(0, 10);
		fd_list_2.right = new FormAttachment(100, -10);
		fd_list_2.bottom = new FormAttachment(100, -10);
		
		list_2.setLayoutData(fd_list_2);
		
		Group grpLocateActor = new Group(composite_4, SWT.NONE);
		grpLocateActor.setText("Locate Actor");
		grpLocateActor.setLayout(new FormLayout());
		
		Label lblPersonName = new Label(grpLocateActor, SWT.NONE);
		FormData fd_lblPersonName = new FormData();
		fd_lblPersonName.top = new FormAttachment(0, 10);
		fd_lblPersonName.left = new FormAttachment(0, 10);
		lblPersonName.setLayoutData(fd_lblPersonName);
		lblPersonName.setText("Person name:");
		
		text_4 = new Text(grpLocateActor, SWT.BORDER);
		FormData fd_text_4 = new FormData();
		fd_text_4.top = new FormAttachment(lblPersonName, -3, SWT.TOP);
		fd_text_4.left = new FormAttachment(lblPersonName, 6);
		fd_text_4.right = new FormAttachment(100, -10);
		text_4.setLayoutData(fd_text_4);
		
		List list_3 = new List(grpLocateActor, SWT.BORDER);
		FormData fd_list_3 = new FormData();
		fd_list_3.top = new FormAttachment(lblPersonName, 6);
		fd_list_3.left = new FormAttachment(lblPersonName, 0, SWT.LEFT);
		fd_list_3.bottom = new FormAttachment(100, -10);
		fd_list_3.right = new FormAttachment(100, -10);
		list_3.setLayoutData(fd_list_3);
		
		TabItem tbtmDirectors = new TabItem(tabFolder, SWT.NONE);
		tbtmDirectors.setText("Directors");
		
		Composite composite_3 = new Composite(tabFolder, SWT.NONE);
		tbtmDirectors.setControl(composite_3);

		try {
			findMovie = schema.createQuery("DISTINCT M.name, M.movie_id", 
					"PopularMovies as PM, Movies as M",
					"PM.movie_id = M.movie_id AND M.name LIKE ?", "name ASC", 20);
			getMovieDetails = schema.createQuery("name, is_film, year, rating, votes", "Movies", 
					"movie_id = ?", 1);
			getMovieGenres = schema.createQuery("MG.genre", "MovieGenres as MG", "MG.movie = ?");
			getAllGenres = schema.createQuery("G.genre_id, G.name", "Genres as G", "true");
			
			updateMovieDetails = schema.createUpdate("Movies", false, "name = ?, is_film = ?, " +
					"year = ?, rating = ?, votes = ?", "movie_id = ?");
			insertMovieDetails = schema.createInsert("Movies", false, "imdb_name", "name", "is_film",
					"year", "rating", "votes");
			insertMovieDetailsToPopular = schema.createInsert("PopularMovies", true, "movie_id"); 
			discardMovieGenres = schema.prepareStatement("DELETE FROM MovieGenres WHERE movie = ?");
			insertMovieGenre = schema.createInsert("MovieGenres", false, "movie", "genre");

			findPerson1 = schema.createQuery("DISTINCT P.imdb_name, P.person_id", "People AS P",
					"P.first_name LIKE ?", "P.imdb_name ASC", 20);
			findPerson2 = schema.createQuery("DISTINCT P.imdb_name, P.person_id", "People AS P",
					"P.first_name LIKE ? AND P.last_name LIKE ?", "P.imdb_name ASC", 20);
			getPersonDetails = schema.createQuery("first_name, middle_name, last_name, " +
					"nick_name, real_name, birth_date, death_date, gender", "People",
					"person_id = ?", 1);
			updatePersonDetails = schema.createUpdate("People", false, "first_name = ?, middle_name = ?, " +
					"last_name = ?, nick_name = ?, real_name = ?, birth_date = ?, death_date = ?, " +
					"gender = ?", "person_id = ?");
			insertPersonDetails = schema.createInsert("People", false, "imdb_name", "first_name", 
					"middle_name", "last_name", "nick_name", "real_name", "birth_date", "death_date",
					"gender");
			
			ResultSet rs = getAllGenres.query();
			while (rs.next()) {
				allGenres.put(rs.getInt(1), rs.getString(2));
				TableItem tableItem = new TableItem(table, SWT.NONE);
				tableItem.setData(rs.getInt(1));
				tableItem.setText(0, rs.getString(2));
			}
			rs.close();
		} catch (SQLException e) {
			e.printStackTrace();
		}

		btnAddNew.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				String name = textName.getText().trim();
				if (name.isEmpty()) {
					errorMsgBox("Error", "Name must not be empty");
					return;
				}
				int year = textYear.getSelection();
				if (year < 1900) {
					errorMsgBox("Error", "Year must be >= 1900");
					return;
				}
				
				Double rating = textRating.getSelection() / 10.0;
				int votes = textVotes.getSelection();
				String imdb_name = name + " (" + year + ")";
				
				try {
					int movie_id = -1;
					try {
						movie_id = insertMovieDetails.insert(imdb_name, name, btnFilm.getSelection() ? 1 : 0, 
							year, rating, votes);
					} catch (SQLException ex) {
						errorMsgBox("Error", "Movie already exists");
						return;
					}
					insertMovieDetailsToPopular.insert(movie_id);
					
					discardMovieGenres.setInt(1, movie_id);
					discardMovieGenres.executeUpdate();
					for (TableItem ti : table.getItems()) {
						if (ti.getChecked()) {
							insertMovieGenre.insert(movie_id, (Integer)ti.getData());
						}
					}
					
					schema.commit();
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		});
		
		btnUpdate.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				String name = textName.getText().trim();
				if (name.isEmpty()) {
					errorMsgBox("Error", "Name must not be empty");
					return;
				}
				Integer movie_id = (Integer)textName.getData();
				int year = textYear.getSelection();
				if (year < 1900) {
					errorMsgBox("Error", "Year must be >= 1900");
					return;
				}
				Double rating = textRating.getSelection() / 10.0;
				int votes = textVotes.getSelection();
				
				if (movie_id == null) {
					return;
				}
				
				try {
					updateMovieDetails.update(name, btnFilm.getSelection() ? 1 : 0, year, 
							rating, votes, movie_id);

					discardMovieGenres.setInt(1, movie_id);
					discardMovieGenres.executeUpdate();
					for (TableItem ti : table.getItems()) {
						if (ti.getChecked()) {
							insertMovieGenre.insert(movie_id, (Integer)ti.getData());
						}
					}
					
					schema.commit();
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		});
		
		list.addSelectionListener(new SelectionListener() {
			@Override
			public void widgetDefaultSelected(SelectionEvent arg0) {
			}

			@Override
			public void widgetSelected(SelectionEvent arg0) {
				int selind = list.getSelectionIndex();
				int movie_id = movieIdMapping.get(selind);
				
				try {
					ResultSet rs = getMovieDetails.query(movie_id);
					if (!rs.next()) {
						return;
					}
					textName.setData(movie_id);
					textName.setText(rs.getString(1));
					if (rs.getInt(2) == 1) {
						btnFilm.setSelection(true);
					} else {
						btnTvShow.setSelection(true);
					}
					textYear.setSelection(rs.getInt(3));
					textRating.setSelection((int)(rs.getDouble(4) * 10));
					textVotes.setSelection(rs.getInt(5));
					rs.close();
					
					HashSet<Integer> genreIds = new HashSet<Integer>();
					rs = getMovieGenres.query(movie_id);
					while (rs.next()) {
						genreIds.add(rs.getInt(1));
					}
					rs.close();
					
					for (TableItem ti : table.getItems()) {
						ti.setChecked(genreIds.contains((Integer)ti.getData()));
					}
					btnUpdate.setEnabled(true);
				} catch (SQLException e) {
					e.printStackTrace();
					return;
				}
			}
		});
		
		textPersonNameSearch.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent arg0) {
				if (textPersonNameSearch.getText().length() < 3) {
					return;
				}
				String[] names = textPersonNameSearch.getText().split(" ");
				
				ResultSet rs;
				try {
					if (names.length == 1) {
						rs = findPerson1.query(names[0] + "%");
					}
					else {
						rs = findPerson2.query(names[0] + "%", names[1] + "%");
					}
					
					list_1.removeAll();
					personIdMapping.clear();
					int i = 0;
					while (rs.next()) {
						list_1.add(rs.getString(1));
						personIdMapping.put(i, rs.getInt(2));
						i++;
					}
					rs.close();
				} catch (SQLException e) {
					e.printStackTrace();
				}
			}
		});
		
		list_1.addSelectionListener(new SelectionListener() {
			@Override
			public void widgetDefaultSelected(SelectionEvent arg0) {
			}

			@SuppressWarnings("deprecation")
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				int selind = list_1.getSelectionIndex();
				int movie_id = personIdMapping.get(selind);
				
				try {
					ResultSet rs = getPersonDetails.query(movie_id);
					if (!rs.next()) {
						return;
					}
					textPersonFN.setData(movie_id);
					textPersonFN.setText(rs.getString(1) == null ? "" : rs.getString(1));
					textPersonMN.setText(rs.getString(2) == null ? "" : rs.getString(2));
					textPersonLN.setText(rs.getString(3) == null ? "" : rs.getString(3));
					textPersonNN.setText(rs.getString(4) == null ? "" : rs.getString(4));
					textPersonRN.setText(rs.getString(5) == null ? "" : rs.getString(5));
					
					Date d = rs.getDate(6);
					if (d == null) {
						birthDate.setDate(1800, 1, 1);
					} else {
						birthDate.setDate(d.getYear() + 1900, d.getMonth(), d.getDay());
					}
					d = rs.getDate(7);
					if (d == null) {
						btnAlive.setSelection(true);
					} else {
						btnAlive.setSelection(false);
						deathDate.setDate(d.getYear(), d.getMonth(), d.getDay());
					}
					if (rs.getString(8).equals("m")) {
						btnMale.setSelection(true);
					}
					else {
						btnFemale.setSelection(true);
					}
					
					btnUpdate2.setEnabled(true);
				} catch (SQLException e) {
					e.printStackTrace();
					return;
				}
			}
		});
		
		btnUpdate2.addSelectionListener(new SelectionAdapter() {
			@SuppressWarnings("deprecation")
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				String fn = textPersonFN.getText().trim();
				if (fn.isEmpty()) {
					errorMsgBox("Error", "First name must not be empty");
					return;
				}
				String mn = textPersonMN.getText().trim();
				if (mn.isEmpty()) mn = null;
				String ln = textPersonLN.getText().trim();
				if (ln.isEmpty()) {
					errorMsgBox("Error", "Last name must not be empty");
					return;
				}
				String nn = textPersonNN.getText().trim();
				if (nn.isEmpty()) nn = null;
				String rn = textPersonRN.getText().trim();
				if (rn.isEmpty()) rn = null;
				Integer person_id = (Integer)textPersonFN.getData();
				
				if (person_id == null) {
					return;
				}
				
				Date bd = null;
				if (birthDate.getYear() > 1800) {
					bd = new Date(birthDate.getYear() - 1900, birthDate.getMonth(), birthDate.getDay());
				}
				Date dd = null;
				if (!btnAlive.getSelection()) {
					dd = new Date(deathDate.getYear() - 1900, deathDate.getMonth(), deathDate.getDay());
				}
				String gender = btnMale.getSelection() ? "m" : "f";

				try {
					updatePersonDetails.update(fn, mn, ln, nn, rn, bd, dd, gender, person_id);
					schema.commit();
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		});
		
		btnAddNew2.addSelectionListener(new SelectionAdapter() {
			@SuppressWarnings("deprecation")
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				String fn = textPersonFN.getText().trim();
				if (fn.isEmpty()) {
					errorMsgBox("Error", "First name must not be empty");
					return;
				}
				String mn = textPersonMN.getText().trim();
				if (mn.isEmpty()) mn = null;
				String ln = textPersonLN.getText().trim();
				if (ln.isEmpty()) {
					errorMsgBox("Error", "Last name must not be empty");
					return;
				}
				String nn = textPersonNN.getText().trim();
				if (nn.isEmpty()) nn = null;
				String rn = textPersonRN.getText().trim();
				if (rn.isEmpty()) rn = null;
				Date bd = null;
				if (birthDate.getYear() > 1800) {
					bd = new Date(birthDate.getYear() - 1900, birthDate.getMonth(), birthDate.getDay());
				}
				Date dd = null;
				if (!btnAlive.getSelection()) {
					dd = new Date(deathDate.getYear() - 1900, deathDate.getMonth(), deathDate.getDay());
				}
				String gender = btnMale.getSelection() ? "m" : "f";
				
				String imdb_name;
				if (mn == null) {
					imdb_name = ln + ", " + fn;
				} else {
					imdb_name = ln + ", " + fn + " " + mn;
				}
				
				try {
					try {
						insertPersonDetails.insert(imdb_name, fn, mn, ln, nn, rn, bd, dd, gender);
					} catch (SQLException ex) {
						errorMsgBox("Error", "Person already exists");
						return;
					}
					
					schema.commit();
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}				
			}
		});
		
		
	}

	@Override
	protected void checkSubclass() {
		// Disable the check that prevents subclassing of SWT components
	}
}
