import io
import telebot
from sql import *

default_data_cnt_for_all = 500

default_data_cnt = dict()
key = open("key.txt").read()
bot = telebot.TeleBot(key)


def default_cnt(chat_id):
    return default_data_cnt.get(chat_id, default_data_cnt_for_all)


def split_message_args(message):
    return message.text.replace('_', ' ').split()


def get_chat_cnt(message, arg_number):
    args = split_message_args(message)
    if len(args) > arg_number:
        if args[arg_number].isdigit():
            return int(args[arg_number])

    return default_cnt(message.chat.id)


def get_requested_source(message):
    args = split_message_args(message)
    if len(args) > 1:
        chars = list(args[1])
        if len(chars) > 0:
            chars[0] = chars[0].upper()
        return ''.join(chars)

    return ''

@bot.message_handler(commands=['start'])
def start_message(message):
    bot.send_message(message.chat.id, 'hello world')


@bot.message_handler(func=lambda msg: msg.text.startswith("/data"))
def data(message):
    contents = get_info_text(False, get_requested_source(message))
    bot.send_message(message.chat.id, contents)
    plot(message)


@bot.message_handler(func=lambda msg: msg.text.startswith("/plot"))
def plot(message):
    image = io.BytesIO()
    write_plot_to_iobytes(image, 10, 5, get_chat_cnt(message, 2), get_requested_source(message))
    image.seek(0, 0)
    bot.send_photo(message.chat.id, image)


@bot.message_handler(func=lambda msg: msg.text.startswith("/def"))
def set_default_data_len(message):
    global default_data_cnt
    default_data_cnt.update({ message.chat.id : get_chat_cnt(message, 1) })
    bot.send_message(message.chat.id, get_chat_cnt(message, 1))


bot.infinity_polling()
