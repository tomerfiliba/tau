package ponytrivia.gui;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Random;
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
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionInfo;
import ponytrivia.question.QuestionRegistry;

public class GameScreen {

	protected Display display;
	protected Shell shlPonyTrivia;
	
	protected static class GameInfo
	{
		public int question_number = 1;
		public final int alotted_time = 20;
		public int remaining_time = alotted_time;
		public int total_score = 0;
		public int correctAnswerIndex = -1;
	}
	
	protected final GameInfo gameinfo = new GameInfo();
	protected final QuestionRegistry questionRegistry;

	/**
	 * Launch the application.
	 * @param args
	 */
	public static void main(String[] args) {
		try {
			Schema schema = new Schema("localhost:3306", "root", "root");
			GameScreen window = new GameScreen(new QuestionRegistry(schema));
			window.open();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	public GameScreen(QuestionRegistry questionRegistry)
	{
		this.questionRegistry = questionRegistry;
	}

	/**
	 * Open the window.
	 */
	public void open() {
		display = Display.getDefault();
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
		
		final Label lblScore = new Label(shlPonyTrivia, SWT.NONE);
		FormData fd_lblScore = new FormData();
		fd_lblScore.left = new FormAttachment(100, -114);
		fd_lblScore.top = new FormAttachment(0, 10);
		fd_lblScore.right = new FormAttachment(100, -10);
		lblScore.setLayoutData(fd_lblScore);
		lblScore.setText("Score: 0");
		
		final Button btnNext = new Button(composite, SWT.NONE);
		btnNext.setEnabled(false);
		FormData fd_btnNext = new FormData();
		fd_btnNext.right = new FormAttachment(100, -10);
		fd_btnNext.left = new FormAttachment(0, 509);
		btnNext.setLayoutData(fd_btnNext);
		btnNext.setText("Next");
		
		final Button btnFiftyFifty = new Button(composite, SWT.NONE);
		FormData fd_btnFiftyFifty = new FormData();
		fd_btnFiftyFifty.right = new FormAttachment(0, 597);
		fd_btnFiftyFifty.top = new FormAttachment(0, 10);
		fd_btnFiftyFifty.left = new FormAttachment(0, 514);
		btnFiftyFifty.setLayoutData(fd_btnFiftyFifty);
		btnFiftyFifty.setImage(SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/lifebelt.gif"));
		
		Composite composite_1 = new Composite(composite, SWT.NONE);
		fd_btnNext.top = new FormAttachment(composite_1, 6);
		composite_1.setLayout(new FillLayout(SWT.VERTICAL));
		FormData fd_composite_1 = new FormData();
		fd_composite_1.bottom = new FormAttachment(100, -44);
		fd_composite_1.top = new FormAttachment(btnFiftyFifty, 6);
		fd_composite_1.right = new FormAttachment(0, 597);
		fd_composite_1.left = new FormAttachment(0, 10);
		composite_1.setLayoutData(fd_composite_1);
		
		SelectionAdapter enableNext = new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				btnNext.setEnabled(true);
			}
		};
		
		final Button btnAnswer_1 = new Button(composite_1, SWT.RADIO);
		btnAnswer_1.addSelectionListener(enableNext);
		btnAnswer_1.setText("Answer 1");
		
		final Button btnAnswer_2 = new Button(composite_1, SWT.RADIO);
		btnAnswer_2.addSelectionListener(enableNext);
		btnAnswer_2.setText("Answer 2");
		
		final Button btnAnswer_3 = new Button(composite_1, SWT.RADIO);
		btnAnswer_3.addSelectionListener(enableNext);
		btnAnswer_3.setText("Answer 3");
		
		final Button btnAnswer_4 = new Button(composite_1, SWT.RADIO);
		btnAnswer_4.addSelectionListener(enableNext);
		btnAnswer_4.setText("Answer 4");
		
		Composite composite_2 = new Composite(composite, SWT.NONE);
		FormData fd_composite_2 = new FormData();
		fd_composite_2.bottom = new FormAttachment(0, 84);
		fd_composite_2.right = new FormAttachment(0, 508);
		fd_composite_2.top = new FormAttachment(0, 10);
		fd_composite_2.left = new FormAttachment(0, 10);
		composite_2.setLayoutData(fd_composite_2);
		composite_2.setLayout(new FillLayout(SWT.HORIZONTAL));
		
		final Label lblQuestionText = new Label(composite_2, SWT.NONE);
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
		
		final Label lblTime = new Label(shlPonyTrivia, SWT.NONE);
		lblTime.setText("Remaining Time: 30");
		FormData fd_lblTime = new FormData();
		fd_lblTime.top = new FormAttachment(0, 10);
		fd_lblTime.left = new FormAttachment(lblNewLabel_1, 0, SWT.LEFT);
		lblTime.setLayoutData(fd_lblTime);
		
		////////////////////////////////////////////////////////////////////////////////////////////

		final Button answerButtons[] = new Button[] {btnAnswer_1, btnAnswer_2, btnAnswer_3, btnAnswer_4};

		final Runnable updateTimeLabel = new Runnable() {
			private Color original = lblTime.getForeground();
			@Override
			public void run()
			{
				lblTime.setText("Remaining time: " + gameinfo.remaining_time);
				if (gameinfo.remaining_time <= gameinfo.alotted_time / 3) {
					lblTime.setForeground(display.getSystemColor(SWT.COLOR_RED));
				}
				else {
					lblTime.setForeground(original);
				}
			}
		};
		updateTimeLabel.run();
		
		final Color origButtonColor = btnAnswer_1.getBackground();

		final Runnable updateQuestion = new Runnable() {
			@Override
			public void run()
			{
				QuestionInfo qi;
				try {
					qi = questionRegistry.getQuestion();
				} catch (SQLException ex) {
					ex.printStackTrace();
					return;
				}
				lblQuestionText.setText(qi.questionText);
				for (int i = 0; i < answerButtons.length; i++) {
					Button btn = answerButtons[i];
					btn.setText(qi.answers.get(i));
					btn.setEnabled(true);
					btn.setBackground(origButtonColor);
					btn.setSelection(false);
				}
				gameinfo.correctAnswerIndex = qi.correctAnswerIndex;
			}
		};
		updateQuestion.run();
		
		final SelectionAdapter answerQuestion = new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				int delta = (label.getBounds().width - lblPony.getBounds().width / 2) / 10;
				int timeout = 1000;
				btnNext.setEnabled(false);
				
				final Button correct = answerButtons[gameinfo.correctAnswerIndex];

				if (correct.getSelection()) {
					gameinfo.total_score += 10 + (gameinfo.remaining_time < 0 ? 0 : gameinfo.remaining_time);
					delta = -delta;
					correct.setBackground(new Color(Display.getCurrent(), 150, 250, 150));
				}
				else {
					gameinfo.total_score -= 10;
					correct.setBackground(new Color(Display.getCurrent(), 250, 150, 150));
					timeout = 1500;
				}
				if (gameinfo.total_score < 0) {
					gameinfo.total_score = 0;
				}
				lblScore.setText("Score: " + gameinfo.total_score);
				gameinfo.question_number += 1;
							
				class AnimatePony implements Runnable
				{
					private int delta;
					private int cnt;
					private int timeout;
					private final int steps = 20;
					private Point orig;
					
					public AnimatePony(int delta, int timeout) {
						this.delta = delta;
						this.timeout = timeout;
						cnt = 0;
						orig = lblPony.getLocation();
					}

					@Override
					public void run() {
						cnt++;
						Point p = lblPony.getLocation();
						double height = Math.abs(Math.sin(((double)cnt / steps) * 3 * Math.PI));
						lblPony.setLocation(p.x + delta / steps, orig.y - (int)(10 * height));
						if (cnt < steps) {
							display.timerExec(timeout / steps, this);
						}
					}
				}

				display.timerExec(0, new AnimatePony(delta, timeout));

				display.timerExec(timeout, new Runnable() {
					@Override
					public void run()
					{
						if (lblPony.getLocation().x < label.getBounds().x) {
							shlPonyTrivia.close();
							return;
						}
						updateTimeLabel.run();
						updateQuestion.run();
						gameinfo.remaining_time = gameinfo.alotted_time;					
					}
				});
			}
		};
		
		btnFiftyFifty.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent arg0) {
				btnFiftyFifty.setEnabled(false);
				ArrayList<Integer> discarded = new ArrayList<Integer>();
				discarded.add(0);
				discarded.add(1);
				discarded.add(2);
				discarded.add(3);
				discarded.remove(gameinfo.correctAnswerIndex);
				Collections.shuffle(discarded);
				discarded.remove(2);
				for (int i : discarded) {
					Button btn = answerButtons[i];
					btn.setEnabled(false);
					btn.setSelection(false);
					btn.setBackground(new Color(Display.getCurrent(), 50, 50, 50));
				}
			}
		});
		
		final Runnable timer = new Runnable() {
			public void run() {
				gameinfo.remaining_time -= 1;
				display.timerExec(1000, this);
				updateTimeLabel.run();
				if (gameinfo.remaining_time < 0) {
					answerQuestion.widgetSelected(null);
				}
			}
		};
		display.timerExec(1000, timer);
		
		btnNext.addSelectionListener(answerQuestion);
	}
	
}
