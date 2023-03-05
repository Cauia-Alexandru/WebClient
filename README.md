# WebClient 

## Description
For each command, I created separate functions which I then called in the main 
function. Besides the basic functions, I also have "get_user_info" which collects 
information about the user and returns it as a JSON string.

## Implementing commands

- For the "register" command, I made a post request, sent the information to the 
server, and then used the "receive_from_server" function to receive the response. 
Since the server returns errors in JSON format, I check for errors and display them 
or display a message indicating whether I have successfully registered.


- For the "login" command, I only used a different URL and stored cookies. With their
help, I can understand whether the user has successfully logged in or not.


- For the command "enter_library", I make a GET request and search through the
message returned by the server for the token. If it exists, it means that I have 
access and I store it so that I can execute the next commands, where I need the token
to prove that I have access.


- For the "get_book" command, I form a URL following the example in the homework 
prompt and parse the response from the server. I display the error or information
about the book.


- For the add_book command, I read all the information about the book from the 
keyboard and made sure that the input is not incomplete or in an incorrect format. 
I formatted the book with Parson and added it to a char** so that I could send it as
a parameter.


- I treated the cases when you want to log in twice in a row or log out twice in a 
row, where I display corresponding messages.