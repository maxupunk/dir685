﻿<h1>МЕНЮ ПОДДЕРЖКИ</h1>
	<table border=0 cellspacing=0 cellpadding=0 width=750 height=1103>
 <tr>
 <td width="750" height=40><font size=4><b>Расширенные настройки</b></font></td>
 </tr>
 <tr>
 <td height=20>&nbsp;</td>

 </tr>
<tr>
	<td height=228>
		<p>
			<a name=05><strong>Расширенное перенаправление портов </strong></a><br>
			Опция Расширенного перенаправления портов позволяет пользователям Интернет получать доступ к сервисам LAN.
			Эта функция полезна для хостинга on-line сервисов, как например, FTP-, Web- или игровых серверов.
			Для каждой записи необходимо задать публичный порт маршрутизатора для перенаправления на внутренний IP-адрес LAN и LAN-порт.
		</p>
		<p>
			Параметры расширенного перенаправления портов<br>
			<strong><em>Имя</em></strong> -
			Выберите подходящее имя виртуального сервера, например, Web Server.
			Несколько наиболее распространенных серверов доступны из выпадающего меню "Имя приложения".
			Вбор одной из этих записей приведет к заполнению некоторых стандартных параметров для этого типа сервера.
			<strong><em>IP-адрес </em></strong> -
			IP-адрес системы во внутренней сети, обеспечивающий виртуальный сервис.
			Например, 192.168.0.50. Можно выбрать компьютер из списка DHCP-клиентов в
			выпадающем меню "Имя компьютера" или ввести вручную IP-адрес компьютера.<br>
			<strong><em>Имя приложения </em></strong> -
			Список определенных популярных приложений, которые может выбрать пользователь для более быстрой настройки.<br>
			<strong><em>Имя компьютера </em></strong> -  
			Список DHCP-клиентов.<br>
			<strong><em>Тип трафика</em></strong> -
			Выберите протокол, используемый этим сервисом. Наиболее распространены опции UDP, TCP и UDP/TCP.
			Они доступны для выбора в выпадающем меню. Чтобы задать использование других протоколов,
			выберите в списке "Другие", а затем введите соответствующий номер протокола (определяется IANA) в поле Протокол.<br>
			<strong><em>Приватный порт</em></strong> -
			Порт, используемый во внутренней сети.<br>
			<strong><em>Публичный порт</em></strong> -
			Порт, доступный из сети Интернет.<br>
		</p>
	</td>
</tr>
 <tr>
 <td height=20>&nbsp;</td>
 </tr>
 <tr>
 <td height=20>&nbsp;</td>

 </tr>
 <tr>
 <td height=20>&nbsp;</td>
 </tr>
 <tr>
 <td height=64> <strong>Правила приложения<font size=4><b><a name=06></a></b></font> </strong><br>
 	Для некоторых приложений требуется множественное соединение, например, Интернет-игры, видео-конференции,
 	Интернет-телефония и другие. Эти приложения работают с проблемами через NAT.  Для работы с такими приложениями укажите порт,
 	который обычно использует данное приложение в поле "Порт условия переключения", укажите тип протокола TCP или UDP, 
 	а затем введите публичные порты, относящиеся к порту условия переключения в поле Порт межсетевого экрана, чтобы открыть их для внутреннего трафика.
 	Наиболее распространенные приложения уже заданы в выпадающем меню Имя приложения. <br>

<strong><em>Имя </em></strong> -  Укажите Имя приложения. <br>
<strong><em>Порт условия переключения </em></strong>- Порт, используемый приложением. Это может быть один порт или диапазон.<br>
<strong><em>Тип трафика </em></strong> - Здесь необходимо задать протокол, используемый для переключения приложения. <br>
<strong><em>Порт межсетевого экрана </em></strong> -  Номер порта со стороны WAN, используемый для доступа к приложению. Можно задать один порт или их диапазон. Для добавления нескольких портов используйте ввод через запятую. <br>
<strong><em>Тип трафика </em></strong> - Здесь необходимо задать протокол, используемый для данного приложения. </td>

 </tr>
 <tr>
 <td height=20>&nbsp;</td>
 </tr>
 <tr>
 <td height=20>&nbsp;</td>
 </tr>
