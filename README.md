# ERP
## Description
This ERP project uses C++ , MySQL, Openssl and Curl. Openssl to safely store our password in the database and MySQL for our backend database. 
OpenSSL is used to provide extra security for password we have used sha256 hashing to secure data and salt too. 
The password is salted then hashed then they are stored in separate table in database with username.Curl to send email otp to user gmail account.

The project has 3 types of users admin, staff and student. Each User has different level of authority and functions they can implement. 
- Admin can do everything from adding fees, semester change, marks update, Attendance Update, Salary Change/Add, Add User and Delete User etc.
- Staff have most function based on student specially students that they teach If a teacher teaches tcs101 to semester 1 sec a then he cannot take attendance of tcs102 or add marks.
  You can only change/Add changes to your own teaching section of that subject.
- Student on other hand has least authority they can only view data like view attendance, view marks, view own data, view teaching faculty data etc.
## Motive
- I wanted to create a OOPS project with a backend for database so I created this then little by little I added more functionalities as I went Adding OpenSSL and Curl in last.

## Software Version
- Visual Studio Version: Microsoft Visual Studio Community 2019 Version 16.11.31
- MySQL version : mysql  Ver 8.0.37 for Win64 on x86_64 (MySQL Community Server - GPL)
- OpenSSL version : OpenSSL 3.3.1 4 Jun 2024 (Library: OpenSSL 3.3.1 4 Jun 2024)
- Curl version : curl 8.7.1 (Windows) libcurl/8.7.1 Schannel zlib/1.3 WinIDN
- In future some files could be different as of now but the way to install and give path will probably be same. This project was completed on 05-07-2024
## Installation
> [!IMPORTANT]
> While giving the paths for the directory in Visual Studion make sure that you are in x64 platform and Release Configuration. I also have provided the screenshots check them out for better understanding

### Copying file address from address bar
Adress bar             
:-------------------------:
![Alt text](screenshots/address_bar.png)  

> [!IMPORTANT]
> If you having problem in adding dependencies in project properties then you can first look at screenshots below to get better understanding of work.

### Installing Dependencies
#### MySQL
  -	While installing MySQL installer please do remember the password save it somewhere.
  -	Before running the code First make sure to include all the tables you can do so by going to the MySQL command Line Client which you can found from searching in start, Enter the password then create a database e.g. CREATE DATABASE erp;
  -	Then use erp(database_name); to enter database and add tables.
  -	Whatever name you give it you will need to change the database.cpp file also change your password in  (!mysql_real_connect(conn, "localhost", "root", "your_password", "database_name", 3306, NULL, 0))
  -	Other than these two most of them will be same.
  -	For Setup of MySQL follow this YouTube video I also followed this person’s instructions and it worked perfectly.
  -	YouTube Link : https://youtu.be/yNniOHn9Xe0?si=gV1jeJSCyXxOEHFd
  -	In tables_sql.txt I already have code for all table data.
  -	In table_sql_fill.txt I have included all the insert query for course and fees data.
  -	And I also, have excel files for both the tables too for better view. You can add your own custom course data if you like this is just sample data I created.
> [!IMPORTANT]
> You must Create an admin (first user) for entering the erp login from the mysql command client itself. After that admin can be added from erp itself.

#### OpenSSL
  - Download the OpenSSL library setup file from Here - https://slproweb.com/products/Win32OpenSSL.html
  - Download the latest full version not light: e.g., Win64 OpenSSL v3.3.1 (Exe File)
  - After Installation include the opensssl path:
  - Go the PC then right click and Properties then Advanced System Setting in there Advanced and click environment variables:In the window in system variable go to path-edit-new-paste the openssl path should be like this: C:\Program Files\OpenSSL-Win64\bin (check your window to verify if different change it)
  - Make sure the following settings are setup in the project property pages:
  - Go To Project: [project name] Properties
    - [C/C++ -> General -> Additional Include Directories] value: OpenSSL’s include directory in your machine (e.g. C:\Program Files\OpenSSL-Win64\include)
    - [Linker -> General -> Additional Library Directories] value: OpenSSL’s lib directory in your machine (e.g. C:\Program Files\OpenSSL-Win64\lib)
    - [Linker -> Input -> Additional Dependencies] value: C:\Program Files\OpenSSL-Win64\lib\VC\x64\MT\libssl.lib
    - [Linker -> Input -> Additional Dependencies] value: C:\Program Files\OpenSSL-Win64\lib\VC\x64\MT\libcrypto.lib
	- Path could be different so use what’s yours

 #### LibCurl
