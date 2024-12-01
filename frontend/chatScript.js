const host = 'localhost';

let socket = null;
let clientId = '';


// первоначальной подключение к серверу
function connectToServer() {
  const message = {
    request_type: "auth",
    username: clientId,
    history: ""
  };

  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify(message));
    console.log('Запрос авторизации отправлен');
  } else {
    console.error("Соединение с сервером не установлено.");
  }
}


// Действия при загрузке страницы чатов
window.onload = function () {
  createWebSocketClient();
  clientId = localStorage.getItem('clientId');
};

const chatElement = document.querySelector('#messageOutput');
// Функция для создания WebSocket-клиента
function createWebSocketClient() {
  socket = new WebSocket(`ws://${host}:3000`);

  socket.onopen = () => {
    console.log("WebSocket соединение установлено");
    connectToServer();
  };

  socket.onmessage = (evt) => {
    data = JSON.parse(evt.data);
    console.log("Сообщение от сервера:", data);
    if (data.request_type == "msg") {
      chatElement.value += `${data.from} to ${data.to}: ${data.message}\n`;
      chatElement.scrollTop = chatElement.scrollHeight; // Автопрокрутка вниз
    } else if (data.request_type == "history") {
      console.log("history");
      const messages = data.history;
      messages.forEach( (msg) => {        
        chatElement.value += `${msg.from} to ${msg.to}: ${msg.message}\n`;
        chatElement.scrollTop = chatElement.scrollHeight; // Автопрокрутка вниз
      });
    }
  };

  socket.onerror = (error) => {
    console.error("Ошибка WebSocket:", error);
  };

  socket.onclose = () => {
    console.log("WebSocket соединение закрыто");
                                                                                                                // comment later
    localStorage.removeItem("cliendId");
  };
};


const toInput = document.getElementById("toInput");
const messageInput = document.getElementById("messageInput");
const form = document.querySelector('#sendMessageForm');
form.addEventListener('submit', function(evt) {
  evt.preventDefault();

  const messageText = messageInput.value.trim();
  const recipient = toInput.value.trim();

  // отображаем сообщение у себя
  chatElement.value += `${clientId} to ${recipient}: ${messageText}\n`;
  chatElement.scrollTop = chatElement.scrollHeight; // Автопрокрутка вниз

  // отправляем другому
  const message = {
    request_type: "msg",
    from: clientId,
    to: recipient || "all", // если поле "Кому" пустое, отправляем всем
    message: messageText
  };

  if (socket && socket.readyState === WebSocket.OPEN) {
    socket.send(JSON.stringify(message));
    messageInput.value = "";
  } else {
    console.error("Соединение с сервером не установлено.");
  }
});