<tr>
	<td height=99>
		<p>
			<a name=07><strong>Управление доступом</strong></a><br>
			Фильтры позволяют разрешить или запретить компьютерам в LAN доступ к Интернет, а также локальной сети.
			В сети можно использовать эту опцию, чтобы разрешить или запретить доступ компьютеров к интернет на основе их MAC-адреса. 
		</p>
		<p>
			<strong>MAC-фильтры</strong><br>
			Использование MAC-фильтров позволяет запретить компьютерам в локальной сети доступ к Интернет.
			Также существует возможность добавить MAC-адрес вручную или выбрать MAC-адрес из списка килентов,
			подключенных к устройству.<br>
			Выберите "Включить MAC-фильтрацию и РАЗРЕШИТЬ компьютерам с перечисленными ниже MAC-адресами доступ к сети",
			если необходимо точно указать компьютеры, которым разрешен доступ к сети. Всем остальным компьютерам при этом
			доступ к сети будет запрещен.<br>
			Выберите "Включить MAC-фильтрацию и ЗАПРЕТИТЬ компьютерам с перечисленными ниже MAC-адресами доступ к сети",
			если необходимо точно указать компьютеры, которым запрещен доступ к сети. Всем остальным компьютерам при этом
			доступ к сети будет разрешен.<br>
			<strong><em>MAC-адрес</em></strong> -
			MAC-адрес сетевого устройства, который необходимо добавить в список MAC-фильтрации.<br>
			<strong><em>Список DHCP-клиентов</em></strong> -
			DHCP-клиенты характеризуется именем хоста в выпадающем меню Имя компьютера.
			Можно выбрать компьютер клиента, который необходимо добавить в список MAC-фильтра и кликнуть по соответствующей кнопке.
			Это автоматически добавит этот компьютер в список MAC-фильтра. 
		</p>
		<p>
			Пользователи могут использовать выпадающее меню <strong>Всегда</strong> для выбора настроенного
			ранее расписания или кликните по кнопке <strong>Добавить новое</strong>, чтобы добавить нове расписание.
		</p>
		<p>
			Существует возможность ключить или выключить определенную запись, добавив или убрав галочку в соответствующем поле.
		</p>
	</td>
</tr>
<tr><td height=20>&nbsp;</td></tr>
<tr><td height=20>&nbsp;</td></tr>
<tr>
	<td height=20>
		<p>
			<a name=08><strong>Межсетевой экран и DMZ</strong></a><br>
			Раздел Настройки межсетевого экрана содержит опции для настройки DMZ-хоста.
		</p>
		<p>
			<strong>Включить SPI</strong><br>
			SPI (Stateful packet inspection или динамическая фильтрация пакетов) помогает предотвратить атаки,
			отслеживая больше состояний за сессию. Он проверяет, чтобы трафик, переданный за эту сессию,
			удовлетворял соответствующему протоколу. Когда выбран протокол TCP, SPI проверяет,
			чтобы последовательные номера пакетов лежали в разрешенном диапазоне.
			Независимо от того, включен SPI или нет, маршрутизатор всегда отслеживает состояние TCP-соединения и гарантирует,
			что флаги всех TCP-пакетов леажат в корректном диапазоне.
		</p>
		<p>
			<strong>DMZ </strong><br>
			Для поддержки Интернет-приложений
			<?query("/sys/modelname");?>, не работающих должным образом за маршрутизатором, 
			существует возможность предоставить соответствующему компьютеру в сети неограниченный доступ к Интернет. 
			Введите IP-адрес этого компьютера как хост DMZ (димилитаризованная зона) с неограниченными правами доступа к Интернет. 
			Компьютер, расположенный в зоне DMZ, подвержен различным рискам безопасности. Поэтому устанавливайте эту опцию лишь в крайнем случае.
		</p>
		<p>
			<strong>Правила межсетевого экрана</strong><br>
			Для поддержки Интернет-приложений, не работающих должным образом за маршрутизатором,
			существует возможность предоставить соответствующему компьютеру в сети неограниченный доступ к Интернет.
			Введите IP-адрес этого компьютера как хост DMZ (димилитаризованная зона) с неограниченными правами доступа к Интернет.
			Компьютер, расположенный в зоне DMZ, подвержен различным рискам безопасности.
			Поэтому устанавливайте эту опцию лишь в крайнем случае.<br>
			<strong><em>Имя</em></strong> - Пользователи могут присвоить имя правилу межсетевого экрана.<br>
			<strong><em>Действие</em></strong> - Пользователи могут выбрать, запретить или разрешить прохождение определенного трафика.<br>
			<strong>Интерфейс</strong><br>
			<strong><em>Источник</em></strong> -
			Используйте данное выпадающее меню для выбора соответствующего назначения трафика для применения соответствующего действия. <br>
			<strong><em>Назначение</em></strong> -
			Используйте данное выпадающее меню для выбора соответствующего назначения трафика для применения соответствующего действия.<br>
			<strong><em>IP-адрес</em></strong> -
			Здесь существует возможность задать единичный IP-адрес источника или назначения,
			введя соответствующий параметр в верхнее поле. Также можно задать диапазон IP-адресов,
			введя начальный IP-адрес в верхнее поле, а конечный IP-адрес - в нижнее.<br>
			<strong><em>Протокол</em></strong> -
			Используйте выпадающее меню <strong>Протокол</strong>, чтобы указать тип трафика.<br>
			<strong><em>Диапазон портов</em></strong> -
			Чтобы задать диапазон портов, введите начальный номер порта в диапазоне в верхнее поле,
			а конечный - в нижнее. Если нужно задать один порт, введите его номер в оба поля.<br>
			<strong><em>Расписание</em></strong> -
			Используйте выпадающее меню <strong>Всегда</strong> для выбора ранее заданного
			расписания или кликните по кнопке Добавить новое, чтобы добавить новое расписание.
 </p>
	</div></td>
