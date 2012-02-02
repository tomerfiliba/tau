package ponytrivia.gui;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.TabFolder;
import org.eclipse.swt.widgets.TabItem;
import org.eclipse.swt.widgets.List;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.layout.GridData;
import swing2swt.layout.BoxLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.custom.SashForm;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.DateTime;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;

public class EditDB extends Shell {
	private Text text;
	private Text text_1;
	private Text text_2;
	private Text text_3;
	private Text text_4;
	private Text txtGenres;
	private Text text_5;
	private Text text_6;
	private Text text_7;
	private Text text_8;
	private Text text_9;
	private Text text_10;
	private Text text_11;
	private Text text_12;
	private Text text_13;
	private Text text_14;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void main(String args[]) {
		try {
			Display display = Display.getDefault();
			EditDB shell = new EditDB(display);
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

	/**
	 * Create the shell.
	 * @param display
	 */
	public EditDB(Display display) {
		super(display, SWT.SHELL_TRIM);
		setLayout(new FillLayout(SWT.HORIZONTAL));
		
		TabFolder tabFolder = new TabFolder(this, SWT.NONE);
		
		TabItem tbtmMovies = new TabItem(tabFolder, SWT.NONE);
		tbtmMovies.setText("Movies");
		
		Composite composite = new Composite(tabFolder, SWT.NONE);
		tbtmMovies.setControl(composite);
		
		Label lblName = new Label(composite, SWT.NONE);
		lblName.setBounds(10, 10, 80, 25);
		lblName.setText("Name");
		
		text = new Text(composite, SWT.BORDER);
		text.setText("");
		text.setBounds(96, 10, 228, 24);
		
		List list = new List(composite, SWT.BORDER | SWT.MULTI);
		list.setToolTipText("foobar");
		list.setBounds(10, 41, 314, 163);
		
		Group grpDetails = new Group(composite, SWT.NONE);
		grpDetails.setText("Details");
		grpDetails.setBounds(10, 210, 638, 201);
		
		Label lblName_1 = new Label(grpDetails, SWT.NONE);
		lblName_1.setBounds(10, 23, 66, 18);
		lblName_1.setText("Name:");
		
		Label lblYear = new Label(grpDetails, SWT.NONE);
		lblYear.setBounds(10, 57, 66, 18);
		lblYear.setText("Year");
		
		Label lblRating = new Label(grpDetails, SWT.NONE);
		lblRating.setBounds(10, 91, 66, 18);
		lblRating.setText("Rating");
		
		Label lblVotes = new Label(grpDetails, SWT.NONE);
		lblVotes.setBounds(10, 127, 66, 18);
		lblVotes.setText("Votes");
		
		Label lblType = new Label(grpDetails, SWT.NONE);
		lblType.setBounds(10, 157, 66, 18);
		lblType.setText("Type:");
		
		Button btnFilm = new Button(grpDetails, SWT.RADIO);
		btnFilm.setBounds(81, 157, 56, 19);
		btnFilm.setText("Film");
		
		Button btnTvShow = new Button(grpDetails, SWT.RADIO);
		btnTvShow.setBounds(144, 156, 107, 19);
		btnTvShow.setText("TV Show");
		
		text_1 = new Text(grpDetails, SWT.BORDER);
		text_1.setBounds(81, 20, 78, 24);
		
		text_2 = new Text(grpDetails, SWT.BORDER);
		text_2.setBounds(82, 51, 78, 24);
		
		text_3 = new Text(grpDetails, SWT.BORDER);
		text_3.setBounds(82, 91, 78, 24);
		
		text_4 = new Text(grpDetails, SWT.BORDER);
		text_4.setBounds(81, 121, 78, 24);
		
		txtGenres = new Text(grpDetails, SWT.BORDER);
		txtGenres.setText("Genres:");
		txtGenres.setBounds(267, 23, 78, 24);
		
		List list_1 = new List(grpDetails, SWT.BORDER);
		list_1.setBounds(351, 23, 148, 152);
		
		Button btnNewButton = new Button(grpDetails, SWT.NONE);
		btnNewButton.setBounds(521, 147, 107, 28);
		btnNewButton.setText("Update Details");
		
		Button btnAdd = new Button(grpDetails, SWT.NONE);
		btnAdd.setBounds(521, 107, 107, 28);
		btnAdd.setText("Add New");
		
		TabItem tbtmPeople = new TabItem(tabFolder, SWT.NONE);
		tbtmPeople.setText("People");
		
		Composite composite_1 = new Composite(tabFolder, SWT.NONE);
		tbtmPeople.setControl(composite_1);
		
		Label lblName_2 = new Label(composite_1, SWT.NONE);
		lblName_2.setBounds(10, 10, 66, 18);
		lblName_2.setText("Name");
		
		text_5 = new Text(composite_1, SWT.BORDER);
		text_5.setBounds(82, 10, 170, 24);
		
		List list_2 = new List(composite_1, SWT.BORDER);
		list_2.setBounds(10, 43, 242, 161);
		
		Group grpDetails_1 = new Group(composite_1, SWT.NONE);
		grpDetails_1.setText("Details");
		grpDetails_1.setBounds(10, 210, 719, 219);
		
		Label lblFirstName = new Label(grpDetails_1, SWT.NONE);
		lblFirstName.setBounds(10, 25, 79, 18);
		lblFirstName.setText("First name:");
		
		text_6 = new Text(grpDetails_1, SWT.BORDER);
		text_6.setBounds(95, 19, 78, 24);
		
		Label lblLastName = new Label(grpDetails_1, SWT.NONE);
		lblLastName.setBounds(10, 55, 79, 18);
		lblLastName.setText("Last name");
		
		Label lblNickName = new Label(grpDetails_1, SWT.NONE);
		lblNickName.setBounds(10, 88, 79, 19);
		lblNickName.setText("Nick name");
		
		text_7 = new Text(grpDetails_1, SWT.BORDER);
		text_7.setBounds(95, 49, 78, 24);
		
		text_8 = new Text(grpDetails_1, SWT.BORDER);
		text_8.setBounds(95, 83, 78, 24);
		
		DateTime dateTime = new DateTime(grpDetails_1, SWT.BORDER | SWT.DROP_DOWN);
		dateTime.setBounds(95, 113, 102, 28);
		
		Label lblBirthDate = new Label(grpDetails_1, SWT.NONE);
		lblBirthDate.setBounds(10, 123, 72, 18);
		lblBirthDate.setText("Birth date");
		
		Label lblDeathDate = new Label(grpDetails_1, SWT.NONE);
		lblDeathDate.setBounds(10, 156, 79, 18);
		lblDeathDate.setText("Death date");
		
		final DateTime dateTime_1 = new DateTime(grpDetails_1, SWT.BORDER | SWT.DROP_DOWN);
		dateTime_1.setEnabled(false);
		dateTime_1.setBounds(95, 146, 102, 28);
		
		final Button btnAlive = new Button(grpDetails_1, SWT.CHECK);
		btnAlive.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				dateTime_1.setEnabled(!btnAlive.getSelection());
			}
		});
		btnAlive.setSelection(true);
		btnAlive.setBounds(221, 156, 111, 19);
		btnAlive.setText("Alive");
		
