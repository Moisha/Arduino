import requests
token = '6431439137:AAGIWzBgIfrNWuMwWsTNuDk6o5E6x9dW7uo'
method = 'sendMessage'

'''
response = requests.post(
    url='https://api.telegram.org/bot{0}/{1}'.format(token, method),
    data={'chat_id': 681184471, 'text': '/data'}
).json()
'''
response = requests.post(
    url='http://192.168.1.10:9000',
    data={'chat_id': 681184471, 'text': '/data'}
).text

print(response)