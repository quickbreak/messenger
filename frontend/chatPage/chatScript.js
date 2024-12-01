const host = 'localhost';
let socket = null;
let clientId = '';
const chatList = document.getElementById('chatList');
const chatTitle = document.getElementById('chatTitle');
const chatItemTemplate = document.getElementById('chatItemTemplate');
const messageList = document.getElementById('messageList');
const messageItemTemplate = document.getElementById('messageItemTemplate');
const sendMessageForm = document.getElementById('sendMessageForm');
// const toInput = document.getElementById('toInput');
const messageInput = document.getElementById('messageInput');

let activeChat = null;
let chats = {}; // Содержит все чаты и их сообщения

// Добавить чат в список
function addChat(chatId, chatName) {
  if (!chats[chatId]) {
    chats[chatId] = [];
    const chatItem = chatItemTemplate.content.cloneNode(true);
    chatItem.querySelector('.chat-item').setAttribute('data-chat-id', chatId);
    chatItem.querySelector('.chat-name').textContent = chatName;
    chatItem.querySelector('.chat-item').onclick = () => selectChat(chatId, chatName);
    chatList.appendChild(chatItem);
  }
}

// Выбрать активный чат
function selectChat(chatId, chatName) {
  // console.log(`selectChat: ${chatId}, ${chatName}`);
  activeChat = chatId;
  chatTitle.textContent = `${chatName}`;

  if (chats[chatId].length == 0) {
    // console.log('Запрос истории сообщений');
    // Запросить историю для чата
    if (socket && socket.readyState === WebSocket.OPEN) {
      const message = {
        request_type: 'history',
        username: clientId,
        chat_with: chatId,
      };
      socket.send(JSON.stringify(message));
    }
    // в ответ на этот запрос отобразятся сообщения
    // в socket.onmessage
  }  
  // deleteChildren(messageList);
  else {
    // console.log('Сообщения есть, отображаю');
    renderMessages(chatId);
  }
}


// Удалить всех детей DOM-элемента
function deleteChildren(list) {
  // while (list.firstChild) {
  //   list.removeChild(list.firstChild);
  // }
  list.replaceChildren();
};

// Отобразить новое сообщение
function displayNewMessage(msg) {
  // console.log(`displayNewMessage: ${msg}`);
  const newMessage = messageItemTemplate.content.cloneNode(true);
  newMessage.querySelector('.message').textContent = `${msg.text}`;
  // console.log(newMessage);
  // console.log(newMessage.querySelector('.message').classList);
  newMessage.querySelector('.message-item').classList.add(`${msg.type}`);
  messageList.appendChild(newMessage);
  messageList.scrollTop = messageList.scrollHeight;
}

// Отобразить сообщения для активного чата
function renderMessages(chatId) {
  // console.log(`renderMessages: ${chatId}`);
  deleteChildren(messageList); // очистить чат
  (chats[chatId] || []).forEach((msg) => { // для каждого сообщения
    displayNewMessage(msg); // отобразить сообщение
  });
  messageList.scrollTop = messageList.scrollHeight;
}

// Добавить сообщение в чат
function addMessage(chatId, from, text, type = 'received') {
  // console.log(`addMessage: ${chatId}, ${from}, ${text}, ${type}`);
  chats[chatId] = chats[chatId] || [];
  chats[chatId].push({ from, text, type });
  if (chatId === activeChat) {
    msg = {
      from: from,
      text: text,
      type: type
    };
    displayNewMessage(msg);
  }
}

// Подключение к серверу
function connectToServer() {
  const message = {
    request_type: 'auth',
    username: clientId,
  };

  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify(message));
    console.log('Запрос авторизации отправлен');
  } else {
    console.error('Соединение с сервером не установлено.');
  }
}

// Создание WebSocket-клиента
function createWebSocketClient() {
  socket = new WebSocket(`ws://${host}:3000`);

  socket.onopen = () => {
    console.log('WebSocket соединение установлено');
    connectToServer();
  };

  socket.onmessage = (evt) => {
    const data = JSON.parse(evt.data);
    console.log('Сообщение от сервера:', data);

    if (data.request_type === 'msg') {
      addMessage(data.to === clientId ? data.from : data.to, data.from, data.message);
    } else if (data.request_type === 'history') {
      const chatId = data.chat_with;
      const messages = data.history || [];
      chats[chatId] = [];
      messages.forEach((msg) => {
        addMessage(chatId, msg.from, msg.message, msg.from === clientId ? 'sent' : 'received');
      });
      // отобразить сообщения
      renderMessages(activeChat);
    } else if (data.request_type === 'chats') {
      const availableChats = data.chats || [];
      availableChats.forEach((chat) => addChat(chat.id, chat.user2));
    }
  };

  socket.onerror = (error) => {
    console.error('Ошибка WebSocket:', error);
  };

  socket.onclose = () => {
    console.log('WebSocket соединение закрыто');
    localStorage.removeItem('clientId');
  };
}

// Отправка сообщения
sendMessageForm.addEventListener('submit', (evt) => {
  evt.preventDefault();
  const messageText = messageInput.value.trim();
  const recipient = activeChat;

  if (messageText) {
    addMessage(activeChat, 'Вы', messageText, 'sent');

    const message = {
      request_type: 'msg',
      from: clientId,
      to: recipient,
      message: messageText,
    };

    if (socket && socket.readyState === WebSocket.OPEN) {
      socket.send(JSON.stringify(message));
      messageInput.value = '';
    } else {
      console.error('Соединение с сервером не установлено.');
    }
  }
});

// При загрузке страницы
window.onload = function () {
  clientId = localStorage.getItem('clientId');
  createWebSocketClient();
};
