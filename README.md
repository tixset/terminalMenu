# Терминальное меню для контроллеров esp32 и esp8266 

При разработке командного интерфейса управления устройством в определенный момент сталкиваешься с проблемой огромной горы условий в коде, что зачастую приводит к неудобству его обслуживания.

Для упрощения этого процесса я разработал небольшую библиотку — конструктор терминаального меню, предназначенную для контроллеров esp32 и esp8266.

В примере "testTelnetMenu.ino" показано использование данной библиотеке на примере протокола telnet, но использовать ее можно не только в telnet, но и например при общении с контроллером по последовательному интерфейсу.

Библиотека позволяет удобно редактировать меню в процессе разработки без потори его структуры. Так же она поддерживает редактирование пунктов меню в процессе работы программы (переименование, перемещение, привязка другой функции и удаление).

Библиотека автоматически генерирует простую справку по пунтам меню, получить ее можно например вот так:
```c++
void tRootHelp(tfuncParams Params) {
  int x = 0;
  while (1) {
    String line = tMenu.getHelpLine(x);
    if (line == "") break;
    telnetSend(line, Params.clientIndex);
    x++;
  }
}
```
Использование этого кода присутствует в примере "testTelnetMenu.ino".

Далее приведу список функций класса "terminalMenu", а так же их назначение.
### Инициализация библиотеки:
```c++
		void init(tmenuLines* mLines, int mSize);
		void init(tmenuLines* mLines, int mSize, bool helpSubsEn);
```
В эту функцию передается основной массив меню и его размер, а так же есть возможность указать флаг который отключает вывод подпунктов меню в квадратных скобках, при вызове помощи.
### Привязка функций помощи и сообщения об ошибке:
```c++
		void helpAttach(func Func);
		void errAttach(func Func);
```
Про функцию помощи я уже написал ранее, а функция сообщения об ошибке нужна для вывода ошибки если пользователь неверно указал пункт меню. При этом если к предыдущему пункту меню привязана функция, то передаваемое следом значение считается параметром и передается внутрь функции.
### Добавление пунктов меню
```c++
		int add(String name);
		int add(String name, func Func);
		int add(int sub, String name);
		int add(int sub, String name, func Func);
```
При добавлении пункта меню указывается его название и функция которая будет выполняться при его вызове, если функцию не указать то к пункту меню автоматически привяжется функция помощи. Функция "add" при выполении возвращает id добавленного пункта меню, а указав этот id в качестве параметра "sub" следующего пункта меню можно вложить один пункт в другой.

<details> 
  <summary>Структуру меню из примера "testTelnetMenu.ino" можно увидеть под этим спойлером.</summary>
   <img alt="tixset, terminalMenu" src="https://github.com/tixset/terminalMenu/raw/main/screenshots/testTelnetMenu-menu-tree.jpg">
</details>
### Редактирование и удаление пунктов меню
```c++
		void ed(int index, String name);
		void ed(int index, int sub);
		void ed(int index, func Func);
		void del(int index);
```
Как я писал ранее, библиотека поддерживает редактирование пунктов меню в процессе работы программы (переименование, перемещение, привязка другой функции и удаление).
Если с редактированием все и так очевидно, то про удаление напишу чуть подробнее.
При вызове функции удпления пункта меню в функцию передается только порядковый номер этого пункта (id), при этом функция фактически не удаляет выбранный пункт, а только стирает его имя, что скрывает его из меню. При желании его так же легко можно восстановить обратно задав ему имя снова с помощью функции редактирования.
### Вызов функции из меню
```c++
		int goMenu(String line, int clientIndex);
```
Для вызова функции из меню мы передаем в функцию "goMenu" строку - адрес пункста меню, строка эта выглядит например так: "chip freq get". Так же в эту функцию мы передаем "clientIndex" для обратной связи с клиентом.

Пример вызова меню:
```
esp-test-telnet-menu:1# chip freq get
240 MHz
```
### Функции помощи
```c++
		String getHelpLine(int index);
		String getHelpLine(int index, int sub);
```
Про функцию помоши я писал в самом начале. Уточню тут что если в этой функции в параметре "sub" указать id элемента, то функция вернет строки помощи по этому элементу, а если не указывать, то вернутся строки помощи по корню меню.

