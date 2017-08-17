2017.08.16  

### 关于 IDF 下 make menuconfig 以及 Kconfig, Kconfig.projbuild, sdkconfig, sdkconfig.defaults, sdkconfig.old 等文件的一些理解和使用。  

在 IDF 下的 demo，如果你导出了 IDF_PATH 后输入 make menuconfig 那么会出现很多配置选项，这时候你会问：  
    
**1.为什么会出现这个蓝淡灰色的界面呢？**	// keyword: make menuconfig, GNU, Makefile  
**2.这个界面的作用是什么呢？**			// keyword: 配置, 参数  
**3.界面中的这些选项是如何来的呢？**		// keyword: Kconfig, Kconfig.projbuild  
**4.这些选项和 demo 是如何相互工作的呢？**	// keyword: sdkconfig, sdkconfig.defaults, sdkconfig.old, Kconfig, Kconfig.projbuild  
**5.我们该怎么用它呢？**				// keyword: Kconfig, Kconfig.projbuild, sdkconfig, sdkconfig.defaults, kconfig语法  
**6.Kconfig 语法？**						// keyword: 自定义配置，自定义参数  


我们依次根据这些问题来谈谈 make menuconfig 以及这些配置文件。

### 1. make menuconfig 之后为什么会出现这个蓝淡灰色的界面呢？	  
// **keyword: make menuconfig, GNU, Makefile**  
make menuconfig 并不是 IDF 独有的，在很多配置内核选项的地方都有设计这种做法。IDF 基于此种想法，沿袭了该种设计。下面说说出现这个界面出现的过程。  

在输入 make menuconfig 之后，GNU make 会找到当前目录的 Makefile 文件并执行执行命令。在该文件中，包含 include $(IDF_PATH)/make/project.mk ,于是 GNU 编译器去到 $(IDF_PATH)/make/project.mk 文件中执行命令，该 project.mk 文件中，又包含有 include $(IDF_PATH)/make/project_config.mk,于是 GNU 编译器又去到 $(IDF_PATH)/make/project_config.mk 文件中执行命令。  

project_config.mk 中做了以下主要事情：
- a) 指定一个编译目录 kconfig：`KCONFIG_TOOL_DIR=$(IDF_PATH)/tools/kconfig` 。该目录是完成整个界面的渲染工作以及 kconfig/lxdialog 目录下指定了一些box,list的显示方式等。
- b) 编译该目录: `$(MAKE) -C $(KCONFIG_TOOL_DIR)` 。编译完成之后，你可以在 kconfig 目录下看到 mconf 和 conf 两个可执行文件。编译这两个文件过程是由 kconfig/Makefile 决定的。
- c) 指定 make menuconfig 运行方式: `menuconfig: $(KCONFIG_TOOL_DIR)/mconf` 。由于在 b）中，我们已经编译生成用于界面渲染的 mconf 可执行文件，所以在用户输入 `make menuconfig`之后，GNU会调用 mconf ,用户即可看到蓝淡灰色的界面。

### 2. 这个界面的作用是什么呢？	
// **keyword: 配置, 参数**  
这部分很容易理解。这个界面是用来配置 ESP32 运行环境的一些参数。部分配置如下:
- Bootloader config -> Bootloader log verbosity 用来配置 cpu 启动和 bootloader 启动后的一些早期的LOG。这些早期LOG分别调用 ESP_EARLY_LOGX 系列接口来不同显示。// PS: 这个log显示很好玩，有红黄绿白各种颜色的。
- Security features // 请勿修改。这是决定是否开启 secureboot 和 flash 加密的配置。
- Serial flasher config -> Default serial port // 经常修改，来设置默认串口【make flash 烧写过程使用的串口】
- Serial flasher config -> Default baud rate // 经常设置为 921600，加快烧写速度。
- Serial flasher config -> Flash size // 可设为 4MB，来增大默认 flash 大小
- Partition Table // 分区表，这是设置 ESP32 的 user-bin 位置以及在线升级（OTA）的设置功能。 我们经常设置 Factory app, two OTA definitions,客户经常设置为 Custom partition table CSV来使用客户自定义的分区表
- Example Configuration // 这部分不一定都有，这是每个 example 可通过 Kconfig.projbuild 自由设置example本身所需要的一些设置。通常用来设置 WiFi 用户名密码等。  

