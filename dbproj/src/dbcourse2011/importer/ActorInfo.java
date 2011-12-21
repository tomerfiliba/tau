package dbcourse2011.importer;

import java.util.List;

public class ActorInfo
{
	public String name;
	public List<String> movies;
	public List<String> series;
	
	public ActorInfo()
	{
	}
	
	static ActorInfo parse(List<String> lines) 
	{
		ActorInfo ai = new ActorInfo();
		return ai;
	}
}
