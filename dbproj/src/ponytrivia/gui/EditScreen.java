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
	
	protected SimpleQuery findActorMovies;
	protected SimpleQuery findDirectorMovies;
	
	protected SimpleInsert insertRole;
	protected SimpleInsert insertDirector;
	protected PreparedStatement deleteRole;
	protected PreparedStatement deleteDirector;
	
	private Text textPersonNameSearch;
	private Text textPersonFN;
	private Text textPersonMN;
	private Text textPersonLN;
	private Text textPersonNN;
	private Text textPersonRN;
	private Text textActPersonName;
	private Text textDirMovieName;
	private Text textDirPersonName;
	private Text textActSearchMovie;
	private Text textActCharName;
	private Spinner textActCredPos;
	private Table tableActPlaysIn;
	private Table tableDirectedMovies;

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
				if (EditScreen.this.isDisposed()) {
					return;
				}
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
				if (EditScreen.this.isDisposed()) {
					return;
				}
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
		composite_2.setLayout(new FillLayout(SWT.HORIZONTAL));
		
		Composite composite_4 = new Composite(composite_2, SWT.NONE);
		FillLayout fl_composite_4 = new FillLayout(SWT.HORIZONTAL);
		fl_composite_4.marginWidth = 5;
		fl_composite_4.marginHeight = 5;
		fl_composite_4.spacing = 5;
		composite_4.setLayout(fl_composite_4);
		
		Group grpLocateActor = new Group(composite_4, SWT.NONE);
		grpLocateActor.setText("Locate Actor");
		grpLocateActor.setLayout(new FormLayout());
		
		Label lblPersonName = new Label(grpLocateActor, SWT.NONE);
		FormData fd_lblPersonName = new FormData();
		fd_lblPersonName.top = new FormAttachment(0, 10);
		fd_lblPersonName.left = new FormAttachment(0, 10);
		lblPersonName.setLayoutData(fd_lblPersonName);
		lblPersonName.setText("Person name:");
		
		textActPersonName = new Text(grpLocateActor, SWT.BORDER);
		FormData fd_textActPersonName = new FormData();
		fd_textActPersonName.top = new FormAttachment(lblPersonName, -3, SWT.TOP);
		fd_textActPersonName.left = new FormAttachment(lblPersonName, 6);
		fd_textActPersonName.right = new FormAttachment(100, -10);
		textActPersonName.setLayoutData(fd_textActPersonName);
		
		final List listActPeople = new List(grpLocateActor, SWT.BORDER);
		FormData fd_listActPeople = new FormData();
		fd_listActPeople.top = new FormAttachment(lblPersonName, 6);
		fd_listActPeople.left = new FormAttachment(lblPersonName, 0, SWT.LEFT);
		fd_listActPeople.bottom = new FormAttachment(100, -10);
		fd_listActPeople.right = new FormAttachment(100, -10);
		listActPeople.setLayoutData(fd_listActPeople);
		
		Composite composite_6 = new Composite(composite_2, SWT.NONE);
		FillLayout fl_composite_6 = new FillLayout(SWT.VERTICAL);
		fl_composite_6.marginWidth = 5;
		fl_composite_6.marginHeight = 5;
		fl_composite_6.spacing = 5;
		composite_6.setLayout(fl_composite_6);
		
		Group grpPlaysIn = new Group(composite_6, SWT.NONE);
		grpPlaysIn.setText("Plays In");
		grpPlaysIn.setLayout(new FormLayout());
		
		Button btnRemoveSelectedRole = new Button(grpPlaysIn, SWT.NONE);

		FormData fd_btnRemoveSelectedRole = new FormData();
		fd_btnRemoveSelectedRole.bottom = new FormAttachment(100, -10);
		fd_btnRemoveSelectedRole.right = new FormAttachment(100, -10);
		btnRemoveSelectedRole.setLayoutData(fd_btnRemoveSelectedRole);
		btnRemoveSelectedRole.setText("Remove Selected Roles");
		
		tableActPlaysIn = new Table(grpPlaysIn, SWT.BORDER | SWT.CHECK | SWT.FULL_SELECTION);
		FormData fd_tableActPlaysIn = new FormData();
		fd_tableActPlaysIn.top = new FormAttachment(0, 10);
		fd_tableActPlaysIn.left = new FormAttachment(0, 10);
		fd_tableActPlaysIn.right = new FormAttachment(100, -10);
		fd_tableActPlaysIn.bottom = new FormAttachment(btnRemoveSelectedRole, -10);
		tableActPlaysIn.setLayoutData(fd_tableActPlaysIn);
		tableActPlaysIn.setHeaderVisible(true);
		tableActPlaysIn.setLinesVisible(true);
		
		TableColumn tblclmnMovie = new TableColumn(tableActPlaysIn, SWT.NONE);
		tblclmnMovie.setWidth(133);
		tblclmnMovie.setText("Movie");
		
		TableColumn tblclmnCharacter = new TableColumn(tableActPlaysIn, SWT.NONE);
		tblclmnCharacter.setWidth(88);
		tblclmnCharacter.setText("Character");
		
		TableColumn tblclmnCreditPosition = new TableColumn(tableActPlaysIn, SWT.NONE);
		tblclmnCreditPosition.setWidth(91);
		tblclmnCreditPosition.setText("Credit Position");
		
		Group grpEditRoles = new Group(composite_6, SWT.NONE);
		grpEditRoles.setText("Edit Roles");
		grpEditRoles.setLayout(new FormLayout());
		
		Label lblLocateMovie = new Label(grpEditRoles, SWT.NONE);
		FormData fd_lblLocateMovie = new FormData();
		fd_lblLocateMovie.top = new FormAttachment(0, 10);
		fd_lblLocateMovie.left = new FormAttachment(0, 10);
		lblLocateMovie.setLayoutData(fd_lblLocateMovie);
		lblLocateMovie.setText("Locate movie");
		
		textActSearchMovie = new Text(grpEditRoles, SWT.BORDER);
		FormData fd_textActSearchMovie = new FormData();
		fd_textActSearchMovie.top = new FormAttachment(lblLocateMovie, -3, SWT.TOP);
		fd_textActSearchMovie.left = new FormAttachment(lblLocateMovie, 28);
		fd_textActSearchMovie.right = new FormAttachment(100, -10);
		textActSearchMovie.setLayoutData(fd_textActSearchMovie);
		
		final List listActAddMovie = new List(grpEditRoles, SWT.BORDER);
		FormData fd_listActAddMovie = new FormData();
		fd_listActAddMovie.top = new FormAttachment(lblLocateMovie, 6);
		fd_listActAddMovie.left = new FormAttachment(lblLocateMovie, 0, SWT.LEFT);
		fd_listActAddMovie.right = new FormAttachment(100, -10);
		listActAddMovie.setLayoutData(fd_listActAddMovie);
		
		Label lblCreditPosition = new Label(grpEditRoles, SWT.NONE);
		FormData fd_lblCreditPosition = new FormData();
		fd_lblCreditPosition.bottom = new FormAttachment(100, -10);
		fd_lblCreditPosition.left = new FormAttachment(lblLocateMovie, 0, SWT.LEFT);
		lblCreditPosition.setLayoutData(fd_lblCreditPosition);
		lblCreditPosition.setText("Credit position");
		
		Label lblCharacterName = new Label(grpEditRoles, SWT.NONE);
		FormData fd_lblCharacterName = new FormData();
		fd_lblCharacterName.bottom = new FormAttachment(lblCreditPosition, -6);
		fd_lblCharacterName.left = new FormAttachment(lblLocateMovie, 0, SWT.LEFT);
		lblCharacterName.setLayoutData(fd_lblCharacterName);
		lblCharacterName.setText("Character name");
		fd_listActAddMovie.bottom = new FormAttachment(lblCharacterName, -10);
		
		textActCharName = new Text(grpEditRoles, SWT.BORDER);
		FormData fd_textActCharName = new FormData();
		fd_textActCharName.top = new FormAttachment(lblCharacterName, -3, SWT.TOP);
		fd_textActCharName.left = new FormAttachment(textActSearchMovie, 0, SWT.LEFT);
		fd_textActCharName.right = new FormAttachment(100, -10);
		textActCharName.setLayoutData(fd_textActCharName);
		
		Button btnAddupdateRole = new Button(grpEditRoles, SWT.NONE);

		FormData fd_btnAddupdateRole = new FormData();
		fd_btnAddupdateRole.top = new FormAttachment(lblCreditPosition, 0, SWT.TOP);
		fd_btnAddupdateRole.right = new FormAttachment(100, -10);
		btnAddupdateRole.setLayoutData(fd_btnAddupdateRole);
		btnAddupdateRole.setText("Add/Update Role");
		
		textActCredPos = new Spinner(grpEditRoles, SWT.BORDER);
		FormData fd_textActCredPos = new FormData();
		fd_textActCredPos.top = new FormAttachment(lblCreditPosition, 0, SWT.TOP);
		fd_textActCredPos.left = new FormAttachment(textActSearchMovie, 0, SWT.LEFT);
		textActCredPos.setLayoutData(fd_textActCredPos);

		TabItem tbtmDirectors = new TabItem(tabFolder, SWT.NONE);
		tbtmDirectors.setText("Directors");
		
		Composite composite_3 = new Composite(tabFolder, SWT.NONE);
		tbtmDirectors.setControl(composite_3);
		FillLayout fl_composite_3 = new FillLayout(SWT.HORIZONTAL);
		fl_composite_3.spacing = 5;
		fl_composite_3.marginWidth = 5;
		fl_composite_3.marginHeight = 5;
		composite_3.setLayout(fl_composite_3);
		
		Group grpLocateDirector = new Group(composite_3, SWT.NONE);
		grpLocateDirector.setText("Locate Director");
		grpLocateDirector.setLayout(new FormLayout());
		
		Label label_1 = new Label(grpLocateDirector, SWT.NONE);
		label_1.setText("Person name:");
		FormData fd_label_1 = new FormData();
		fd_label_1.top = new FormAttachment(0, 10);
		fd_label_1.left = new FormAttachment(0, 10);
		label_1.setLayoutData(fd_label_1);
		
		textDirPersonName = new Text(grpLocateDirector, SWT.BORDER);
		FormData fd_textDirPersonName = new FormData();
		fd_textDirPersonName.top = new FormAttachment(label_1, -3, SWT.TOP);
		fd_textDirPersonName.right = new FormAttachment(100, -10);
		fd_textDirPersonName.left = new FormAttachment(label_1, 6);
		textDirPersonName.setLayoutData(fd_textDirPersonName);
		
		final List listDirPeople = new List(grpLocateDirector, SWT.BORDER);
		FormData fd_listDirPeople = new FormData();
		fd_listDirPeople.bottom = new FormAttachment(100, -10);
		fd_listDirPeople.top = new FormAttachment(label_1, 6);
		fd_listDirPeople.right = new FormAttachment(100, -10);
		fd_listDirPeople.left = new FormAttachment(label_1, 0, SWT.LEFT);
		listDirPeople.setLayoutData(fd_listDirPeople);
		
		Composite composite_5 = new Composite(composite_3, SWT.NONE);
		FillLayout fl_composite_5 = new FillLayout(SWT.VERTICAL);
		fl_composite_5.marginHeight = 5;
		fl_composite_5.spacing = 5;
		fl_composite_5.marginWidth = 5;
		composite_5.setLayout(fl_composite_5);
		
		Group grpDirected = new Group(composite_5, SWT.NONE);
		grpDirected.setText("Directed");
		grpDirected.setLayout(new FormLayout());
		
		Button btnRemoveSelectedMovies = new Button(grpDirected, SWT.NONE);
		FormData fd_btnRemoveSelectedMovies = new FormData();
		fd_btnRemoveSelectedMovies.bottom = new FormAttachment(100, -10);
		fd_btnRemoveSelectedMovies.right = new FormAttachment(100, -10);
		btnRemoveSelectedMovies.setLayoutData(fd_btnRemoveSelectedMovies);
		btnRemoveSelectedMovies.setText("Remove Selected Movies");
		
		tableDirectedMovies = new Table(grpDirected, SWT.BORDER | SWT.CHECK | SWT.FULL_SELECTION);
		FormData fd_tableDirectedMovies = new FormData();
		fd_tableDirectedMovies.top = new FormAttachment(0, 10);
		fd_tableDirectedMovies.left = new FormAttachment(0, 10);
		fd_tableDirectedMovies.right = new FormAttachment(100, -10);
		fd_tableDirectedMovies.bottom = new FormAttachment(btnRemoveSelectedMovies, -10);
		tableDirectedMovies.setLayoutData(fd_tableDirectedMovies);
		tableDirectedMovies.setHeaderVisible(true);
		tableDirectedMovies.setLinesVisible(true);
		
		TableColumn tblclmnMovie_1 = new TableColumn(tableDirectedMovies, SWT.NONE);
		tblclmnMovie_1.setWidth(292);
		tblclmnMovie_1.setText("Movie");
		
		Group group_1 = new Group(composite_5, SWT.NONE);
		group_1.setText("Locate Movie");
		group_1.setLayout(new FormLayout());
		
		Label label = new Label(group_1, SWT.NONE);
		label.setText("Movie name:");
		FormData fd_label = new FormData();
		fd_label.top = new FormAttachment(0, 10);
		fd_label.left = new FormAttachment(0, 10);
		label.setLayoutData(fd_label);
		
		textDirMovieName = new Text(group_1, SWT.BORDER);
		FormData fd_textDirMovieName = new FormData();
		fd_textDirMovieName.top = new FormAttachment(label, -3, SWT.TOP);
		fd_textDirMovieName.right = new FormAttachment(100, -10);
		fd_textDirMovieName.left = new FormAttachment(label, 6);
		textDirMovieName.setLayoutData(fd_textDirMovieName);
		
		final List listDirMovies = new List(group_1, SWT.BORDER);
		FormData fd_listDirMovies = new FormData();
		fd_listDirMovies.top = new FormAttachment(label, 6);
		fd_listDirMovies.right = new FormAttachment(100, -10);
		fd_listDirMovies.left = new FormAttachment(0, 10);
		listDirMovies.setLayoutData(fd_listDirMovies);
		
		Button btnAddMovie = new Button(group_1, SWT.NONE);

		FormData fd_btnAddMovie = new FormData();
		fd_btnAddMovie.bottom = new FormAttachment(100, -10);
		fd_btnAddMovie.right = new FormAttachment(100, -10);
		btnAddMovie.setLayoutData(fd_btnAddMovie);
		btnAddMovie.setText("Add Movie");
		fd_listDirMovies.bottom = new FormAttachment(btnAddMovie, -10);
		
		textDirPersonName.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				if (textDirPersonName.getText().length() < 3) {
					return;
				}
				String[] names = textDirPersonName.getText().split(" ");
				
				ResultSet rs;
				try {
					if (names.length == 1) {
						rs = findPerson1.query(names[0] + "%");
					}
					else {
						rs = findPerson2.query(names[0] + "%", names[1] + "%");
					}
					
					listDirPeople.removeAll();
					personIdMapping.clear();
					int i = 0;
					while (rs.next()) {
						listDirPeople.add(rs.getString(1));
						personIdMapping.put(i, rs.getInt(2));
						i++;
					}
					rs.close();
				} catch (SQLException e) {
					e.printStackTrace();
				}
			}
		});	

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
			
			findActorMovies = schema.createQuery("M.movie_id, M.name, R.char_name, R.credit_pos", 
					"Movies as M, Roles as R", "R.actor = ? AND R.movie = M.movie_id", "M.name ASC");
			findDirectorMovies = schema.createQuery("M.movie_id, M.name", "Movies as M, MovieDirectors as MD",
					"MD.director = ? AND MD.movie = M.movie_id", "M.name ASC");
			
			insertRole = schema.createInsert("Roles", true, "actor", "movie", "char_name", "credit_pos"); 
			insertDirector = schema.createInsert("MovieDirectors", true, "director", "movie"); 
			deleteRole = schema.prepareStatement("DELETE FROM Roles WHERE actor = ? AND movie = ?");
			deleteDirector = schema.prepareStatement("DELETE FROM MovieDirectors WHERE director = ? AND movie= ?");
			
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
				if (EditScreen.this.isDisposed()) {
					return;
				}
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
					MainScreen.rebuildPopularTables = true;
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		});
		
		btnUpdate.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
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
					MainScreen.rebuildPopularTables = true;
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
				if (EditScreen.this.isDisposed()) {
					return;
				}
				int selind = list.getSelectionIndex();
				if (selind < 0) {
					return;
				}
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
				if (EditScreen.this.isDisposed()) {
					return;
				}
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
				if (EditScreen.this.isDisposed()) {
					return;
				}
				int selind = list_1.getSelectionIndex();
				if (selind < 0) {
					return;
				}
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
				if (EditScreen.this.isDisposed()) {
					return;
				}
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
					MainScreen.rebuildPopularTables = true;
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
				if (EditScreen.this.isDisposed()) {
					return;
				}
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
					MainScreen.rebuildPopularTables = true;
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}				
			}
		});

		
		textActPersonName.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				if (textActPersonName.getText().length() < 3) {
					return;
				}
				String[] names = textActPersonName.getText().split(" ");
				
				ResultSet rs;
				try {
					if (names.length == 1) {
						rs = findPerson1.query(names[0] + "%");
					}
					else {
						rs = findPerson2.query(names[0] + "%", names[1] + "%");
					}
					
					listActPeople.removeAll();
					personIdMapping.clear();
					int i = 0;
					while (rs.next()) {
						listActPeople.add(rs.getString(1));
						personIdMapping.put(i, rs.getInt(2));
						i++;
					}
					rs.close();
				} catch (SQLException e) {
					e.printStackTrace();
				}
			}
		});

		final SelectionListener updatePlayedInTable = new SelectionListener() {
			@Override
			public void widgetDefaultSelected(SelectionEvent arg0) {
			}

			@Override
			public void widgetSelected(SelectionEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				int selind = listActPeople.getSelectionIndex();
				if (selind < 0) {
					return;
				}
				int person_id = personIdMapping.get(selind);
				
				try {
					ResultSet rs = findActorMovies.query(person_id);
					tableActPlaysIn.removeAll();
					tableActPlaysIn.setData(person_id);
					while (rs.next()) {
						TableItem ti = new TableItem(tableActPlaysIn, SWT.NONE);
						ti.setData(rs.getInt(1));
						ti.setText(0, rs.getString(2));
						ti.setText(1, rs.getString(3) == null ? "" : rs.getString(3));
						ti.setText(2, rs.getInt(4) <= 0 ? "" : new Integer(rs.getInt(4)).toString());
					}
				} catch (SQLException e) {
					e.printStackTrace();
					return;
				}
			}
		};
		
		listActPeople.addSelectionListener(updatePlayedInTable);

		btnRemoveSelectedRole.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				boolean found = false;
				int person_id = (Integer)tableActPlaysIn.getData();
				int i = -1;
				for (TableItem ti : tableActPlaysIn.getItems()) {
					i++;
					if (!ti.getChecked()) {
						continue;
					}
					found = true;
					int movie_id = (Integer)ti.getData();
					try {
						deleteRole.setInt(1, person_id);
						deleteRole.setInt(2, movie_id);
						deleteRole.executeUpdate();
					} catch (SQLException e) {
						e.printStackTrace();
					}
					tableActPlaysIn.remove(i);
				}
				if (found) {
					try {
						schema.commit();
						MainScreen.rebuildPopularTables = true;
					} catch (SQLException e) {
						e.printStackTrace();
					}
				} else {
					errorMsgBox("Error", "No movies selected");
				}
			}
		});
		
		textActSearchMovie.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				if (textActSearchMovie.getText().length() < 3) {
					return;
				}
				String movie_name = "%"+ textActSearchMovie.getText() + "%";
				listActAddMovie.removeAll();
				movieIdMapping.clear();
				try {
					ResultSet rs = findMovie.query(movie_name);
					int i = 0;
					while (rs.next()) {
						listActAddMovie.add(rs.getString(1));
						movieIdMapping.put(i, rs.getInt(2));
						i++;
					}
				} catch (SQLException e) {
					e.printStackTrace();
				}
			}
		});

		btnAddupdateRole.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				int selind = listActAddMovie.getSelectionIndex();
				if (selind < 0) {
					errorMsgBox("Error", "Select a movie first");
					return;
				}
				int movie_id = movieIdMapping.get(selind);
				String charname = textActCharName.getText().trim();
				if (charname.isEmpty()) {
					charname = null;
				}
				Integer creditpos = textActCredPos.getSelection();
				if (creditpos <= 0) {
					creditpos = null;
				}
				int person_id = (Integer)tableActPlaysIn.getData();
				try {
					deleteRole.setInt(1, person_id);
					deleteRole.setInt(2, movie_id);
					deleteRole.executeUpdate();
					insertRole.insert(person_id, movie_id, charname, creditpos);
					schema.commit();
					MainScreen.rebuildPopularTables = true;
				} catch (SQLException e) {
					e.printStackTrace();
				}
				updatePlayedInTable.widgetSelected(null);
			}
		});
				
		textDirMovieName.addModifyListener(new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				if (textDirMovieName.getText().length() < 3) {
					return;
				}
				String movie_name = "%"+ textDirMovieName.getText() + "%";
				listDirMovies.removeAll();
				movieIdMapping.clear();
				try {
					ResultSet rs = findMovie.query(movie_name);
					int i = 0;
					while (rs.next()) {
						listDirMovies.add(rs.getString(1));
						movieIdMapping.put(i, rs.getInt(2));
						i++;
					}
				} catch (SQLException e) {
					e.printStackTrace();
				}
			}
		});
		
		final SelectionListener updateDirectedTable = new SelectionListener() {
			@Override
			public void widgetDefaultSelected(SelectionEvent arg0) {
			}

			@Override
			public void widgetSelected(SelectionEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				int selind = listDirPeople.getSelectionIndex();
				if (selind < 0) {
					return;
				}
				int person_id = personIdMapping.get(selind);
				
				try {
					ResultSet rs = findDirectorMovies.query(person_id);
					tableDirectedMovies.removeAll();
					tableDirectedMovies.setData(person_id);
					while (rs.next()) {
						TableItem ti = new TableItem(tableDirectedMovies, SWT.NONE);
						ti.setData(rs.getInt(1));
						ti.setText(0, rs.getString(2));
					}
				} catch (SQLException e) {
					e.printStackTrace();
					return;
				}
			}
		};
		
		listDirPeople.addSelectionListener(updateDirectedTable);		
		
		btnRemoveSelectedMovies.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				boolean found = false;
				int person_id = (Integer)tableDirectedMovies.getData();
				int i = -1;
				for (TableItem ti : tableDirectedMovies.getItems()) {
					i++;
					if (!ti.getChecked()) {
						continue;
					}
					found = true;
					int movie_id = (Integer)ti.getData();
					try {
						deleteDirector.setInt(1, person_id);
						deleteDirector.setInt(2, movie_id);
						deleteDirector.executeUpdate();
					} catch (SQLException e) {
						e.printStackTrace();
					}
					tableDirectedMovies.remove(i);
				}
				if (found) {
					try {
						schema.commit();
						MainScreen.rebuildPopularTables = true;
					} catch (SQLException e) {
						e.printStackTrace();
					}
				} else {
					errorMsgBox("Error", "No movies selected");
				}
			}
		});
		
		btnAddMovie.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				if (EditScreen.this.isDisposed()) {
					return;
				}
				int selind = listDirMovies.getSelectionIndex();
				if (selind < 0) {
					errorMsgBox("Error", "Select a movie first");
					return;
				}
				int movie_id = movieIdMapping.get(selind);
				int person_id = (Integer)tableDirectedMovies.getData();
				try {
					insertDirector.insert(person_id, movie_id);
					schema.commit();
					MainScreen.rebuildPopularTables = true;
				} catch (SQLException e) {
					e.printStackTrace();
				}
				updateDirectedTable.widgetSelected(null);
			}
		});		
	}

	@Override
	protected void checkSubclass() {
		// Disable the check that prevents subclassing of SWT components
	}
}
