const host = 'localhost';
let clientId = '';

const form = document.querySelector('#registrationForm');
const usernameInput = document.getElementById('username');
const passwordInput = document.getElementById('password');

form.addEventListener('submit', async function (evt) {
  evt.preventDefault();

  clientId = usernameInput.value.trim();
  password = passwordInput.value.trim();
  console.log(`Регистрация: введённые данные: ${clientId}, ${password}`);

  try {
    // Отправляем HTTP POST-запрос на сервер
    const response = await fetch(`http://${host}:5000/reg`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ username: clientId, password: password })
    });

    if (response.ok) {
      const data = await response.json();
      console.log(`Ответ сервера:`, data);

      if (data.status == 'success') {
        alert(data.comment);
        // Сохраняем clientId в localStorage и переходим на страницу чата
        localStorage.setItem("clientId", clientId);
        window.location.href = "../chatPage/chat.html";
      } else {
        alert(data.comment || "Не удалось авторизоваться.");
      }
    } else {
      console.error(`Ошибка HTTP: ${response.status}`);
      const data = await response.json();
      console.log(`Ответ сервера:`, data);
      alert(data.comment || "Не удалось авторизоваться.");
    }
  } catch (error) {
    console.error("Ошибка при авторизации:", error);
    alert("Произошла ошибка. Проверьте соединение с сервером.");
  }
});
