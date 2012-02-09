package ponytrivia.question;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * holds question info: the text, the answers, and the index of the correct answer.
 * all fields are `public final`, there's nothing to "protected" here.
 * it's also in charge of mangling the right and wrong answers
 */
public class QuestionInfo {
	public final String questionText;
	public final List<String> answers;
	public final int correctAnswerIndex;
	
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
