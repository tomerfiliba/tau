import lxml.etree as XML

e = XML.fromstring("""
<document>
    <course>
        <name>foo</name>
        <prereq>
            <course>
                <name>databases</name>
                <prereq/>
                <teacher>smith</teacher>
                <teacher>bob</teacher>
                <student>william</student>
                <student>herring</student>
                <student>zooba</student>
            </course>
        </prereq>
        <teacher>moses</teacher>
        <teacher>aaron</teacher>
        <student>tzipora</student>
    </course>

    <course>
        <name>lala</name>
        <prereq>
            <course>
                <name>foo</name>
                <prereq>
                    <course>
                        <name>databases</name>
                        <prereq/>
                        <teacher>smith</teacher>
                        <teacher>bob</teacher>
                        <student>william</student>
                        <student>herring</student>
                        <student>zooba</student>
                    </course>
                </prereq>
                <teacher>moses</teacher>
                <teacher>aaron</teacher>
                <student>tzipora</student>
            </course>
        </prereq>
        <teacher>john</teacher>
        <teacher>long john</teacher>
        <student>zilver</student>
    </course>

    <course>
        <name>zaza</name>
        <prereq>
            <course>
                <name>lala</name>
                <prereq>
                    <course>
                        <name>foo</name>
                        <prereq>
                            <course>
                                <name>databases</name>
                                <prereq/>
                                <teacher>smith</teacher>
                                <teacher>bob</teacher>
                                <student>william</student>
                                <student>herring</student>
                                <student>zooba</student>
                            </course>
                        </prereq>
                        <teacher>moses</teacher>
                        <teacher>aaron</teacher>
                        <student>tzipora</student>
                    </course>
                </prereq>
                <teacher>john</teacher>
                <teacher>long john</teacher>
                <student>zilver</student>
            </course>
        </prereq>
        <teacher>saba</teacher>
        <teacher>garga</teacher>
        <teacher>mel</teacher>
        <student>darda</student>
    </course>

</document>
""")

# //course[prereq/course/name='databases']
# //course[prereq//prereq/course/name='databases' and count(teacher) >= 2]
# //course[count(teacher) > count(student)]

for e2 in e.xpath("//course[count(teacher) > count(student)]/name"):
    print "<course>\n    <name>%s</name>\n    <...>\n</course>" % (e2.text,)