		Label lblGender = new Label(grpDetails_1, SWT.NONE);
		lblGender.setBounds(352, 25, 66, 18);
		lblGender.setText("Gender:");
		
		Button btnMale = new Button(grpDetails_1, SWT.RADIO);
		btnMale.setBounds(430, 25, 72, 19);
		btnMale.setText("Male");
		
		Button btnFemale = new Button(grpDetails_1, SWT.RADIO);
		btnFemale.setBounds(508, 25, 79, 19);
		btnFemale.setText("Female");
		
		Button btnUpdateDetails = new Button(grpDetails_1, SWT.NONE);
		btnUpdateDetails.setBounds(571, 156, 138, 33);
		btnUpdateDetails.setText("Update Details");
		
		Button btnAddNew = new Button(grpDetails_1, SWT.NONE);
		btnAddNew.setBounds(571, 123, 138, 28);
		btnAddNew.setText("Add New");
		
		TabItem tbtmRoles = new TabItem(tabFolder, SWT.NONE);
		tbtmRoles.setText("Roles");
		
		Composite composite_2 = new Composite(tabFolder, SWT.NONE);
		tbtmRoles.setControl(composite_2);
		
		text_9 = new Text(composite_2, SWT.BORDER);
		text_9.setBounds(94, 10, 152, 24);
		
