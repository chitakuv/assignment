# 下载安装 bazel

##### 配置 依赖：

sudo apt install openjdk-11-jdk  安装 java 11 
```bash
	sudo apt install openjdk-11-source openjdk-11-jdk-headless openjdk-11-dbg
	java -version  #验证 java 版本
```



##### 配置 jdk 环境变量到 ~/.bashrc 
```bash
	echo 'export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-arm64' >> ~/.bashrc

	echo 'export PATH=$JAVA_HOME/bin:$PATH' >> ~/.bashrc

	source ~/.bashrc

	echo $JAVA_HOME  验证是否配置成功
```


#### 下载安装 bazel：

去这个网页，找到 basel 的 dist 包 下载把这个包 放入 ~/Downloads

[https://github.com/bazelbuild/bazel/releases/tag/6.5.0](https://github.com/bazelbuild/bazel/releases/tag/6.5.0)  

```bash
	unzip bazel-6.5.0-dist.zip -d bazel   #在 ~/Downloads 解压dist包 为 bazel
	
	cd bazel   #cd 进入 Bazel 源代码的目录
	
	export CC=$(which gcc)  #配置gcc， 防止 bazel 链接不到 gcc
	
	sudo bash ./compile.sh   #还没有 bazel，首次编译 bazel源码
	
	~/Downloads/bazel/output/bazel   #编译成功后会在这个位置
	
	sudo cp output/bazel /usr/local/bin/bazel  #安装

	bazel --version   #查看版本信息
```


下载完后，将 改变 gcc 路径，否则 编译时 fetch 不到 rules_cc：

```bash
	export CC=/usr/bin/gcc
	
	export CXX=/usr/bin/g++
	
	source ~/.profile
```



# Bazel 的核心语法文件


| WORKSPACE   | 表示一个 Bazel 项目根目录，记录外部依赖等 |
| ----------- | ----------------------------------------- |
| BUILD       | 描述该目录下的模块如何编译                |
| BUILD.bazel | 等同于 BUILD，二选一                      |
| .bzl 文件   | Bazel 的模块/函数库（类似 .mk）           |

#  目录结构

根目录下所有内容 为工作区，在根目录 运行 bazel 编译  

	.
	│ ── main      #代码库
	│            ├── BUILD        #定义包 main
	│            ├── BUILD.toolchain
	│            └── hello-world.cc       #用户编写的要编译的 C++ 代码
	├── README.md
	├── MODULE.bazel   #标记工作区
	└── WORKSPACE      #标记工作区


# BUILD 文件 常见构建规则（规则 = 目标类型）：


	cc_binary 生成 C/C++ 的可执行程序
	cc_library 生成 C/C++ 的静态/动态库
	cc_test 生成测试目标
	android_binary 编译 Android 应用 APK
	java_library 编译 Java 类库
	filegroup 打包文件组（打包用）
	genrule 自定义 shell 脚本构建规则


# C++ 实例

	.
	│────── lib # 外部依赖
	│        ├── BUILD
	│        ├── hello-time.cc # 外部库
	│        └── hello-time.h # 外部库头文件
	├────── main # 主目录
	│         ├── BUILD
	│         ├── hello-greet.cc # 内部依赖（库）
	│         ├── hello-greet.h # 内部依赖（库头文件）
	│         └── hello-world.cc # 主程序
	├── MODULE.bazel
	└── README.md

##### lib/BUILD 外部依赖单独 写一个 库

```bash
cc_library(

	name = "hello-time",
	srcs = ["hello-time.cc"],
	hdrs = ["hello-time.h"],
	visibility = ["//main:__pkg__"], # 控制该库的可见性 ---> 只有 main目录下的目标可以依赖 hello-time
	                                 # __pkg__ 表示仅限 pkg 标记的目标可以访问该库
)
```



##### main/BUILD

```bash
# 创建 一个 c++ 库 (默认为 静态库)
cc_library( 
	name = "hello-greet", #要生成的库名
	srcs = ["hello-greet.cc"], #库的源文件
	hdrs = ["hello-greet.h"], #库的头文件
	linking = "dynamic", # 如果有这行，代表指定生成 动态库

)

# 生成一个 C/C++ 可执行文件
cc_binary( 
	name = "hello-world",      #要生成的文件名
	srcs = ["hello-world.cc"], #要编译的源文件
	deps = [                    #依赖列表，冒号表示当前目录的 label
		":hello-greet",         #依赖为 hello-greet 内部库
		"//lib:hello-time",     #依赖为 hello-time 外部库
	],
)
```


# 命令

```bash
	bazel build //main:hello-world   # name 字段是 hello-world，编译时 就是 hello-world

	./bazel-bin/main/hello-world   # 运行可执行文件

	bazel clean --expunge  #
```



#  测试

##### 配置 workspace
```bash
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
git_repository(
    name = "gtest",
    remote = "https://gitee.com/mirrors/googletest.git",
    branch = "v1.12.x",

)
```

##### 在 main 目录下 创建 test 目录

##### 配置main/BUILD 加入：


```bash
cc_test(
	name = "hello-test",
	srcs = ["test/hello-test.cc"], # 代表当前目录main下开始引用 test
	deps = [
		"@gtest//:gtest", # 依赖 Google Test 库
		"@gtest//:gtest_main", # 依赖 Google Test 主库
	],
)
```

##### 执行测试

```bash
	bazel test //main:hello-test
```