</tr>
<tr><td height=20>&nbsp;</td></tr>
<tr><td height=20>&nbsp;</td></tr>
 <tr>
 <td height=20><p><strong>Расширенные настрйоки беспроводного доступа <a name=09></a></strong></p>

 <p>Эти опции предназначены для настройки сетевыми профессионалами или в соответствии с их рекомендацией, поскольку их неправильная настройка может привести к снижению производительности маршрутизатора. </p>
 <p><strong><em>Скорости передачи (TX) - </em></strong> Выберите основные скорости передачи, основываясь на скорости беспроводных адаптеров WLAN. </p>
 <p><strong><em>Мощность передатчика - </em></strong> Существует возможность снизить выходную мощность, задав меньшее процентное соотношение в этом выпадающем меню. Доступны опции: 100%, 50%, 25% и 12.5%. </p>
 <p><strong><em>Beacon Interval - </em></strong> Beacon - это пакеты, которые отправляет точка доступа для синхронизации беспроводной сети. Задайте значение Beacon между 20 и 1000. Поумолчнаию установлено значение 100 мс. </p>

 <p><strong><em>RTS Threshold - </em></strong> Рекомендуетсфя оставить эту настройку в значении по умолчанию 2346. При несовместимом потоке данных рекомендуется лишь незначительно менять это значение. Возможный диапазон от 256 до 2346. </p>
 <p><strong><em>Фрагментация - </em></strong>  Рекомендуется оставить это значение по умолчанию 2346. При существенном количестве ошибочных пакетов можно немного увеличить значение данного поля (возможны значения от 1500 до 2346). Слишком маленькое значение в данном поле может привести к существенному снижению производительности. </p>
 <p><strong><em>Интервал DTIM - </em></strong> Введите значение между 1 и 255 для Delivery Traffic Indication Message (DTIM). DTIM - это счетчик, работающий на вычитание  и информирующий клиентов о прослушивании многоадресных и широковещательных сообщений. Когда в буфер точки доступа поступают многоадресный или широковещательные сообщения для соответствующих клиентов, она отправляет следующий DTIM через интервал DTIM. Клиенты точки доступа слышат beacon и готовятся к получению широковещательных или многоадресных сообщений. По умолчанию интервал DTIM установлен в 1. </p>

 <p><strong><em>Тип преамбулы - </em></strong> Тип преамбулы задает длину блока CRC для взаимодействия между точкой доступа и беспроводными адаптерами роуминга. </p>
 <p><span class="style2">Примечание: </span> При наличии большого количества сетевого трафика рекомендуется использовать меньший тип рпеамбулы. CRC - это основная техника, используемая для обнаружения ошибок передачи. </p>
 <p><strong><em>Режим CTS - </em></strong> Выбор опции Нет позволяет выключить эту функцию. Выберите "Всегда", чтобы все беспроводные устройства обменивались CTS/RTS перед передачей данных. Выберите "Авто" для автоматического определения необходимости обмена CTS/RTS. </p>
<?if(query("/runtime/func/ieee80211n")!="1"){
 echo "<p><strong><em>Режим только 802.11g - </em></strong>  Выберите этот режим, если сеть состоит из исключительно устройств 802.11g. Если на сети присутствуют как устройства 802.11b, так и 802.11g, выключите этот режим. </p> </td>";
}?>

 </tr>
 <tr>
 <td height=20>&nbsp;</td>
 </tr>
 <tr>
 <td height=20>&nbsp;</td>
 </tr>
 <tr>
 <td height=20><p><strong>Расширенные настройки сети <a name=10></a></strong></p>

 <p>В этом разделе содержатся настройки, которые позволяют изменить способ обработки определенного трафика марщрутизатором. Рекомендуется оставить эти настройки в значении по умолчанию. Для изменения данных настроек необходимо обладать специальными знаниями или посоветоваться со специалистами техподдержки. </p>