- Component config // 设置 esp_idf/component 的每个 component 工作方式。有下面几个设置可留意一下。
- Component config -> ESP32-secific // 设置 CPU 频率，中断 watchdog , task watchdog[如果在程序中使用 vTaskDelay，可能会触发 watchdog] 等。
- Component config -> FreeRTOS // 设置操作系统是否运行在双核{APP CPU + PRO CPU}等。
- Component config -> Log output // 设置 Log 显示等级。
- Component config -> LWIP // 设置 lwip 协议栈相关属性，如打开 SO_REUSEADDR SO_RCVBUF 功能等。
- Component config -> mbedTLS //  SSL/TLS 如果使用 mbedTLS lib,可通过此选项配置 mbedTLS 工作方式，如开启调试[打印更多log]等。
- Component config -> OpenSSL //  SSL/TLS 如果使用 ssl lib,可通过此选项配置 ssl 工作方式，如开启调试[打印更多log]等。

### 3. 【原理】界面中的这些选项是如何来的呢？	 
// **keyword: Kconfig, Kconfig.projbuild**  
在 esp-idf 根目录下，有 Kconfig 文件，该文件是界面中所有选项的入口地方。Kconfig 文件被导出到环境变量，在 make menuconfig 执行中，mconf 会使用该 Kconfig 作为其参数[这一句是我大概理解的，我不是很确定]。  

在该 Kconfig 文件中，描述了 make menuconfig 中一系列选项的布局过程。  

`mainmenu "Espressif IoT Development Framework Configuration" ` 
// 描述了 make menuconfig 界面的标题

`menu "SDK tool configuration"` 
// 于是有了 make menuconfig 中的第一个选项 SDK tool configuration

`source "$COMPONENT_KCONFIGS_PROJBUILD"`	
// 这步将调用 component 下所有的 Kconfig.projbuild 文件！如: esp-idf/components/bootloader/Kconfig.projbuild, 于是有了 Bootloader config 选项  
// 在部分 example 的 main/ 目录下，也有 Kconfig.projbuild ，这也会形成一个选项，出现在make menuconfig 中。

`choice OPTIMIZATION_LEVEL`	
// 于是有了 make menuconfig 中的选项 Optimization level

`menu "Component config"`		
// 于是有了 make menuconfig 中的选项 Component config 

`source "$COMPONENT_KCONFIGS"`	
// 这步将调用 components 下所有的 Kconfig 文件！这些kconfig文件配置将作为 Component config 的子选项。如: esp-idf/components/freertos/Kconfig 配置文件，于是有了 Component config -> FreeRTOS 选项，components 下每个 Kconfig 文件都将作为 Component config 的一个子选项，显示在 make menuconfig 中。

