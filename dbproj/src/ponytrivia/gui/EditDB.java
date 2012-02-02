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

public class EditDB extends Shell {

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
		
		TabItem tbtmPeople = new TabItem(tabFolder, SWT.NONE);
		tbtmPeople.setText("People");
		
		Composite composite_1 = new Composite(tabFolder, SWT.NONE);
		tbtmPeople.setControl(composite_1);
		
		TabItem tbtmRoles = new TabItem(tabFolder, SWT.NONE);
		tbtmRoles.setText("Roles");
		
		Composite composite_2 = new Composite(tabFolder, SWT.NONE);
		tbtmRoles.setControl(composite_2);
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