<!--
 <p><strong>VPN Passthrough </strong><br>The device supports VPN (Virtual Private Network) passthrough for PPTP (Point-to-Point Tunneling Protocol) and IPSec (IP Security). Once VPN passthrough is enabled, there is no need to create any Virtual Server or Port Forwarding entries in order for outbound VPN sessions to establish properly. Multiple VPN connections can be made through the device. This is useful when you have many VPN clients on the Local Area Network. </p>
-->

 <p><strong>UPnP </strong><br>UPnP (Universal Plug and Play) представляет собой сетевую архитектуру, обеспечивающую совместимость между сетевым оборудованием, программным обеспечением и периферией. Данный маршрутизатор поддерживает UPnP  и может быть использован с другими устройствами/ программным обеспечением UPnP. Если в испольховании UPnP нет необходимости, выберите, пожалуйста, опцию &quot;Выключено &quot;. </p>
 <p><strong>WAN Ping </strong><br>При включении данной опции IP-адрес WAN устройства будет отвечать на ping-команды, отправляемые пользователями Интернет. Следует помнить, что пингование IP-адресов WAN очень часто испольуется хакерами. </p>
 <p><strong>Скорость на WAN-порту </strong><br>Здесь существует возможность выбрать скорость на WAN-интерфейсе. Доступные опции: 100Мбит/с, 10Мбит/с или автоматический выбор 10/100Мбит/с </p>

<!--
 <p><strong>Игровой режим </strong><br>При возникновении трудностей при работе с online-играми или даже некоторыми приложениями, использующими передачу голоса, рекомендуется включить Игровой режим для работы с такими приложениями. В противном случае рекомендуется выключить Игровой режим. </p>
--> 
<?if(query("/runtime/func/dis_multicast_stream")!="1"){
echo " <p><strong>Multicast Streams</strong><br>Enable this option to allow Multicast traffic to pass from the Internet to your network more efficiently. </p> </td>\n";
}
?>
 </tr>

<?if(query("/runtime/func/dis_multicast_stream")!="1"){
echo "<tr><td height=20>&nbsp;</td></tr>\n";
echo "<tr>\n";
echo "	<td height=20>\n";
echo "		<p>\n"
echo "			<strong>Включение многоадресных потоков</strong><br>\n";
echo "			Включите эту опцию, если необходимо получение услуги video on demand из Интернет.\n";
echo "			Маршрутизатор использует IGMP-протокол для поддержки эффективной передачи многоадресных сообщений - например,\n";
echo "			передача мультимедиа от источника к нескольким получателям.\n"; 
echo "			Эту опцию необходимо включить, если приложения LAN работают с многоадресными группами.\n"; 
echo "			Если обнаруживается проблема с LAN-приложениями, которые не получают ожидаемый контент, попробуйте выключить эту опцию.\n";
echo "		</p>\n";
echo "	</td>\n";
echo "</tr>\n";
}
?>
 <tr>
 <td height=20>&nbsp;</td>
 </tr>
 <tr>
 <td height=20>&nbsp;</td>
 </tr>
<?if(query("/runtime/func/dis_routing")!="1"){
echo "<tr>\n";
echo "	<td height=99><strong>Маршрутизация </strong><a name=11></a><br>\n";
echo "		Опция Маршрутизация позволяет задать фиксированные маршруты, задающие пункты назначения.<br>\n";
echo "		<strong><em>Включено </em></strong> - опция, позволяющая включить или выключить определенную запись.<br>\n";
echo "		<strong><em>Интерфейс </em></strong> - Определите интерфейс (WAN или физический WAN), который должны использовать IP-пакеты при использовании этого маршрута.<br>\n";
echo "		<strong><em>Интерфейс (WAN)</em></strong> - Этот интерфейс получает IP-адрес от провайдера для доступа к Интернет.<br>\n";
echo "		<strong><em>Интерфейс (физический WAN)</em></strong> - этот интерфейс получает IP-адрес от провайдера для получения доступа к оборудованию провайдера.<br>\n";
echo "		<strong><em>Назначение </em></strong> - IP-адрес для пакетов, проходящих через данный маршрут.<br>\n";
echo "		<strong><em>Маска подсети </em></strong> - Один бит маски определяет, какие биты IP-адреса должны совпадать.<br>\n";
echo "		<strong><em>Шлюз </em></strong> - Определяет следующий шаг (hop) при использовании данного маршрута.\n";
echo "		Значение 0.0.0.0 в данном поле подразумевает, что следующий шаг отсутствует, и IP-адрес принадлежит маршрутизатору,\n";
echo "		а пакет отправляется на соответствующий интерфейс (WAN или Физический WAN).\n";
echo " </td>\n";
echo " </tr>\n";
}
?>

 <tr>
 <td height=20>&nbsp;</td>
 </tr>

 </table>
