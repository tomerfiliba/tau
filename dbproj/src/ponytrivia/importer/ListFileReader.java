package ponytrivia.importer;

import java.io.BufferedReader;
import java.io.EOFException;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.GZIPInputStream;

/**
 * A utility class for reading IMDB's list files
 */
public class ListFileReader {
	private FileInputStream fstream;
	private GZIPInputStream gzstream;
	private BufferedReader reader;
	public final String fileName;

	/**
	 * constructs a ListFileReader from the given File object
	 * @param f
	 * @throws IOException
	 */
	public ListFileReader(File f) throws IOException {
		if (!f.exists()) {
			f = new File(f.getPath() + ".gz");
		}
		fileName = f.getName();
		fstream = new FileInputStream(f);
		if (fileName.endsWith(".gz") || fileName.endsWith(".gzip")) {
			gzstream = new GZIPInputStream(fstream);
			reader = new BufferedReader(new InputStreamReader(gzstream));
		}
		else {
			gzstream = null;
			reader = new BufferedReader(new InputStreamReader(fstream));
		}
	}

	/**
	 * closes the file and releases all resources
	 */
	public void close() {
		if (reader != null) {
			try {
				reader.close();
			} catch (IOException ex) {
			}
		}
		if (gzstream != null) {
			try {
				gzstream.close();
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
		gzstream = null;
		reader = null;
	}
	
	/**
	 * returns the current file position
	 * @return file position
	 * @throws IOException
	 */
	public long getPosition() throws IOException {
		return fstream.getChannel().position();
	}
	
	/**
	 * returns the file's size
	 * @return file's size
	 * @throws IOException
	 */
	public long getSize() throws IOException {
		return fstream.getChannel().size();
	}

	/**
	 * reads the next line from the line
	 * @return the line or null on EOF
	 * @throws IOException
	 */
	public String readLine() throws IOException {
		return reader.readLine();
	}

	/**
	 * skips all lines until (and including) the given pattern
	 * @param regex
	 * @throws IOException
	 */
	public void skipUntil(String regex) throws IOException {
		readUntil(regex);
	}

	/**
	 * reads a block of lines from the file, until the pattern is matched
	 * @param regex
	 * @return a list of lines, not including the last (matching) line
	 * @throws IOException
	 */
	public List<String> readUntil(String regex) throws IOException {
		return readUntil(regex, false, true);
	}

	/**
	 * reads a block of lines from the file, until the pattern is matched
	 * @param regex
	 * @param includeLastLine - whether to include the last (matching) line or not
	 * @param throwOnEOF - whether to throw EOFException on EOF, if the pattern was not found
	 * @return a list of lines
	 * @throws IOException
	 */
	public List<String> readUntil(String regex, boolean includeLastLine, boolean throwOnEOF)
			throws IOException {
		ArrayList<String> lines = new ArrayList<String>();
		String line;
		Pattern p = Pattern.compile(regex);

		while (true) {
			line = readLine();
			if (line == null) {
				if (lines.isEmpty() && throwOnEOF) {
					throw new EOFException("Pattern not found");
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
		if (lines.isEmpty()) {
			return null;
		}
		return lines;
	}

}