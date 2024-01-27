import io
import telebot
from sql import *

default_data_cnt_for_all = 500

default_data_cnt = dict()
bot = telebot.TeleBot('6431439137:AAGIWzBgIfrNWuMwWsTNuDk6o5E6x9dW7uo')


def default_cnt(chat_id):
    return default_data_cnt.get(chat_id, default_data_cnt_for_all)


def get_chat_cnt(message):
    args = message.text.split(' ')
    if len(args) > 1:
        if args[1].isdigit():
            return int(args[1])

    return default_cnt(message.chat.id)


@bot.message_handler(commands=['start'])
def start_message(message):
    bot.send_message(message.chat.id, 'hello world')


@bot.message_handler(func=lambda msg: msg.text.startswith("/data"))
def data(message):
    contents = get_info_text(False)
    bot.send_message(message.chat.id, contents)
    plot(message)


@bot.message_handler(func=lambda msg: msg.text.startswith("/plot"))
def plot(message):
    image = io.BytesIO()
    write_plot_to_iobytes(image, 10, 5, get_chat_cnt(message))
    image.seek(0, 0)
    bot.send_photo(message.chat.id, image)


@bot.message_handler(func=lambda msg: msg.text.startswith("/def"))
def set_default_data_len(message):
    global default_data_cnt
    default_data_cnt.update({ message.chat.id : get_chat_cnt(message) })
    bot.send_message(message.chat.id, get_chat_cnt(message))


bot.infinity_polling()
