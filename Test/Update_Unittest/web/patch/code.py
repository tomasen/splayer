import web
import os

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
        doc += '<p>Get the file by id: (only 1 is valid)</p>'
        doc += '<form method="POST" accept-charset="utf-8" action="Update">'
        doc += '<input type="text" name="setupfileid" value="100"/><br/>'
        doc += '<input type="text" name="MD5" value="2fd11376033f148d705ed5718b30a2f2"/><br/>'
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
    def GetFileByMd5(self, inputmd5):
        scope = {}
        filename = '.\\desc.txt'
        if not os.path.exists(filename):
            return ''
        textfile=open(filename, "r")
        s = textfile.read()
        textfile.close()
        print s
        exec(s, scope)
        print scope['pathces']
        #return scope["entries"]
        if (scope['pathces'].has_key(inputmd5)):
            return scope['pathces'].get(inputmd5)
        else:
            return ''

    def POST(self):
        user_data = web.input(setupfileid = 'Noitem', MD5='NOMD5')
        if (user_data.setupfileid == "1"):
            if (user_data.MD5 == 'NOMD5' ):
                web.header('Content-Type', 'application/octet-stream')
                a = open('.\\out.gz','rb')
            else:
                sPatch = self.GetFileByMd5(user_data.MD5)
                a = open(sPatch,'rb')
            s = a.read()        
            a.close()
        elif (user_data.setupfileid == "Noitem"):
            textfile=open("desc1.txt", "r")
            s = textfile.read()
            textfile.close()
        else:
            return web.notfound("404 - Not found")
        return s;
        

        
        
if __name__ == "__main__": app.run()

