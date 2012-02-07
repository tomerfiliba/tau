package ponytrivia.gui;

import java.sql.SQLException;
import java.util.List;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Shell;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionRegistry;

import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.widgets.ProgressBar;
import org.eclipse.swt.graphics.Point;

public class ApplyFilterScreen extends Shell {
	protected Schema schema;
	protected int minYear;
	protected int maxYear;
	protected List<Integer> genres_ids;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void run(Display display, Schema schema, int minYear, int maxYear, List<Integer> genres_ids) {
		try {
			ApplyFilterScreen shell = new ApplyFilterScreen(display, schema, minYear, maxYear, 
					genres_ids);
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
	
	QuestionRegistry qr = null;
	
	class ApplyFilterThread extends Thread 
	{
		@Override
		public void run() {
			try {
				qr = new QuestionRegistry(schema, false);
				qr.setFilter(minYear, maxYear, genres_ids);
				if (qr.numOfFilteredMovies < 100) {
					// we need at least 100 movies to play...
					qr = null;
				} else {
					qr.startBgThread();
				}
			} catch (SQLException e) {
				e.printStackTrace();
				qr = null;
			}
		}
	}

	protected void errorMsgbox(String title, String message) {
		MessageBox mb = new MessageBox(this, SWT.ICON_ERROR | SWT.OK);
        mb.setText(title);
        mb.setMessage(message);
        mb.open();
	}

	/**
	 * Create the shell.
	 * @param display
	 */
	public ApplyFilterScreen(final Display display, Schema schema, int minYear, int maxYear, 
			List<Integer> genres_ids) {
		super(display, SWT.SHELL_TRIM);
		this.schema = schema;
		this.minYear = minYear;
		this.maxYear = maxYear;
		this.genres_ids = genres_ids;
		
		setMinimumSize(new Point(400, 130));
		setLayout(new FormLayout());
		setText("Please Wait...");
		setSize(400, 130);
		final ApplyFilterThread filterThread = new ApplyFilterThread();
		filterThread.setDaemon(true);
		filterThread.start();

		Label lblApplyingYourRequested = new Label(this, SWT.NONE);
		FormData fd_lblApplyingYourRequested = new FormData();
		fd_lblApplyingYourRequested.top = new FormAttachment(0, 10);
		fd_lblApplyingYourRequested.left = new FormAttachment(0, 10);
		lblApplyingYourRequested.setLayoutData(fd_lblApplyingYourRequested);
		lblApplyingYourRequested.setText("Applying your requested game filters, please wait...");
		
		final ProgressBar progressBar = new ProgressBar(this, SWT.NONE);
		FormData fd_progressBar = new FormData();
		fd_progressBar.top = new FormAttachment(lblApplyingYourRequested, 18);
		fd_progressBar.left = new FormAttachment(0, 10);
		fd_progressBar.right = new FormAttachment(100, -10);
		progressBar.setLayoutData(fd_progressBar);
		
		display.timerExec(50, new Runnable() {
			int i = 0;
			@Override
			public void run() {
				if (isDisposed()) {
					return;
				}
				progressBar.setSelection(i % progressBar.getMaximum());
				i++;
				if (filterThread.isAlive()) {
					display.timerExec(50, this);
				}
				else {
					if (qr == null) {
						errorMsgbox("Filter Error", "The requested filter has too few movies\n" +
								"Please broaden your filter");
						ApplyFilterScreen.this.close();
					}
					else {
						ApplyFilterScreen.this.setEnabled(false);
						ApplyFilterScreen.this.setVisible(false);
						GameScreen.run(display, qr);
						ApplyFilterScreen.this.close();
					}
				}
			}
		});
	}

	@Override
	protected void checkSubclass() {
		// Disable the check that prevents subclassing of SWT components
	}

}