- The curl already comes in Windows 10, version 1803 or later. To check curl version Open command prompt and type : curl -V
- You cross check your version with the latest version curl and if you still want to upgrade then you can follow this guide:
- https://stackoverflow.com/questions/9507353/how-do-i-install-and-use-curl-on-windows
- The top answer "You might already have curl" shows how.
- The below steps are still needed to be done even if you install again.
- Go to start -> Visual Studio 20xx -> x64 native tools command prompt or Developer command prompt anyone is fine
- now create a folder 'curl' somewhere for curl library it will be permanemt so choose c:\ or d:\ somewhere safe
- Now copy the address from address bar then type in visual studio prompt: cd C:\curl\
- then type these command
- git clone https://github.com/curl/curl.git
- buildconf.bat
- cd winbuild
- nmake /f Makefile.vc mode=static
- After this your build would be made in builds folder

Setting-up Project Configuration
- ${curl-dir} : is your directory of curl folder copy it from address bar
1. Create a new /Open an existing C++ project.

2. Open Project Properties.

3. Add ${curl-dir}\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl\include to C/C++ -> General -> Additional Include Directories.

4. Add ${curl-dir}\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl\lib to Linker -> General -> Additional Library Directories.

5. Add CURL_STATICLIB to Configuration Properties -> C/C++ -> Preprocessor -> Preprocessor Definitions.

6. In Configuration Properties -> Linker -> Input -> Additional Dependencies, add these followings lines-

- ${curl-dir}\builds\libcurl-vc-x86-release-static-ipv6-sspi-winssl\lib\libcurl_a.lib
- Ws2_32.lib
- Wldap32.lib
- Crypt32.lib
- Normaliz.lib

### Setting email values
- To use email otp sending you need to First Get your gmail app specific password.
- The curl code of sending mail is taken from official curl repositary: https://github.com/curl/curl/blob/master/docs/examples/smtp-tls.c
- You can change the code as per your liking but the steps below are necessary for sending mail.
  #### Creating App specific Password
  - The Gmail used in this will be the sender gmail.
  - 1. First you need to activate 2 Factor Authentication of Gmail :https://support.google.com/accounts/answer/185839?hl=en&co=GENIE.Platform%3DDesktop
  - 2. Now you need to create a app specific password.Store this password somewhere safe. You can follow this guide : https://www.zdnet.com/article/gmail-app-passwords-what-they-are-how-to-create-one-and-why-to-use-them/
  - Now in PasswordFunctions.cpp Change these values
    - Change #define FROM_MAIL     "your_email@gmail.com" to this email address
    - Now use encrypt_decrypt_app_pass.cpp in extra_fun folder to encrypt your app password.
      - Copy your app password in string password it will have space btwn like gvyu pong etzx poly as it is.
      - Change the int shift to your shift key 7,8,9,11,3, anything btwn 1-25
      - After checking validating decrypted pass copy this encrypted password to pass_app.txt as paste it.Only the encrypted text should be there in text file.
      - Also change the int shift value in sendMail funciton in in PasswordFunctions.cpp to your choosen shift key.
      - This is done to just increase the security as app password is a sensitive information.
    - curl_easy_setopt(curl, CURLOPT_USERNAME, "your_email@gmail.com");
    - In string mail_str change the body of mail as per your liking.

### Project File/Configuration ScreenShots
Project Files             |  In Project Preview
:-------------------------:|:-------------------------:
![Alt text](screenshots/1.jpg)  |  ![Alt text](screenshots/2.jpg)
Project Properties             |  C/C++ General Paths
![Alt text](screenshots/3.jpg)  |  ![Alt text](screenshots/4.jpg)
C/C++ Preprocessor Paths            |  C/C++ Code Generation: Setting
![Alt text](screenshots/5.jpg)  |  ![Alt text](screenshots/6.jpg)
Linker General Paths            |  Linker Input Paths
![Alt text](screenshots/7.jpg)  |  ![Alt text](screenshots/8.jpg)

## Running
- First Create an empty Project in Visual Studio with name you want.
- Add all the files in this repository the .cpp and .h files in the source except make_fun folder it's only for encrypt function test and  make sure to add the file. Right click on project on right side you can add file then copy paste or just add existing file. Don’t forget to change the database name and password. I already told above.
- Make sure you are running this on x64 and release Configuration.

#### You can play around with the functionalities in the ERP and understand it or add your own functionalities too.
ERP Login Screen             |  ERP Admin Menu
:-------------------------:|:-------------------------:
![Alt text](screenshots/9.jpg)  |  ![Alt text](screenshots/10.jpg)
ERP Staff Screen             |  ERP Student Menu
![Alt text](screenshots/11.JPG)  |  ![Alt text](screenshots/12.JPG)



## Good Luck [-_-]
![Alt text](screenshots/doctor_chopper.jpg)  
