# This script takes the output from pandoc and converts it into the format needed by
# the website at Zint.org.uk
#
# Warning: This code is ugly... but it saves days of manual effort updating the website.
#
# Copyright (C) 2022 <rstuart114@gmail.com>

# Works out which tags should influence indentation and puts them on their own line
def isolate_tag(tag):
    global stage
    
    indentable_tag = True
    for keyword in indent_skip:
        if keyword in tag:
            indentable_tag = False
    
    if '</' in tag:
        # Close tag
        if (indentable_tag):
            stage += "\n"
            stage += tag
            stage += "\n"
        else:
            stage += tag
    else:
        # Open tag
        if (indentable_tag):
            stage += "\n"
            stage += tag
            stage += "\n"
        else:
            stage += tag

# Add the right amount of indendation (indentation X 4 spaces)
def add_indent():
    global indentation
    retval = ""
    
    for i in range(0,indentation):
        retval += "    "
        
    return retval

# Apply indentation to text
def with_indent(text):
    global indentation
    retval = ""
    d = ''
    
    for c in text:
        if d == '\n':
            retval += d
            retval += add_indent()
        else:
            retval += d
        d = c
        
    retval += d
            
    return retval

# Read file and pull some tags onto their own lines for later processing
manual = ""
tag = False
tag_buffer = ""
text_buffer = ""
stage = ""
indent_skip = ['img', 'code', 'pre', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6', 'span', '<a', '</a', 'sup', '<col', '</col', '<hr', 'div']

print("Reading... manual.html")
with open('manual.html') as f:
    manual = f.read()
    
    for c in manual:
        if c == '<':
            stage += text_buffer
            tag = True
            tag_buffer = ""
        
        if (tag):
            tag_buffer += c
        else:
            text_buffer += c
            
        if c == '>':
            tag_buffer = tag_buffer.replace("\n", " ")
            isolate_tag(tag_buffer)
            tag = False
            text_buffer = ""
            
f.close()
manual = stage
stage = ""

print("Adjusting HTML")
# Change the guts of the HTML tags
in_dd = False
to_remove = False
remove_next = False
span_literal = False
for c in manual:
    if c == '<':
        # Remove "{#tbl:" table identifiers
        if '{#tbl:' in text_buffer:
            text_buffer = text_buffer[text_buffer.index('tag=') + 7:-3]
            text_buffer = text_buffer.replace('\n', ' ')
            text_buffer = '\n' + text_buffer + '\n'
            
        # Remove "{@tabl:" table references
        if 'tbl:' in text_buffer:
            text_buffer = ''
        
        stage += text_buffer
        tag = True
        tag_buffer = ""
        to_remove = False
    
    if (tag):
        tag_buffer += c
    else:
        text_buffer += c
        
    if c == '>':
        # Remove some tags which aren't needed on website
        if 'span' in tag_buffer:
            to_remove = True
            
        if 'div' in tag_buffer:
            to_remove = True
            
        if '<col' in tag_buffer:
            to_remove = True
            
        if '</col' in tag_buffer:
            to_remove = True
            
        if (remove_next):
            to_remove = True
            remove_next = False
            
        if ('a href' in tag_buffer) and ('aria-hidden="true"' in tag_buffer):
            to_remove = True
            remove_next = True
            
        if '<a href="#' in tag_buffer:
            to_remove = True
            remove_next = True        
            
        # Don't allow <p> and </p> between <dd> and </dd>
        if (tag_buffer == "<dd>"):
            in_dd = True
        if (tag_buffer == "</dd>"):
            in_dd = False
            
        if (in_dd and tag_buffer == '<p>'):
            to_remove = True
            
        if (in_dd and tag_buffer == '</p>'):
            to_remove = True
            
        # Remove attributes for some tags
        if '<pre' in tag_buffer:
            tag_buffer = '<pre>'
            
        if '<table' in tag_buffer:
            tag_buffer = '<table>'
            
        if '<tr' in tag_buffer:
            tag_buffer = '<tr>'
            
        if '<td' in tag_buffer:
            tag_buffer = '<td>'
        
        if '<th ' in tag_buffer:
            tag_buffer = '<th>'
            
        # Bump all headers up one level
        tag_buffer = tag_buffer.replace('<h6', '<h7')
        tag_buffer = tag_buffer.replace('</h6', '</h7')
        tag_buffer = tag_buffer.replace('<h5', '<h6')
        tag_buffer = tag_buffer.replace('</h5', '</h6')
        tag_buffer = tag_buffer.replace('<h4', '<h5')
        tag_buffer = tag_buffer.replace('</h4', '</h5')
        tag_buffer = tag_buffer.replace('<h3', '<h4')
        tag_buffer = tag_buffer.replace('</h3', '</h4')
        tag_buffer = tag_buffer.replace('<h2', '<h3')
        tag_buffer = tag_buffer.replace('</h2', '</h3')
        tag_buffer = tag_buffer.replace('<h1', '<h2')
        tag_buffer = tag_buffer.replace('</h1', '</h2')
        
        # Change class names for code snippets
        tag_buffer = tag_buffer.replace('class="sourceCode bash"', 'class="language-bash"')
        tag_buffer = tag_buffer.replace('class="sourceCode c"', 'class="language-cpp"')
        
        # Change location of images
        tag_buffer = tag_buffer.replace('src="images/', 'src="/images/manual/')
        
        # Change <code> without language to <span>
        if tag_buffer == '<code>':
            tag_buffer = '<span class="literal">'
            span_literal = True
            
        if tag_buffer == '</code>' and span_literal:
            tag_buffer = '</span>'
            span_literal = False

        if not to_remove:
            stage += tag_buffer
        tag = False
        text_buffer = ""
        
manual = stage
stage = ""

print("Removing empty lines")
# Remove blank lines unless in between <pre> and </pre>
last_char = ''
in_pre = False
for c in manual:
    if c == '<':
        tag = True
        tag_buffer = ""
    
    if (tag):
        tag_buffer += c
    else:
        text_buffer += c
        
    if c == '>':
        if ("<pre" in tag_buffer):
            in_pre = True
        if ("</pre" in tag_buffer):
            in_pre = False
        tag = False
        text_buffer = ""
    
    if c == '\n':
        if (last_char != '\n') or (in_pre == True):
            stage += c
    else:
        stage += c
    last_char = c
            
manual = stage
stage = ""

print("Applying indentation")
# Indent the code to make it easier to read
indentation = 1
in_pre = False
paragraph_block = False
document_start = True
chapter_six = False
last_char = ''
for c in manual:
    if c == '<':
        #Fix 'floating' full stops
        text_buffer = text_buffer.replace(' . ', '. ')
        
        # Apply indentation to text
        if in_pre:
            stage += text_buffer
        else:
            stage += with_indent(text_buffer)
        tag = True
        tag_buffer = ""
    
    if (tag):
        tag_buffer += c
    else:
        # Strip '{}' from already removed table references
        if c == '}' and last_char == '{':
            text_buffer = text_buffer[:-1]
        else:
            text_buffer += c
        last_char = c
        
    if c == '>':
        indentable_tag = True
        for keyword in indent_skip:
            if keyword in tag_buffer:
                indentable_tag = False
        
        # Protect the indentation in <pre> segments
        if ('<pre' in tag_buffer):
            in_pre = True
        if ('</pre' in tag_buffer):
            in_pre = False
            
        # Chapter 6 requires special treatment - detect beginning and end
        if ('id="types-of-symbology"' in tag_buffer):
            chapter_six = True
        if ('id="legal-and-version-information"' in tag_buffer):
            chapter_six = False
        
        if '</' in tag_buffer:
            # Close tag
            if (indentable_tag):
                indentation -= 1
                stage += add_indent()
                stage += tag_buffer
            else:
                if text_buffer.endswith('\n'):
                    stage += add_indent()
                stage += tag_buffer
        else:
            # Split into sections
            if (indentation == 1) and ('<p' in tag_buffer):
                if not paragraph_block:
                    if document_start:
                        document_start = False
                    else:
                        stage += '</section>\n'
                    stage += '<section class="container">\n'
                    paragraph_block = True
                    
            # Handle headers but also decide where to split into multiple HTML files and mark with <page>
            if (indentation == 1):
                if ('<h2' in tag_buffer):
                    if document_start:
                        document_start = False
                        stage += '<section class="container">\n'
                        paragraph_block = True
                    else:
                        stage += '</section>\n'
                        stage += '<page>\n'
                        stage += '<section class="container">\n'
                        paragraph_block = True
                elif ('<h3' in tag_buffer) and chapter_six:
                        stage += '</section>\n'
                        stage += '<page>\n'
                        stage += '<section class="container">\n'
                        paragraph_block = True
                elif ('<h' in tag_buffer):
                    if not paragraph_block:
                        stage += '</section>\n'
                        stage += '<section class="container">\n'
                        paragraph_block = True
                        
            # <dl> section has it's own class
            if (indentation == 1) and ('<dl' in tag_buffer):
                stage += '</section>\n'
                stage += '<section class="definition-list container">\n'
                paragraph_block = False
                
            # <table> section has it's own class
            if (indentation == 1) and ('<table' in tag_buffer):
                stage += '</section>\n'
                stage += '<section class="table">\n'
                paragraph_block = False
            
            # Open tag
            if (indentable_tag):
                stage += add_indent()
                stage += tag_buffer
                indentation += 1
            else:
                if text_buffer.endswith('\n'):
                    stage += add_indent()
                stage += tag_buffer
        tag = False
        text_buffer = ""

stage += '\n</section>\n'
manual = stage
stage = ""

# Remove <h2> data and split into output files
out_filenames = ['chapter1.html', 'chapter2.html', 'chapter3.html', 'chapter4.html', 'chapter5.html',
                 'chapter6.0.html', 'chapter6.1.html', 'chapter6.2.html', 'chapter6.3.html', 'chapter6.4.html',
                 'chapter6.5.html', 'chapter6.6.html', 'chapter6.7.html', 'chapter7.html', 'appendixa.html', 'appendixb.html']
page = 0
print("Writing... ", out_filenames[page])
f = open(out_filenames[page], "w")
h2_tag = False
for c in manual:
    if c == '<':
        if h2_tag == False:
            stage += text_buffer
        tag = True
        tag_buffer = ""
    
    if (tag):
        tag_buffer += c
    else:
        text_buffer += c
        
    if c == '>':
        if '<h2' in tag_buffer:
            h2_tag = True
        elif '</h2' in tag_buffer:
            h2_tag = False
        elif tag_buffer == '<page>':
            f.write(stage)
            f.close()
            stage = ""
            page += 1
            print("Writing... ", out_filenames[page])
            f = open(out_filenames[page], "w")
        else:
            stage += tag_buffer
        tag = False
        text_buffer = ""

f.write(stage)
f.close()
