package ponytrivia.questions;

public class QuestionInfo {
	public String questionText;
	public String[] answers;
	public int correctAnswerIndex;
	
	public QuestionInfo(String text, String[] answers, int correct) {
		this.questionText = text;
		this.answers = answers;
		this.correctAnswerIndex = correct;
	}
	
	@Override
	public String toString()
	{
		return "Question: " + questionText;
	}
}
