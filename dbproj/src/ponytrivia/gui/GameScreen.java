package ponytrivia.gui;

import java.util.ArrayList;
import java.util.concurrent.ArrayBlockingQueue;

import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.SWT;
import org.eclipse.wb.swt.SWTResourceManager;
import org.eclipse.swt.widgets.ProgressBar;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;

public class GameScreen {

	protected Shell shlPonyTrivia;
	protected final ArrayList<Integer> questionCounter = new ArrayList<Integer>();

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			GameScreen window = new GameScreen();
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
		shlPonyTrivia.setSize(645, 481);
		shlPonyTrivia.setText("Pony Trivia");
		shlPonyTrivia.setLayout(new FormLayout());
		
		final Label lblPony = new Label(shlPonyTrivia, SWT.NONE);
		FormData fd_lblPony = new FormData();
		lblPony.setLayoutData(fd_lblPony);
		lblPony.setImage(SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/kitty.gif"));
		
		questionCounter.add(1);
		
		Label lblNewLabel_1 = new Label(shlPonyTrivia, SWT.NONE);
		fd_lblPony.left = new FormAttachment(lblNewLabel_1, 184);
		fd_lblPony.bottom = new FormAttachment(lblNewLabel_1, 0, SWT.BOTTOM);
		FormData fd_lblNewLabel_1 = new FormData();
		fd_lblNewLabel_1.left = new FormAttachment(0, 10);
		fd_lblNewLabel_1.top = new FormAttachment(0, 34);
		lblNewLabel_1.setLayoutData(fd_lblNewLabel_1);
		lblNewLabel_1.setImage(SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/flower.gif"));
		
		Label label_1 = new Label(shlPonyTrivia, SWT.NONE);
		fd_lblPony.right = new FormAttachment(label_1, -191);
		FormData fd_label_1 = new FormData();
		fd_label_1.right = new FormAttachment(100, -10);
		fd_label_1.top = new FormAttachment(lblPony, 0, SWT.TOP);
		label_1.setLayoutData(fd_label_1);
		label_1.setImage(SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/hell_boy.gif"));
		
		Composite composite = new Composite(shlPonyTrivia, SWT.NONE);
		composite.setLocation(10, -227);
		composite.setLayout(new FormLayout());
		FormData fd_composite = new FormData();
		fd_composite.bottom = new FormAttachment(100, -10);
		fd_composite.left = new FormAttachment(0, 10);
		fd_composite.right = new FormAttachment(100, -10);
		composite.setLayoutData(fd_composite);
		
		final Label lblQuestion = new Label(shlPonyTrivia, SWT.NONE);
		FormData fd_lblQuestion = new FormData();
		fd_lblQuestion.top = new FormAttachment(0, 10);
		fd_lblQuestion.right = new FormAttachment(100, -10);
		lblQuestion.setLayoutData(fd_lblQuestion);
		lblQuestion.setText("Question 15/20");
		
		Button btnNext = new Button(composite, SWT.NONE);
		FormData fd_btnNext = new FormData();
		fd_btnNext.right = new FormAttachment(100, -10);
		fd_btnNext.left = new FormAttachment(0, 509);
		btnNext.setLayoutData(fd_btnNext);
		btnNext.setText("Next");
		
		Button button = new Button(composite, SWT.NONE);
		FormData fd_button = new FormData();
		fd_button.right = new FormAttachment(0, 597);
		fd_button.top = new FormAttachment(0, 10);
		fd_button.left = new FormAttachment(0, 514);
		button.setLayoutData(fd_button);
		button.setImage(SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/lifebelt.gif"));
		
		Composite composite_1 = new Composite(composite, SWT.NONE);
		fd_btnNext.top = new FormAttachment(composite_1, 6);
		composite_1.setLayout(new FillLayout(SWT.VERTICAL));
		FormData fd_composite_1 = new FormData();
		fd_composite_1.bottom = new FormAttachment(100, -44);
		fd_composite_1.top = new FormAttachment(button, 6);
		fd_composite_1.right = new FormAttachment(0, 597);
		fd_composite_1.left = new FormAttachment(0, 10);
		composite_1.setLayoutData(fd_composite_1);
		
		final Button btnAnswer_1 = new Button(composite_1, SWT.RADIO);
		btnAnswer_1.setText("Answer 1");
		
		final Button btnAnswer_2 = new Button(composite_1, SWT.RADIO);
		btnAnswer_2.setText("Answer 2");
		
		final Button btnAnswer_3 = new Button(composite_1, SWT.RADIO);
		btnAnswer_3.setText("Answer 3");
		
		final Button btnAnswer_4 = new Button(composite_1, SWT.RADIO);
		btnAnswer_4.setText("Answer 4");
		
		Composite composite_2 = new Composite(composite, SWT.NONE);
		FormData fd_composite_2 = new FormData();
		fd_composite_2.bottom = new FormAttachment(0, 84);
		fd_composite_2.right = new FormAttachment(0, 508);
		fd_composite_2.top = new FormAttachment(0, 10);
		fd_composite_2.left = new FormAttachment(0, 10);
		composite_2.setLayoutData(fd_composite_2);
		composite_2.setLayout(new FillLayout(SWT.HORIZONTAL));
		
		Label lblQuestionText = new Label(composite_2, SWT.NONE);
		lblQuestionText.setText("Question Text");
		
		final Label label = new Label(shlPonyTrivia, SWT.SEPARATOR | SWT.HORIZONTAL);
		fd_composite.top = new FormAttachment(label, 6);
		fd_lblNewLabel_1.bottom = new FormAttachment(100, -306);
		FormData fd_label = new FormData();
		fd_label.top = new FormAttachment(lblPony, 6);
		fd_label.bottom = new FormAttachment(100, -298);
		fd_label.left = new FormAttachment(0, 82);
		fd_label.right = new FormAttachment(100, -94);
		label.setLayoutData(fd_label);
		
		btnNext.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				int delta = (label.getBounds().width - lblPony.getBounds().width / 2) / 20;
				
				Point p = lblPony.getLocation();
				if (btnAnswer_3.getSelection()) {
					delta = -delta;					
				}
				
				lblPony.setLocation(p.x + delta, p.y);
				int cnt = questionCounter.get(0);
				lblQuestion.setText("Question " + cnt + "/20");
				questionCounter.set(0, cnt + 1);
			}
		});

	}
}