### [重要]4. 这些选项和 demo 是如何相互工作的呢？  
// **keyword: sdkconfig, sdkconfig.defaults, sdkconfig.old, Kconfig, Kconfig.projbuild**  
这些选项配置好之后，最终都会将这些配置保存在生成的 sdkconfig 文件中。sdkconfig 文件中每一条配置你可以简单理解为宏定义，sdkconfig 文件是直接提供给 demo 使用的。demo 编译时候，根据 sdkconfig 中的配置来编译每个 component 以及相关设置。
- a) Kconfig 和 Kconfig.projbuild 上一节说过，是生成 sdkconfig 文件的源头。
- b) 如果你没有 sdkconfig 文件，在运行 make 时，会自动弹出 make menuconfig 菜单，然后生成 sdkconfig 文件。
- c) 如果你有 sdkconfig 文件，即可直接 make 该demo，也可通过 make menuconfig 来修改该 sdkconfig。
- d) sdkconfig.defaults 是设置生成 sdkconfig 中的默认值，可以理解为一种中间文件。我知道的至少有三个用途。
  - 其一，**在没有 sdkconfig 文件时候，但有 sdkconfig.defaults** 时，运行 make 或 make menuconfig ，系统将通过 sdkconfig.defaults 帮你选择某个选项默认的值。如在 sdkconfig.defaults 中有 CONFIG_ESPTOOLPY_BAUD_2MB=y ，那么在 make menuconfig 后，Serial flasher config -> Default baud rate 将默认帮你选择 2MBbaud
  - 其二，**在没有 sdkconfig 文件时候，但有 sdkconfig.defaults** 时，运行 make 或 make menuconfig ，系统将根据 sdkconfig.defaults 来决定 Kconfig, Kconfig.projbuild 中配置的显示。如 esp-idf/components/aws_iot/Kconfig 中有 `depends on AWS_IOT_SDK` ,如果没有sdkconfig.defaults文件或sdkconfig.defaults文件中没有 CONFIG_AWS_IOT_SDK=y ,那么make menuconfig 是无法配置 AWS IoT Endpoint Hostname 和 AWS IoT MQTT Port 的，因为他们的字段依赖于 AWS_IOT_SDK，一旦在sdkconfig.defaults中声明了 CONFIG_AWS_IOT_SDK，则可以配置 AWS IoT Endpoint Hostname 和 AWS IoT MQTT Port  [注:CONFIG_AWS_IOT_SDK 和 AWS_IOT_SDK 是配置生成的不同时期的状态，带 CONFIG_XXX 是配置文件中 sdkconfig/sdkconfig.defaults/sdkconfig.old存储的,而去掉前缀 CONFIG_ 是 Kconfig/Kconfig.projbuild 中存储的，这两者之间有一个转化过程]，所有Kconfig, Kconfig.projbuild 中带有 depends on XXX 的配置都可通过在 sdkconfig.defaults 中定义宏来打开配置选项。
  - 其三，sdkconfig.defaults 中每一条你都可以理解为一条宏定义。同上，如果 sdkconfig.defaults中有一条 CONFIG_AWS_IOT_SDK=y ，那么编译 aws_iot 这个component时候，esp-idf/components/aws_iot/component.mk 中将根据宏 CONFIG_AWS_IOT_SDK 来条件编译并生成 libaws_iot.a ，如果 sdkconfig.defaults 中没有 CONFIG_AWS_IOT_SDK=y 或者甚至没有 sdkconfig.defaults，那么编译器 将不会完整编译 aws_iot ，不会生成 libaws_iot.a,取而代之的是生成 一个临时文件 component_project_vars.mk
- f) sdkconfig.defaults 是在没有 sdkconfig 文件时候才起作用，如果有sdkconfig,那么 sdkconfig.defaults 将忽略。
- g) sdkconfig.old 是 sdkconfig 的备份文件，在每一次使用 make menuconfig 修改配置后，那么上一次的配置将保存在 sdkconfig.old 中，其他的用途暂且不知。
- h) 生成的 sdkconfig 你可以理解为一种宏定义的集合，在编译过程过程中，会根据这些宏或代替某些字段或条件编译某些component等。

#### 小结：   
**sdkconfig 文件是绝对的老大。Kconfig+Kconfig.projbuild 是生成 sdkconfig 的主要源头，sdkconfig.defaults 是配置 Kconfig+Kconfig.projbuild 中 默认选择的选项和 depends on 字段的选项。sdkconfig.old 是 sdkconfig 的备份。只要有 sdkconfig ，sdkconfig.defaults 将不起作用！**

