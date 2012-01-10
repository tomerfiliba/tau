package ponytrivia.gui;

import java.awt.EventQueue;

import javax.swing.JFrame;
import javax.swing.JMenuBar;
import javax.swing.JMenu;
import javax.swing.JLabel;
import java.awt.BorderLayout;

public class Gui {

	private JFrame frame;

	/**
	 * Launch the application.
	 */
	public static void main(String[] args) {
		EventQueue.invokeLater(new Runnable() {
			public void run() {
				try {
					Gui window = new Gui();
					window.frame.setVisible(true);
				} catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
	}

	/**
	 * Create the application.
	 */
	public Gui() {
		initialize();
	}

	/**
	 * Initialize the contents of the frame.
	 */
	private void initialize() {
		frame = new JFrame();
		frame.setBounds(100, 100, 450, 300);
		frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		
		JMenuBar menuBar = new JMenuBar();
		frame.setJMenuBar(menuBar);
		
		JMenu mnFile = new JMenu("Game");
		menuBar.add(mnFile);
		
		JMenu mnNew = new JMenu("New Game");
		mnFile.add(mnNew);
		
		JMenu mnImportImdbFiles = new JMenu("Import IMDB files");
		mnFile.add(mnImportImdbFiles);
		
		JMenu mnViewHighscores = new JMenu("View Highscores");
		mnFile.add(mnViewHighscores);
		
		JMenu mnExit = new JMenu("Exit");
		mnFile.add(mnExit);
		
		JMenu mnEdit = new JMenu("Database");
		menuBar.add(mnEdit);
		
		JMenu mnHelp = new JMenu("Help");
		menuBar.add(mnHelp);
		
		JMenu mnGuide = new JMenu("Guide");
		mnHelp.add(mnGuide);
		
		JMenu mnAbout = new JMenu("About");
		mnHelp.add(mnAbout);
		
		JLabel lblNewLabel = new JLabel("New label");
		frame.getContentPane().add(lblNewLabel, BorderLayout.NORTH);
	}

}
