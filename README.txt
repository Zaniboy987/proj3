Azan Nazar     | B00882606 | anazar1@binghamton.edu
Winnie Yong    | B00844437 | wyong1@binghamton.edu  

********************* HOW IT WORKS *********************

- For the following, the project relies on the curl command for a variety
of different tasks with 10 different options:
    - Creating a todo list
    - Viewing all todo lists
    - Deleting a todo list
    - Adding a task to a list
    - Viewing all tasks of a list
    - Completing a task in a list
    - Deleting a task in a list
    - Viewing completed task history
    - Clearing all completed tasks in a list
    - Shutdown server

********************* HOW TO RUN *********************
> make clean
> make
> ./server

Then, in another terminal, use the following `curl` commands:

***************** TODO API Server *****************
1. Create a New List:  
   curl -X POST http://localhost:8080/lists/<list> 
   Example: curl -X POST http://localhost:8080/lists/groceries 
   Example: curl -X POST http://localhost:8080/lists/school

2. View All Lists:  
   curl http://localhost:8080/lists  

3. Delete a List:  
   curl -X DELETE http://localhost:8080/lists/<list>  
   Example: curl -X POST http://localhost:8080/lists/groceries 
   Example: curl -X DELETE http://localhost:8080/lists/school  

4. Add a Task to a List: 
   curl -X POST -d "Your task description" http://localhost:8080/todos/<list>  
   Example: curl -X POST -d "Get 2% milk" http://localhost:8080/todos/groceries 
   Example: curl -X POST -d "Get mac and cheese" http://localhost:8080/todos/groceries
   Example: curl -X POST -d "Finish math homework" http://localhost:8080/todos/school

5. View All Tasks: 
   curl http://localhost:8080/todos/<list>
   Example: curl http://localhost:8080/todos/groceries

6. Complete a Task (by index): 
   curl -X PUT http://localhost:8080/todos/<list>/<index>/complete
   Example: curl -X PUT http://localhost:8080/todos/groceries/2/complete
   Example: curl -X PUT http://localhost:8080/todos/school/1/complete

7. Delete a Task (by index): 
   curl -X DELETE http://localhost:8080/todos/<index>
   Example: curl -X DELETE http://localhost:8080/todos/groceries/1
   Example: curl -X DELETE http://localhost:8080/todos/school/1

8. View Task History (Completed Tasks) in a list: 
   curl http://localhost:8080/todos/<list>/history

9. Clear All Completed Tasks in a List:  
   curl -X DELETE http://localhost:8080/todos/<list>/completed  

10. Shutdown Server:  
   curl http://localhost:8080/shutdown 


********** MEMBER CONTRIBUTION **********

Azan:
- Setting underlying structure for port, sockets, main and Makefile
- Implemented adding tasks, viewing all tasks, completing tasks, 
deleting a task, viewing task history

Winnie:
- Extended functionality to support multiple named lists  
- Added support for clearing completed tasks, and remote shutdown  