### 5. 我们该怎么用它呢？  				  
// **keyword: Kconfig, Kconfig.projbuild, sdkconfig, sdkconfig.defaults**  
如果你理解了上面四节，那么也是很容易去使用它。
- a)如果你只是很简单的为自己的 demo 加一点配置，当然这些配置也可以放在 demo 的代码中。我们可以在 demo 的 main/ 目录下，增加一个 Kconfig.projbuild 文件，在该文件中增加一些配置，关于配置的语法，请参考第六节 Kconfig 语法。即可在 make menuconfig 主选项中看到自己增加的配置
- b)如果你想在 component 下增加一个组件，并为这个组件增加一些配置。那么你有两种选择：
  - 一: 将你的配置选项放在make menuconfig主选项中[和component config选项平级显示]，你需要在你的组件根目录下，新建一个 Kconfig.projbuild 文件。在该文件中增加配置，关于配置的语法，请参考第六节 Kconfig 语法。[参见 partition_table 组件部署]
  - 二: 将你的配置选项放在component config 选项之下[和FreeRTOS 选项平级显示]，你需要在你的组件根目录下，新建一个 Kconfig 文件。在该文件中增加配置，关于配置的语法，请参考第六节 Kconfig 语法。[参见 freertos 组件部署]
- c) 如果你想条件显示你的组件配置选项，或为你的组件配置设置默认值。请在组件下的Kconfig/Kconfig.projbuild 中使用 depends on 关键字，并结合第四节 sdkconfig.defaults 的用法来配置你的 demo. [参见 aws_iot组件部署]

### 6. Kconfig 语法	
// **keyword: 自定义配置，自定义参数**  
IDF 下的 Kconfig 语法和 linux 下的 Kconfig 语法几乎相同。主要有下面几类:  

#### 6.1  用户输入配置【如配置WiFi SSID/WiFi passwd】  
```
	menu "WiFi Settings"
	config WIFI_SSID
		string "wifi ssid"
		default "myssid"
		help
			helps ssid.
	config "WiFi PASSWORD"
		string "wifi passwd"
		default "mypasswd"
		help
			helps passwd
	endmenu
```
  
----------  

**说明:**  
  
**上面关键字有 menu,endmenu,config,string, default, help**  
  
`menu "WiFi Settings"`  
  
// 给用户显示看到的选项介绍  
  
`config WIFI_SSID`	 
  
// 最终 WiFi ssid 是由此决定的。如目前这句，最终会生成 CONFIG_WIFI_SSID   
  
`string "wifi ssid"`	  
  
// 给用户显示看到  
  
`default "myssid"`	  
  
// 设置默认的ssid ，以上你可以简单理解为 #define CONFIG_WIFI_SSID myssid 这句宏定义  
  
`help` 	
  
// 下面是对此选项的一些说明，相当于代码中的注释。  

----------  

#### 6.2 用户选择配置【如选择LOG等级】  
----------
```
choice OPTIMIZATION_LEVEL
    prompt "Optimization level"
    default OPTIMIZATION_LEVEL_DEBUG
    help
        This option sets optimization level.
        
config OPTIMIZATION_LEVEL_DEBUG
    bool "Debug"
config OPTIMIZATION_LEVEL_RELEASE
    bool "Release"
endchoice
```
----------
**说明:**  

**上面关键字有 choice, endchoice,prompt, default,help,config,bool**  

`choice OPTIMIZATION_LEVEL`	  
// choice 为关键字，OPTIMIZATION_LEVEL 可用来区分不同的 choice  
`prompt "Optimization level"`	  
// 给用户看到的选项介绍  
`default OPTIMIZATION_LEVEL_DEBUG`   
// 选定默认的一个选择，必须是接下来 config 中一个  
`help`   
// help为关键字，接下来是对此选项的一些说明，相当于代码中的注释。  
`config OPTIMIZATION_LEVEL_DEBUG`   
// 用户如果选择此选项，则在生成的 sdkconfig 就成了宏 CONFIG_OPTIMIZATION_LEVEL_DEBUG  
`bool "Debug"`	  
// bool 为关键字，"Debug 是显示给用户看的"，如果用户选择此选项，那么就意味这 #define CONFIG_OPTIMIZATION_LEVEL_DEBUG true,在 sdkconfig 中显示为 CONFIG_OPTIMIZATION_LEVEL_DEBUG=y  


#### 6.3 其他高级用法可根据关键字 source / depends on/tristate/ if endif/comment/select等来自行设置和测试。


