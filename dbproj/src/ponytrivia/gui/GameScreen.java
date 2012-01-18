package ponytrivia.gui;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Random;
import java.util.concurrent.ArrayBlockingQueue;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.UnsupportedAudioFileException;

import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
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
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.graphics.Rectangle;

import ponytrivia.db.Schema;
import ponytrivia.question.QuestionInfo;
import ponytrivia.question.QuestionRegistry;
import org.eclipse.swt.widgets.Scale;
import org.eclipse.swt.widgets.ExpandBar;

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
		public int questions_to_win = 10;
		public int pony_pos = 0;
		protected int turnsBeforeEnablingFiftyFifty;
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
	private final Image imgKitty1 = SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/res/kitty1.gif");
	private final Image imgKitty2 = SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/res/kitty2.gif");
	private final BgMusicThread bgMusicThread = new BgMusicThread();
	
	protected void createContents() {
		shlPonyTrivia = new Shell();
		shlPonyTrivia.setSize(800, 600);
		shlPonyTrivia.setText("Pony Trivia");
		shlPonyTrivia.setLayout(new FormLayout());
		shlPonyTrivia.setMinimumSize(shlPonyTrivia.getSize());
		
		bgMusicThread.start();
		
		final Label lblFlower = new Label(shlPonyTrivia, SWT.NONE);
		FormData fd_lblFlower = new FormData();
		fd_lblFlower.left = new FormAttachment(0, 10);
		lblFlower.setLayoutData(fd_lblFlower);
		lblFlower.setImage(SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/res/flower.gif"));
		
		final Label lblDevil = new Label(shlPonyTrivia, SWT.NONE);
		FormData fd_lblDevil = new FormData();
		fd_lblDevil.right = new FormAttachment(100, -10);
		fd_lblDevil.top = new FormAttachment(lblFlower, 0, SWT.TOP);
		lblDevil.setLayoutData(fd_lblDevil);
		lblDevil.setImage(SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/res/hell_boy.gif"));
		
		Composite composite = new Composite(shlPonyTrivia, SWT.NONE);
		composite.setLocation(10, -227);
		composite.setLayout(new FormLayout());
		FormData fd_composite = new FormData();
		fd_composite.bottom = new FormAttachment(100, -10);
		fd_composite.left = new FormAttachment(0, 10);
		fd_composite.right = new FormAttachment(100, -10);
		composite.setLayoutData(fd_composite);
		
		final Button btnNext = new Button(composite, SWT.NONE);
		btnNext.setEnabled(false);
		FormData fd_btnNext = new FormData();
		fd_btnNext.right = new FormAttachment(100, -10);
		btnNext.setLayoutData(fd_btnNext);
		btnNext.setText("Next");
		
		final Button btnFiftyFifty = new Button(composite, SWT.NONE);
		fd_btnNext.left = new FormAttachment(btnFiftyFifty, 0, SWT.LEFT);
		btnFiftyFifty.setToolTipText("");
		FormData fd_btnFiftyFifty = new FormData();
		fd_btnFiftyFifty.right = new FormAttachment(100, -10);
		btnFiftyFifty.setLayoutData(fd_btnFiftyFifty);
		btnFiftyFifty.setImage(SWTResourceManager.getImage(GameScreen.class, "/ponytrivia/gui/res/lifebelt.gif"));
		
		Composite composite_1 = new Composite(composite, SWT.NONE);
		fd_btnNext.top = new FormAttachment(composite_1, 4);
		fd_btnFiftyFifty.bottom = new FormAttachment(composite_1, -6);
		composite_1.setLayout(new FillLayout(SWT.VERTICAL));
		FormData fd_composite_1 = new FormData();
		fd_composite_1.left = new FormAttachment(0, 10);
		fd_composite_1.right = new FormAttachment(100, -10);
		fd_composite_1.top = new FormAttachment(0, 90);
		fd_composite_1.bottom = new FormAttachment(100, -44);
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
		fd_composite_2.right = new FormAttachment(btnFiftyFifty, -6);
		fd_composite_2.bottom = new FormAttachment(0, 84);
		fd_composite_2.top = new FormAttachment(0, 10);
		fd_composite_2.left = new FormAttachment(0, 10);
		composite_2.setLayoutData(fd_composite_2);
		composite_2.setLayout(new FillLayout(SWT.HORIZONTAL));
		
		final Label lblQuestionText = new Label(composite_2, SWT.NONE);
		lblQuestionText.setText("Question Text");
		
		Composite composite_3 = new Composite(shlPonyTrivia, SWT.NONE);
		fd_lblFlower.top = new FormAttachment(composite_3, 37);
		composite_3.setLayout(new FormLayout());
		FormData fd_composite_3 = new FormData();
		fd_composite_3.left = new FormAttachment(0);
		fd_composite_3.right = new FormAttachment(100);
		fd_composite_3.top = new FormAttachment(0, 10);
		fd_composite_3.bottom = new FormAttachment(0, 51);
		composite_3.setLayoutData(fd_composite_3);
		
		final Label lblTime = new Label(composite_3, SWT.NONE);
		FormData fd_lblTime = new FormData();
		fd_lblTime.top = new FormAttachment(0, 10);
		fd_lblTime.left = new FormAttachment(0, 10);
		lblTime.setLayoutData(fd_lblTime);
		lblTime.setText("Remaining Time: 30");
		
		final Label lblScore = new Label(composite_3, SWT.NONE);
		FormData fd_lblScore = new FormData();
		fd_lblScore.left = new FormAttachment(100, -89);
		fd_lblScore.top = new FormAttachment(lblTime, 0, SWT.TOP);
		fd_lblScore.right = new FormAttachment(100, -10);
		lblScore.setLayoutData(fd_lblScore);
		lblScore.setText("Score: 0");
		
		final Composite composite_4 = new Composite(shlPonyTrivia, SWT.NONE);
		fd_composite.top = new FormAttachment(0, 222);
		composite_4.setLayout(null);
		FormData fd_composite_4 = new FormData();
		fd_composite_4.bottom = new FormAttachment(composite, -6);
		fd_composite_4.top = new FormAttachment(composite_3, 6);
		fd_composite_4.right = new FormAttachment(lblDevil, -6);
		fd_composite_4.left = new FormAttachment(lblFlower, 6);
		composite_4.setLayoutData(fd_composite_4);

		final Label lblGrass = new Label(composite_4, SWT.NONE);
		lblGrass.setBounds(10, 129, 589, 20);

		final Label lblPony = new Label(composite_4, SWT.NONE);
		lblPony.setBounds(260, 37, 84, 86);
		lblPony.setAlignment(SWT.CENTER);
		lblPony.setImage(imgKitty1);
		lblGrass.setBackground(SWTResourceManager.getColor(SWT.COLOR_DARK_GREEN));
		
		Scale scale = new Scale(composite_4, SWT.NONE);
		scale.setEnabled(false);
		scale.setSelection(50);
		scale.setBounds(10, 101, 589, 48);
		
		//////////////////////////////////////////////////////////////////////////////////0/////////

		final Button answerButtons[] = new Button[] {btnAnswer_1, btnAnswer_2, btnAnswer_3, btnAnswer_4};

		composite_4.addListener(SWT.Resize, new Listener() {
			public void handleEvent(Event e) {
				Rectangle r = lblGrass.getBounds(); 
				r.width = composite_4.getBounds().width - 6;
				lblGrass.setBounds(r);
				
				int x = (int)((lblGrass.getBounds().width / 2) * (1 - ((double)gameinfo.pony_pos) / gameinfo.questions_to_win));
				lblPony.setLocation(x - lblPony.getBounds().width / 2, lblPony.getLocation().y);
			}
		});
		
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
				int delta = lblGrass.getBounds().width / (2 * gameinfo.questions_to_win);
				int timeout = 1000;
				btnNext.setEnabled(false);
				
				final Button correct = answerButtons[gameinfo.correctAnswerIndex];

				if (correct.getSelection()) {
					gameinfo.total_score += 10 + (gameinfo.remaining_time < 0 ? 0 : gameinfo.remaining_time);
					delta = -delta;
					correct.setBackground(new Color(Display.getCurrent(), 150, 250, 150));
					lblPony.setImage(imgKitty1);
					gameinfo.pony_pos += 1;
				}
				else {
					gameinfo.total_score -= 10;
					correct.setBackground(new Color(Display.getCurrent(), 250, 150, 150));
					timeout = 1500;
					lblPony.setImage(imgKitty2);
					gameinfo.pony_pos -= 1;
				}
				if (gameinfo.total_score < 0) {
					gameinfo.total_score = 0;
				}
				lblScore.setText("Score: " + gameinfo.total_score);
				gameinfo.question_number += 1;
				
				gameinfo.turnsBeforeEnablingFiftyFifty -= 1;
				
				class AnimatePony implements Runnable
				{
					private int delta;
					private int cnt;
					private int timeout;
					private final int steps = 20;
					private Point origPony;
					private Point origFlower;
					private Point origDevil;
					
					public AnimatePony(int delta, int timeout) {
						this.delta = delta;
						this.timeout = timeout;
						cnt = 0;
						origPony = lblPony.getLocation();
						origFlower = lblFlower.getLocation();
						origDevil = lblDevil.getLocation();
					}

					@Override
					public void run() {
						cnt++;
						Point p = lblPony.getLocation();
						double height = Math.abs(Math.sin(((double)cnt / steps) * 3 * Math.PI));
						lblPony.setLocation(p.x + delta / steps, origPony.y - (int)(10 * height));
						
						if (delta < 0) {
							lblFlower.setLocation(origFlower.x, origFlower.y - (int)(10 * Math.sin((double)cnt / steps * Math.PI)));
						}
						else {
							lblDevil.setLocation(origDevil.x, origDevil.y - (int)(10 * Math.sin((double)cnt / steps * Math.PI)));
						}
						
						if (cnt < steps) {
							display.timerExec(timeout / steps, this);
						}
					}
				}

				display.timerExec(0, new AnimatePony(delta, (int)(timeout * 0.9)));

				display.timerExec(timeout, new Runnable() {
					@Override
					public void run()
					{
						if (gameinfo.pony_pos >= gameinfo.questions_to_win) {
							shlPonyTrivia.close();
							return;
						}
						if (gameinfo.turnsBeforeEnablingFiftyFifty <= 0) {
							gameinfo.turnsBeforeEnablingFiftyFifty = 0;
							btnFiftyFifty.setEnabled(true);
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
				gameinfo.turnsBeforeEnablingFiftyFifty = 3;
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
	
	public static class BgMusicThread extends Thread
	{
		Clip clip;
		
		public void run()
		{
			InputStream ins = GameScreen.class.getResourceAsStream("res/nyan.wav");
			AudioInputStream audioIn;
			System.out.println(ins);
			
			try {
				clip = AudioSystem.getClip();
				audioIn = AudioSystem.getAudioInputStream(ins);
				clip.open(audioIn);
			} catch (UnsupportedAudioFileException e) {
				// ignore
				e.printStackTrace();
				return;
			} catch (IOException e) {
				// ignore
				e.printStackTrace();
				return;
			} catch (LineUnavailableException e) {
				// ignore
				e.printStackTrace();
				return;
			}
			System.out.println("woohoo");

			clip.loop(Clip.LOOP_CONTINUOUSLY);
		}
		
		public void stopMusic()
		{
			clip.stop();
		}
	}
}
