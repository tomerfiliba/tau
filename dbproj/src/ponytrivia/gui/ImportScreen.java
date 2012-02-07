package ponytrivia.gui;

import java.io.File;
import java.io.IOException;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.DirectoryDialog;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.events.DisposeEvent;
import org.eclipse.swt.events.DisposeListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.ProgressBar;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.wb.swt.SWTResourceManager;

import ponytrivia.db.Schema;
import ponytrivia.importer.Importer;

public class ImportScreen extends Shell {
	private Text txtDir;
	private Text text;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void run(Display display, Schema schema) {
		try {
			final ImportScreen shell = new ImportScreen(display, schema);
			shell.open();
			shell.layout();
			shell.addDisposeListener(new DisposeListener() {
				@SuppressWarnings("deprecation")
				@Override
				public void widgetDisposed(DisposeEvent arg0) {
					if (bgImporter != null && bgImporter.isAlive()) {
						bgImporter.interrupt();
						try {
							bgImporter.join(1000);
						} catch (InterruptedException e) {
							bgImporter.stop();
						}
						bgImporter = null;
					}
				}
			});
			while (!shell.isDisposed()) {
				if (!display.readAndDispatch()) {
					display.sleep();
				}
			}
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	protected void errorMsgbox(String title, String message) {
		MessageBox mb = new MessageBox(this, SWT.ICON_ERROR | SWT.OK);
        mb.setText(title);
        mb.setMessage(message);
        mb.open();
	}
	
	protected void successMsgbox(String title, String message) {
		MessageBox mb = new MessageBox(this, SWT.ICON_INFORMATION | SWT.OK);
        mb.setText(title);
        mb.setMessage(message);
        mb.open();
	}
	
	protected Schema schema;

	protected class BackgroundImporter extends Thread
	{
		protected String errorMessage = null;
		protected Importer imp;
		protected File directory;
		
		public BackgroundImporter(File directory) {
			this.directory = directory;
		}
		
		@Override
		public void run()
		{
			imp = new Importer(schema);
			try {
				imp.importLists(directory);
				schema.buildPopularTables(true);
			} catch (Exception ex) {
				ex.printStackTrace();
				errorMessage = ex.toString();
			}
		}
		
		public boolean isSuccessful() {
			return errorMessage == null;
		}
		
		public String getErrorMessage() {
			return errorMessage;
		}
		
		public Importer.ImportStatus getStatus() throws IOException {
			return imp.getStatus();
		}
		
		
	}
	
	static protected BackgroundImporter bgImporter = null;
	protected final Image imgPonyLeft = SWTResourceManager.getImage(ImportScreen.class, "/ponytrivia/gui/res/kitty2.gif");
	protected final Image imgPonyRight = SWTResourceManager.getImage(ImportScreen.class, "/ponytrivia/gui/res/kitty1.gif");

	/**
	 * Create the shell.
	 * @param display
	 */
	public ImportScreen(final Display display, Schema schema) {
		super(display, SWT.SHELL_TRIM);
		this.schema = schema;
		
		setText("Import IMDB files...");
		setSize(666, 492);
		setMinimumSize(new Point(400, 300));
		setLayout(new FormLayout());

		Label lblDirectoryOfImdb = new Label(this, SWT.NONE);
		FormData fd_lblDirectoryOfImdb = new FormData();
		fd_lblDirectoryOfImdb.top = new FormAttachment(0, 20);
		fd_lblDirectoryOfImdb.left = new FormAttachment(0, 10);
		lblDirectoryOfImdb.setLayoutData(fd_lblDirectoryOfImdb);
		lblDirectoryOfImdb.setText("Directory of IMDB files:");
		
		final Button btnImport = new Button(this, SWT.NONE);
		FormData fd_btnImport = new FormData();
		fd_btnImport.bottom = new FormAttachment(lblDirectoryOfImdb, 0, SWT.BOTTOM);
		fd_btnImport.right = new FormAttachment(100, -10);
		btnImport.setLayoutData(fd_btnImport);
		btnImport.setText("Import");
		
		Button btnBrowse = new Button(this, SWT.NONE);
		btnBrowse.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				DirectoryDialog dialog = new DirectoryDialog(ImportScreen.this);
				if (!txtDir.getText().isEmpty()) {
					dialog.setFilterPath(txtDir.getText());
				}
				String dir = dialog.open();
				if (dir != null) {
					txtDir.setText(dir);
				}
			}
		});
		FormData fd_btnBrowse = new FormData();
		fd_btnBrowse.top = new FormAttachment(btnImport, 0, SWT.TOP);
		fd_btnBrowse.right = new FormAttachment(btnImport, -6);
		btnBrowse.setLayoutData(fd_btnBrowse);
		btnBrowse.setText("...");
		
		txtDir = new Text(this, SWT.BORDER);
		FormData fd_text = new FormData();
		fd_text.right = new FormAttachment(btnBrowse, -6);
		fd_text.top = new FormAttachment(0, 14);
		fd_text.left = new FormAttachment(lblDirectoryOfImdb, 6);
		txtDir.setLayoutData(fd_text);
		
		Group grpProgress = new Group(this, SWT.NONE);
		grpProgress.setText("Progress");
		grpProgress.setLayout(new FormLayout());
		FormData fd_grpProgress = new FormData();
		fd_grpProgress.bottom = new FormAttachment(100, -10);
		fd_grpProgress.left = new FormAttachment(0, 10);
		fd_grpProgress.top = new FormAttachment(lblDirectoryOfImdb, 6);
		fd_grpProgress.right = new FormAttachment(btnImport, 0, SWT.RIGHT);
		grpProgress.setLayoutData(fd_grpProgress);
		
