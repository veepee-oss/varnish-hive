from flask import Flask

app = Flask(__name__)

@app.route('/', defaults={'path': ''})
@app.route('/<path:path>')
def catch_all(path):
    return 'This is %s' % path

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8080)
