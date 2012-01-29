package ponytrivia.question;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class QuestionInfo {
	public String questionText;
	public List<String> answers;
	public int correctAnswerIndex;
	
	public QuestionInfo(String questionText, String rightAnswer, List<String> wrongAnswers) {
		this.questionText = questionText;
		answers = new ArrayList<String>();
		assert(!wrongAnswers.contains(rightAnswer));
		answers.add(rightAnswer);
		answers.addAll(wrongAnswers);
		Collections.shuffle(answers);
		correctAnswerIndex = answers.indexOf(rightAnswer);
	}
}