Также класс "terminalMenu" помимо функций, имеет еще и переменную "MenuCount" в которой хранится количество добавленных элементов меню.

### Скрин окошка терминала:
<img alt="tixset, terminalMenu" src="https://github.com/tixset/terminalMenu/raw/main/screenshots/testTelnetMenu-putty.jpg">

# Terminal menu for esp32 and esp8266 controllers

When developing a command interface for device management, at some point you face the problem of a huge mountain of conditions in the code, which often leads to the inconvenience of its maintenance.

To simplify this process, I have developed a small terminal menu constructor library designed for esp32 and esp8266 controllers.

The example "testTelnetMenu.ino" shows the use of this library on the example of the telnet protocol, but it can be used not only in telnet, but also, for example, when communicating with the controller via the serial interface.

The library allows you to conveniently edit the menu during development without changing its structure. It also supports editing menu items during the program (renaming, moving, linking another function and deleting).

The library automatically generates a simple help on menu punts, you can get it for example like this:
```c++
void tRootHelp(tfuncParams Params) {
int x = 0;
while (1) {
String line = tMenu.getHelpLine(x);
if (line == "") break;
telnetSend(line, Params.clientIndex);
x++;
}
}
```
The use of this code is present in the example "testTelnetMenu.ino".

Below is a list of functions of the "terminalMenu" class, as well as their purpose.
### Library initialization:
```c++
void init(tmenuLines* mLines, int mSize);
void init(tmenuLines* mLines, int mSize, bool helpSubsEn);
```
The main menu array and its size are passed to this function, and it is also possible to specify a flag that disables the output of menu items in square brackets when calling help.
### Linking help functions and error messages:
```c++
void helpAttach(func Func);
void errAttach(func Func);
```
I have already written about the help function earlier, and the error message function is needed to output an error if the user incorrectly specified a menu item. At the same time, if a function is linked to the previous menu item, then the value passed next is considered a parameter and is passed inside the function.
### Adding menu items
```c++
int add(String name);
int add(String name, func Func);
int add(int sub, String name);
int add(int sub, String name, func Func);
```
When adding a menu item, its name and the function that will be performed when it is called are indicated, if the function is not specified, the help function will automatically be linked to the menu item. The "add" function returns the id of the added menu item when executed, and by specifying this id as the "sub" parameter of the next menu item, you can nest one item into another.

<details>
<summary>The menu structure from the example "testTelnetMenu.ino" can be seen under this spoiler.</summary>
<img alt="tixset, terminalMenu" src="https://github.com/tixset/terminalMenu/raw/main/screenshots/testTelnetMenu-menu-tree.jpg">
</details>
### Edit and delete menu items
```c++
void ed(int index, String name);
void ed(int index, int sub);
void ed(int index, func Func);
void del(int index);
```
As I wrote earlier, the library supports editing menu items during the program (renaming, moving, binding another function and deleting).
If everything is so obvious with editing, then I will write about the deletion in a little more detail.
When calling the function of removing a menu item, only the serial number of this item (id) is passed to the function, while the function does not actually delete the selected item, but only erases its name, which hides it from the menu. If desired, it can also be easily restored by giving it a name again using the edit function.
### Calling a function from the menu
```c++
int goMenu(String line, int clientIndex);
```
To call a function from the menu, we pass a string to the goMenu function - the address of the menu item, this string looks like this: "chip freq get". We also pass "clientIndex" to this function for customer feedback.

Example of menu call:
```
esp-test-telnet-menu:1# chip freq get
240 MHz
```
### Help functions
```c++
String getHelpLine(int index);
String getHelpLine(int index, int sub);
```
I wrote about the help function at the very beginning. I will clarify here that if you specify the id of an element in the "sub" parameter in this function, the function will return help lines for this element, and if you do not specify it, then help lines for the root of the menu will return.

Also, the "terminalMenu" class, in addition to functions, also has a variable "MenuCount" in which the number of added menu items is stored.

### Screen of the terminal window:
<img alt="text, terminal Man" src="https://github.com/tixset/terminalMenu/raw/main/screenshots/testTelnetMenu-putty.jpg">
