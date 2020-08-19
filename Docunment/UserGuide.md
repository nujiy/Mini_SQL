# MiniSQL使用帮助手册

**以一个学生数据库的创建和使用为例**



**数据库的创建**

当你第一次使用MiniSQL，它可能是空的，你可以通过一下命令创建一个数据库：

```C++
create database STU;
```

然后可以使用：

```
show database;
```

显示所有存在的数据库，这时你可以看到刚才创建的数据库了

**数据库的使用**

当我们创建完了数据库，需要使用数据库。在任何时刻，当你需要使用某一个数据库，需要使用一下命令：

```C++
use database STU;
```

现在我们已经可以使用数据库STU了

**数据库表的显示和创建**

每个关系型数据库都是由一张张表组成的，如果你想查看当前数据库下有几张表，可以使用：

```
show tables;
```

这时候，在 table 下面什么也没有，让我们来创建一张表吧

```c++
// 创建一张只有一个整型字段的表,字段名为id
create table student(id int);

// 对于一个学生，除了id,还有可能需要保存成绩，那么我们需要一个记录score的字段，字段类型为浮点数(小数)
create table student(id int, score double);

// 对了，我们还想保存这个学生的名字,学生的名字应该是一个字符串，我们需要指定字符串的长度，命令如下：
create table student(id int, score double, name char(20));
```

现在，我们已经可以使用MiniSQL创建一张表了。

需要注意的几点：

- MiniSQL目前只支持 int、double、char三种数据类型

如果你还想保存日期类型，如1992-08-21，你可以保存一个字符串"1992-08-21"，可以算是个不错的折中方案了~~

另外，你仍然需要注意的是，任何一张表都有一个**主键**，在上面创建表的命令中，我们没有指定任何字段作为主键，那么默认第一个字段为主键，也就是 "id"字段为主键。



*你也可以在创建表的时候指定一个字段为主键，方法就是在需要指定的字段后面添加 primary*

```C++
// 默认 id 为主键
create table student(id int, score double);

// 指定 score 为主键
create table student(id int, score double primary);

// 指定 name 为主键
create table student(id int, score double, name char(20) primary);
```

 **记录的插入和删除**

我们假设创建了一张表`create table student(id int, score double, name char(20));`

我们需要插入一条学生张三的记录他的id是1，成绩为90.5。

插入命令的格式为：insert into table_name(字段名称序列)values(字段对应的值序列);

比如：

```C++
insert into student(id,score,name)values(1,90.5,"ZhangSan");
```

这样我们就插入了张三的记录。注意两个括号里的内容分别为字段名称以及其对应的值，个数必须对应。

你如你想插入学生LiSi的数据，只知道他的id是2，但是不知道成绩，可以使用下面的命令

```C++
insert into student(id,name)values(2,"LiSi");//没有插入的字段将会使用默认值填充
```

如果想要删除某条记录，可以使用如下命令：

delete from 表名 where expr;

其中，expr为我们要删除的记录满足的删除条件，举例：

```C++
delete from student where id = 1; // 删除id为1的记录
delete from student where name = LiSi; // 删除名为LiSi的记录

// 多个删除条件可以组合使用
delete from student where name = LiSi and id > 6; // 删除id大于6且名为LiSi的记录
delete from student where score <= 60 and id > 6; // 删除成绩小于等于60且id大于6的记录
```

**注意**expr表达式只支持与操作(and),不支持或操作。关系运算符只支持 ">","=","<",">=","<="



**记录的更新** 

命令格式：update 表名 set 字段名=新的值 where expr

```C++
update student set score = 60 where id = 2; // 将id为2的记录的score字段的值更新为60
update student set score = 60 where id > 2; // 将id大于2的所有的记录的score字段的值更新为60
```

**记录的查找**

```C++
// select * from 表名 where expr;  表示查看所有满足expr表达式条件的记录
select * from student;              // 查看表中所有的记录 
select * from student where id = 1; // 查看张三的记录数据
select * from student where id > 3; // 查看所有id大于3的记录

// 如果不想显示查找结果所有的字段，可以将 * 替换为想要显示的字段
select id from student where id = 1; // 查看id为1的记录的id字段的值
select id,name from student where id = 1; // 查看id为1的记录的id字段和name字段的值

select score,name from student where id > 10 and score > 60; // 查看id大于10且score大于60的记录的score字段和name字段的值
```

**表的删除**

```C++
drop table student;
```

**数据库的删除**

```C++
drop database STU;
```

