import web

urls = (
    '/(.*)/', 'redirect',
    '/', 'index',
    '/Latest', 'Latest',
    '/Update', 'Update',
)

app = web.application(urls, globals())

class index:
    def GET(self):
        web.header("Content-Type", "text/html; charset=utf-8") 
        doc = '<form method="POST" accept-charset="utf-8" action="Update">'
        doc += '<input name=action type="submit" value="Get File List Latest">'
        doc += '</form>'
        doc += '<p>Get the file by id: (only 100 is valid)</p>'
        doc += '<form method="POST" accept-charset="utf-8" action="Update">'
        doc += '<input type="text" name="setupfileid" value="100"/>'
        doc += '<input name=action type="submit" value="Update">'
        doc += '</form>'
        print doc
        return doc
        
class redirect:
    def GET(self, path):
        web.seeother('/' + path)
        
class Latest:
    def POST(self):
        textfile=open("filelist.txt", "r")
        page=textfile.read()
        textfile.close()
        return page

class Update:
    def POST(self):
        user_data = web.input(setupfileid = 'Noitem')
        if (user_data.setupfileid == "1"):
            web.header('Content-Type', 'application/octet-stream')
            a = open('.\\out.gz','rb')
            s = a.read()        
            a.close()
        elif (user_data.setupfileid == "Noitem"):
            textfile=open("desc.txt", "r")
            s = textfile.read()
            textfile.close()
        else:
            return web.notfound("404 - Not found")
        return s;
        
if __name__ == "__main__": app.run()