		Label lblCurrentFile = new Label(grpProgress, SWT.NONE);
		FormData fd_lblCurrentFile = new FormData();
		fd_lblCurrentFile.top = new FormAttachment(0, 10);
		fd_lblCurrentFile.left = new FormAttachment(0, 10);
		lblCurrentFile.setLayoutData(fd_lblCurrentFile);
		lblCurrentFile.setText("Current file:");
		
		text = new Text(grpProgress, SWT.BORDER | SWT.READ_ONLY);
		text.setEditable(false);
		FormData fd_text2 = new FormData();
		fd_text2.top = new FormAttachment(lblCurrentFile, -3, SWT.TOP);
		fd_text2.left = new FormAttachment(lblCurrentFile, 6);
		fd_text2.right = new FormAttachment(100, -19);
		text.setLayoutData(fd_text2);
		
		Label lblProgess = new Label(grpProgress, SWT.NONE);
		FormData fd_lblProgess = new FormData();
		fd_lblProgess.top = new FormAttachment(lblCurrentFile, 18);
		fd_lblProgess.left = new FormAttachment(lblCurrentFile, 0, SWT.LEFT);
		lblProgess.setLayoutData(fd_lblProgess);
		lblProgess.setText("Progess:");
		
		final ProgressBar progressBar = new ProgressBar(grpProgress, SWT.SMOOTH);
		FormData fd_progressBar = new FormData();
		fd_progressBar.left = new FormAttachment(lblProgess, 31);
		fd_progressBar.right = new FormAttachment(100, -25);
		fd_progressBar.top = new FormAttachment(lblProgess, 0, SWT.TOP);
		progressBar.setLayoutData(fd_progressBar);
		progressBar.setMinimum(0);
		
		Composite composite = new Composite(grpProgress, SWT.NONE);
		FormData fd_composite = new FormData();
		fd_composite.bottom = new FormAttachment(progressBar, 133, SWT.BOTTOM);
		fd_composite.top = new FormAttachment(progressBar, 18);
		fd_composite.left = new FormAttachment(text, 0, SWT.LEFT);
		fd_composite.right = new FormAttachment(100, -65);
		composite.setLayoutData(fd_composite);
		
		final Label lblPony = new Label(composite, SWT.NONE);
		lblPony.setImage(imgPonyLeft);
		lblPony.setBounds(10, 10, 84, 95);
		lblPony.setVisible(false);
		
		btnImport.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				String dir = txtDir.getText();
				if (dir.isEmpty()) {
					errorMsgbox("Invalid Directory", "You must choose a valid directory");
					return;
				}
				File f = new File(dir);
				if (!f.exists() || !f.isDirectory()) {
					errorMsgbox("Invalid Directory", "The given path is not an existing directory");
					return;
				}
				String[] requiredFiles = {"actors.list", "actresses.list", "biographies.list", 
						"directors.list", "genres.list", "movies.list", "ratings.list"};
				for (String fn : requiredFiles) {
					File f2 = new File(f, fn);
					if (!f2.exists() || !f2.isFile()) {
						f2 = new File(f, fn + ".gz");
						if (!f2.exists() || !f2.isFile()) {
							errorMsgbox("Invalid Directory", "Required file " + fn + "[.gz] is missing\n" +
									"from the given directory");
							return;
						}
					}
				}
				btnImport.setEnabled(false);
				if (bgImporter != null && bgImporter.isAlive()) {
					errorMsgbox("Internal Error", "An import is already in progress, I'm confused");
				}
				bgImporter = new BackgroundImporter(f);
				bgImporter.setDaemon(true);
				bgImporter.start();
				
				class AnimatePony implements Runnable
				{
					int delta = 2;
					int counter = 0;
					@Override
					public void run() {
						if (ImportScreen.this.isDisposed()) {
							return;
						}
						counter++;
						lblPony.setVisible(true);
						Point p = lblPony.getLocation();
						p.x += delta;
						if (p.x < 2) {
							delta = -delta;
							lblPony.setImage(imgPonyLeft);
						}
						if (p.x >= lblPony.getParent().getBounds().width - lblPony.getBounds().width - 5) {
							delta = -delta;
							lblPony.setImage(imgPonyRight);
						}
						p.x += delta;
						lblPony.setLocation(p.x, p.y);
						boolean stillRunning = true;
						
						if (counter % 10 == 0) {
							if (bgImporter.isAlive()) {
								Importer.ImportStatus stat = null;
								try {
									stat = bgImporter.getStatus();
								} catch (IOException e) {
									e.printStackTrace();
								}
								text.setText(stat.filename);
								progressBar.setSelection((int)stat.position);
								progressBar.setMaximum((int)stat.size);
							}
							else {
								if (bgImporter.isSuccessful()) {
									successMsgbox("Success", "The IMDB tables have been successfully imported");
									ImportScreen.this.close();
								}
								else {
									errorMsgbox("Import Error", "The import has failed:\n" + bgImporter.getErrorMessage());
								}
								stillRunning = false;
							}
						}

						if (stillRunning) {
							display.timerExec(50, this);
						}
						else {
							lblPony.setVisible(false);
							btnImport.setEnabled(true);
							bgImporter = null;
						}
					}
				}
				display.asyncExec(new AnimatePony());
			}
		});

	}

	@Override
	protected void checkSubclass() {
		// Disable the check that prevents subclassing of SWT components
	}
}
