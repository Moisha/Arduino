import io
import telebot
from sql import *

bot = telebot.TeleBot('6431439137:AAGIWzBgIfrNWuMwWsTNuDk6o5E6x9dW7uo')


@bot.message_handler(commands=['start'])
def start_message(message):
    bot.send_message(message.chat.id, 'hello world')

@bot.message_handler(commands=['data'])
def data(message):
    contents = get_info_text(False)
    bot.send_message(message.chat.id, contents)
    plot(message)


@bot.message_handler(commands=['plot'])
def plot(message):
    image = io.BytesIO()
    write_plot_to_iobytes(image, 10, 5)
    image.seek(0, 0)
    bot.send_photo(message.chat.id, image)


bot.infinity_polling()
