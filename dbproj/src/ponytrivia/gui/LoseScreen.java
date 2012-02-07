package ponytrivia.gui;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.wb.swt.SWTResourceManager;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormAttachment;

public class LoseScreen extends Shell {

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void run(Display display) {
		try {
			LoseScreen shell = new LoseScreen(display);
			shell.open();
			shell.setActive();
			shell.setFocus();
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
	public LoseScreen(Display display) {
		super(display, SWT.SHELL_TRIM);
		setLayout(new FormLayout());
		
		Label lblNewLabel = new Label(this, SWT.NONE);
		lblNewLabel.setAlignment(SWT.CENTER);
		FormData fd_lblNewLabel = new FormData();
		fd_lblNewLabel.bottom = new FormAttachment(100, -10);
		fd_lblNewLabel.top = new FormAttachment(0, 10);
		fd_lblNewLabel.left = new FormAttachment(0, 10);
		fd_lblNewLabel.right = new FormAttachment(100, -10);
		lblNewLabel.setLayoutData(fd_lblNewLabel);
		lblNewLabel.setImage(SWTResourceManager.getImage(LoseScreen.class, "/ponytrivia/gui/res/deadpony1.jpg"));
		createContents();
	}

	/**
	 * Create contents of the shell.
	 */
	protected void createContents() {
		setText("The Devil Has Your Soul!");
		setSize(965, 689);
	}

	@Override
	protected void checkSubclass() {
		// Disable the check that prevents subclassing of SWT components
	}

}