		Label lblActor = new Label(composite_2, SWT.NONE);
		lblActor.setBounds(10, 16, 66, 18);
		lblActor.setText("Actor");
		
		Label lblMovie = new Label(composite_2, SWT.NONE);
		lblMovie.setBounds(317, 13, 66, 18);
		lblMovie.setText("Movie");
		
		text_10 = new Text(composite_2, SWT.BORDER);
		text_10.setBounds(396, 10, 152, 24);
		
		List list_3 = new List(composite_2, SWT.BORDER);
		list_3.setBounds(10, 40, 236, 126);
		
		List list_4 = new List(composite_2, SWT.BORDER);
		list_4.setBounds(312, 37, 236, 129);
		
		Label lblCharacter = new Label(composite_2, SWT.NONE);
		lblCharacter.setBounds(10, 194, 97, 18);
		lblCharacter.setText("Character");
		
		text_11 = new Text(composite_2, SWT.BORDER);
		text_11.setBounds(111, 191, 155, 24);
		
		Label lblCreditPoisition = new Label(composite_2, SWT.NONE);
		lblCreditPoisition.setBounds(10, 224, 97, 18);
		lblCreditPoisition.setText("Credit poisition");
		
		text_12 = new Text(composite_2, SWT.BORDER);
		text_12.setBounds(111, 221, 155, 24);
		
		Button btnUpdateDetails_1 = new Button(composite_2, SWT.NONE);
		btnUpdateDetails_1.setBounds(420, 219, 128, 28);
		btnUpdateDetails_1.setText("Update Details");
		
		TabItem tbtmDirectors = new TabItem(tabFolder, SWT.NONE);
		tbtmDirectors.setText("Directors");
		
		Composite composite_3 = new Composite(tabFolder, SWT.NONE);
		tbtmDirectors.setControl(composite_3);
		
		Label lblDirector = new Label(composite_3, SWT.NONE);
		lblDirector.setBounds(10, 10, 66, 18);
		lblDirector.setText("Director");
		
		text_13 = new Text(composite_3, SWT.BORDER);
		text_13.setBounds(82, 7, 151, 24);
		
		Label lblMovie_1 = new Label(composite_3, SWT.NONE);
		lblMovie_1.setBounds(300, 10, 66, 18);
		lblMovie_1.setText("Movie");
		
		text_14 = new Text(composite_3, SWT.BORDER);
		text_14.setBounds(375, 7, 151, 24);
		
		List list_5 = new List(composite_3, SWT.BORDER);
		list_5.setBounds(10, 34, 223, 114);
		
		List list_6 = new List(composite_3, SWT.BORDER);
		list_6.setBounds(300, 34, 226, 114);
		
		Button btnUpdate = new Button(composite_3, SWT.NONE);
		btnUpdate.setBounds(438, 162, 88, 28);
		btnUpdate.setText("Update");
		createContents();
	}

	/**
	 * Create contents of the shell.
	 */
	protected void createContents() {
		setText("EditDB");
		setSize(765, 527);

	}

	@Override
	protected void checkSubclass() {
		// Disable the check that prevents subclassing of SWT components
	}
}
