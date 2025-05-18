Azan Nazar     | B00882606 | anazar1@binghamton.edu
Winnie Yong     

********************* HOW IT WORKS *********************

- For the following, the project relies on the curl command for a variety
of different tasks with 5 different options:
    - Adding a task
    - Viewing all tasks
    - Completing tasks
    - Deleting tasks
    - Viewing Task History

********************* HOW TO RUN *********************
> make clean
> make
> ./server
...
<add example command using curl as described below>

***************** TODO API Server *****************

1. Add a Task: 
   curl -X POST -d "Your task description" http://localhost:8080/todos
   Example: curl -X POST -d "Get 2% milk" http://localhost:8080/todos 
   Example: curl -X POST -d "Get mac and cheese" http://localhost:8080/todos

2. View All Tasks: 
   curl http://localhost:8080/todos

3. Complete a Task (by index): 
   curl -X PUT http://localhost:8080/todos/<index>/complete
   Example: curl -X PUT http://localhost:8080/todos/1/complete

4. Delete a Task (by index): 
   curl -X DELETE http://localhost:8080/todos/<index>
   Example: curl -X DELETE http://localhost:8080/todos/1

5. View Task History (Completed Tasks): 
   curl http://localhost:8080/todos/history


********** MEMBER CONTRIBUTION **********

Azan:
- Setting underlying structure for port, sockets, main and Makefile
- Implemented adding tasks, viewing all tasks, completing tasks, 
deleting a task, viewing task history

Winnie:
- Implement ...
