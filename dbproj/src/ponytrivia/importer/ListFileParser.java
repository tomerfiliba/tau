package ponytrivia.importer;

import java.io.BufferedReader;
import java.io.EOFException;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ListFileParser
{
	private FileInputStream fstream;
	private BufferedReader reader;
	
	public ListFileParser(String filename) throws IOException 
	{
		fstream = new FileInputStream(filename);
		reader = new BufferedReader(new InputStreamReader(fstream));
	}
	
	public void close()
	{
		if (reader != null) {
			try{
				reader.close();
			} catch (IOException ex) {
			}
		}
		if (fstream != null) {
			try {
				fstream.close();
			} catch (IOException ex) {
			}
		}
		fstream = null;
		reader = null;
	}
	
	public String readLine() throws IOException
	{
		String line = reader.readLine();
		if (line == null) {
			throw new EOFException();
		}
		return line;
	}

	public void skipUntil(String regex) throws IOException
	{
		readUntil(regex);
	}
	
	public List<String> readUntil(String regex) throws IOException
	{
		return readUntil(regex, false);
	}

	public List<String> readUntil(String regex, boolean includeLastLine) throws IOException
	{
		ArrayList<String> lines = new ArrayList<String>();
		String line;
		Pattern p = Pattern.compile(regex);
		
		while (true) {
			try {
				line = readLine();
			} catch (EOFException ex) {
				if (lines.isEmpty()) {
					throw ex;
				}
				break;
			}
			Matcher m = p.matcher(line);
			if (m.matches()) {
				if (includeLastLine) {
					lines.add(line);
				}
				break;
			}
			lines.add(line);
		}
		return lines;
	}

}
